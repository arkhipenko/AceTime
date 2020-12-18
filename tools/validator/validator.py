#!/usr/bin/env python3
#
# Copyright 2018 Brian T. Park
#
# MIT License
"""
Validate the inlined zonedb maps (zone_infos and zone_policies) generated by
InlineGenerator by feeding them into ZoneSpecifier, then comparing the results
of TestDataGenerator (which uses pytz).

TODO: Should this be rewritten as a python test? If this is not run in a
continuous integration pipeline, it's too easy to bitrot.
"""

from typing import List, Dict
import logging
from datetime import datetime
from zone_processor.ingenerator import ZoneInfoMap
from zone_processor.ingenerator import ZonePolicyMap
from zone_processor.zone_specifier import ZoneSpecifier
from zone_processor.zone_specifier import to_utc_string
from zone_processor.zone_specifier import SECONDS_SINCE_UNIX_EPOCH
from zone_processor.zone_specifier import BufferSizeInfo
from .zstdgenerator import TestDataGenerator
from .zstdgenerator import TestData
from .zstdgenerator import TestItem


class Validator:
    """Validate the zone_infos and zone_policies data from the TZ Database,
    as extracted and transformed by Extractor and Transformer. Provides
    2 validation methods:

        * validate_buffer_size(): to determine the sizes of various internal
          buffers using the ZoneSpecifier. The resulting buffer size gives
          insights into the corresponding buffer sizes of the C++ classes.
        * validate_test_data(): to compare the DST transitions between
          those determined by pztz (through TestDataGenerator) and those
          determined by ZoneSpecifier

    Usage:
        # For validation against pytz golden test data
        validator = Validator(zone_infos, zone_policies, ...)
        validator.validate_buffer_size()
        validator.validate_test_data()
    """

    def __init__(
        self,
        zone_infos: ZoneInfoMap,
        zone_policies: ZonePolicyMap,
        viewing_months: int,
        validate_dst_offset: bool,
        debug_validator: bool,
        debug_specifier: bool,
        zone_name: str,
        year: int,
        start_year: int,
        until_year: int,
        in_place_transitions: bool,
        optimize_candidates: bool,
    ):
        """
        Args:
            zone_infos: {name -> zone_info{} }
            zone_policies: {name ->zone_policy{} }
            viewing_months: number of months in the calculation window
                (13, 14, 36)
            validate_dst_offset: validate DST offset against Python in
                addition to total UTC offset
            debug_validator: enable debugging output for Validator
            debug_specifier: enable debugging output for ZoneSpecifier
            zone_name: validate only this zone
            year: validate only this year
            start_year: start year of validation
            until_year: until year of validation
            in_place_transitions: see ZoneSpecifier.in_place_transitions
            optimize_candidates: see ZoneSpecifier.optimize_candidates
        """
        self.zone_infos = zone_infos
        self.zone_policies = zone_policies
        self.viewing_months = viewing_months
        self.validate_dst_offset = validate_dst_offset
        self.debug_validator = debug_validator
        self.debug_specifier = debug_specifier
        self.zone_name = zone_name
        self.year = year
        self.start_year = start_year
        self.until_year = until_year
        self.in_place_transitions = in_place_transitions
        self.optimize_candidates = optimize_candidates

    # The following are public methods.

    def validate_buffer_size(self) -> None:
        """Find the maximum number of actual transitions and the maximum number
        of candidate transitions required for each zone, across a range of
        years.
        """
        # map of {zoneName -> (numTransitions, year)}
        transition_stats: Dict[str, BufferSizeInfo] = {}

        # If 'self.year' is defined, clobber the range of validation years.
        if self.year is not None:
            self.start_year = self.year
            self.until_year = self.year + 1
        logging.info(
            'Calculating transitions from [%s, %s)',
            self.start_year,
            self.until_year,
        )

        # Calculate the buffer sizes for every Zone in zone_infos.
        for zone_name, zone_info in sorted(self.zone_infos.items()):
            if self.zone_name and zone_name != self.zone_name:
                continue
            if self.debug_validator:
                logging.info('Validating zone %s', zone_name)

            zone_specifier = ZoneSpecifier(
                zone_info_data=zone_info,
                viewing_months=self.viewing_months,
                debug=self.debug_specifier,
                in_place_transitions=self.in_place_transitions,
                optimize_candidates=self.optimize_candidates)

            transition_stats[zone_name] = zone_specifier.get_buffer_sizes(
                self.start_year, self.until_year)

        logging.info('Zone Name: #NumTransitions (year); #MaxBufSize (year)')
        transition_stats_by_descending_count = sorted(
            transition_stats.items(),
            key=lambda x: x[1],
            reverse=True,
        )
        for zone_name, count_record in transition_stats_by_descending_count:
            logging.info(
                '{zone_name}: %d (%04d); %d (%04d)',
                zone_name,
                count_record.max_actives.count,
                count_record.max_actives.year,
                count_record.max_buffer_size.count,
                count_record.max_buffer_size.year,
            )

    def validate_test_data(self) -> None:
        """Compare Python and AceTime offsets by generating TestDataGenerator.
        """
        logging.info('Creating test data')
        data_generator = TestDataGenerator(
            self.zone_infos,
            self.zone_policies,
            self.start_year,
            self.until_year)
        (test_data, num_items) = data_generator.create_test_data()
        logging.info('Num zones: %d', len(test_data))
        logging.info('Num test items: %s', num_items)

        num_errors = self._validate_test_data(test_data)
        if num_errors:
            logging.info(f'Errors found with {num_errors} test items')
        else:
            logging.info('No errors found!')

    def _validate_test_data(self, test_data: TestData) -> int:
        num_errors = 0
        for zone_name, items in test_data.items():
            if self.zone_name and zone_name != self.zone_name:
                continue
            if self.debug_validator:
                logging.info('  Validating zone %s', zone_name)
            num_errors += self._validate_test_data_for_zone(zone_name, items)
        return num_errors

    def _validate_test_data_for_zone(
        self,
        zone_name: str,
        items: List[TestItem],
    ) -> int:
        """Compare the given test 'items' generatd by TestDataGenerator (using
        pytz) with the expected datetime components from ZoneSpecifier. Returns
        the number of errors.
        """
        zone_info = self.zone_infos[zone_name]
        zone_specifier = ZoneSpecifier(
            zone_info_data=zone_info,
            viewing_months=self.viewing_months,
            debug=self.debug_specifier,
            in_place_transitions=self.in_place_transitions,
            optimize_candidates=self.optimize_candidates)

        num_errors = 0
        for item in items:
            if self.year is not None and self.year != item.y:
                continue

            # Print out diagnostics if mismatch detected or if debug flag given
            unix_seconds = item.epoch + SECONDS_SINCE_UNIX_EPOCH
            ldt = datetime.utcfromtimestamp(unix_seconds)
            header = (
                f'======== Testing {zone_name}; '
                f'at {_test_item_to_string(item)}w; '
                f'utc {ldt}; '
                f'epoch {item.epoch}; '
                f'unix {unix_seconds}'
            )

            if self.debug_specifier:
                logging.info(header)

            try:
                info = zone_specifier.get_timezone_info_for_seconds(item.epoch)
            except Exception:
                logging.exception('Exception with test data {item}')
                raise
            is_matched = info.total_offset == item.total_offset
            status = '**Matched**' if is_matched else '**Mismatched**'
            ace_time_string = to_utc_string(info.utc_offset, info.dst_offset)
            utc_string = to_utc_string(
                item.total_offset - item.dst_offset,
                item.dst_offset
            )
            body = (
                f'{status}: '
                f'AceTime({ace_time_string}); '
                f'Expected({utc_string})'
            )
            if is_matched:
                if self.debug_specifier:
                    logging.info(body)
                    zone_specifier.print_matches_and_transitions()
            else:
                num_errors += 1
                if not self.debug_specifier:
                    logging.error(header)
                logging.error(body)
                zone_specifier.print_matches_and_transitions()

        return num_errors


def _test_item_to_string(i: TestItem) -> str:
    return f'{i.y:04}-{i.M:02}-{i.d:02}T{i.h:02}:{i.m:02}:{i.s:02}'
