# Copyright 2019 Brian T. Park
#
# MIT License

import logging
import datetime
import collections
import pytz
from zone_specifier import ZoneSpecifier
from zone_specifier import SECONDS_SINCE_UNIX_EPOCH
from zone_specifier import DateTuple

# An entry in the test data set.
TestItem = collections.namedtuple(
    "TestItem", "epoch total_offset dst_offset y M d h m s type")


class TestDataGenerator:
    """Generate the validation test data using the Transitions determined by
    ZoneSpecifier and the UTC offsets determined by pytz. This gives us
    stability which we can use to test other versions of ZoneSpecifier.
    """

    # The following zones have transitions which occurs at a time which is not a
    # multiple of 15 minutes, so cannot be represented by the C++ UtcOffset
    # object. In all cases below, the actual transition occurs at 00:01, but the
    # Transformer filter truncated the transition time to the nearest 15-minute
    # towards 00:00. To produce the correct validation_data.cpp data file,
    # for the transitions appearing below, we must shift the actual transition
    # time to 00:01 before calling the timezone object in PyTz.
    CORRECTIONS = {
        'Gaza': [
            (DateTuple(2010, 3, 17, 60, 'w'), -60),
            (DateTuple(2011, 4, 1, 60, 'w'), -60),
        ],
        'Goose_Bay': [
            (DateTuple(2000, 4, 2, 60, 'w'), -60),
            (DateTuple(2000, 10, 29, 60, 'w'), -60),
            (DateTuple(2001, 4, 1, 60, 'w'), -60),
            (DateTuple(2001, 10, 28, 60, 'w'), -60),
            (DateTuple(2002, 4, 7, 60, 'w'), -60),
            (DateTuple(2002, 10, 27, 60, 'w'), -60),
            (DateTuple(2003, 4, 6, 60, 'w'), -60),
            (DateTuple(2003, 10, 26, 60, 'w'), -60),
            (DateTuple(2004, 4, 4, 60, 'w'), -60),
            (DateTuple(2004, 10, 31, 60, 'w'), -60),
            (DateTuple(2005, 4, 3, 60, 'w'), -60),
            (DateTuple(2005, 10, 30, 60, 'w'), -60),
            (DateTuple(2006, 4, 2, 60, 'w'), -60),
            (DateTuple(2006, 10, 29, 60, 'w'), -60),
            (DateTuple(2007, 3, 11, 60, 'w'), -60),
            (DateTuple(2007, 11, 4, 60, 'w'), -60),
            (DateTuple(2008, 3, 9, 60, 'w'), -60),
            (DateTuple(2008, 11, 2, 60, 'w'), -60),
            (DateTuple(2009, 3, 8, 60, 'w'), -60),
            (DateTuple(2009, 11, 1, 60, 'w'), -60),
            (DateTuple(2010, 3, 14, 60, 'w'), -60),
            (DateTuple(2010, 11, 7, 60, 'w'), -60),
            (DateTuple(2011, 3, 13, 60, 'w'), -60),
        ],
        'Hebron': [
            (DateTuple(2011, 4, 1, 60, 'w'), -60),
        ],
        'Moncton': [
            (DateTuple(2000, 4, 2, 60, 'w'), -60),
            (DateTuple(2000, 10, 29, 60, 'w'), -60),
            (DateTuple(2001, 4, 1, 60, 'w'), -60),
            (DateTuple(2001, 10, 28, 60, 'w'), -60),
            (DateTuple(2002, 4, 7, 60, 'w'), -60),
            (DateTuple(2002, 10, 27, 60, 'w'), -60),
            (DateTuple(2003, 4, 6, 60, 'w'), -60),
            (DateTuple(2003, 10, 26, 60, 'w'), -60),
            (DateTuple(2004, 4, 4, 60, 'w'), -60),
            (DateTuple(2004, 10, 31, 60, 'w'), -60),
            (DateTuple(2005, 4, 3, 60, 'w'), -60),
            (DateTuple(2005, 10, 30, 60, 'w'), -60),
            (DateTuple(2006, 4, 2, 60, 'w'), -60),
            (DateTuple(2006, 10, 29, 60, 'w'), -60),
        ],
        'St_Johns': [
            (DateTuple(2000, 4, 2, 60, 'w'), -60),
            (DateTuple(2000, 10, 29, 60, 'w'), -60),
            (DateTuple(2001, 4, 1, 60, 'w'), -60),
            (DateTuple(2001, 10, 28, 60, 'w'), -60),
            (DateTuple(2002, 4, 7, 60, 'w'), -60),
            (DateTuple(2002, 10, 27, 60, 'w'), -60),
            (DateTuple(2003, 4, 6, 60, 'w'), -60),
            (DateTuple(2003, 10, 26, 60, 'w'), -60),
            (DateTuple(2004, 4, 4, 60, 'w'), -60),
            (DateTuple(2004, 10, 31, 60, 'w'), -60),
            (DateTuple(2005, 4, 3, 60, 'w'), -60),
            (DateTuple(2005, 10, 30, 60, 'w'), -60),
            (DateTuple(2006, 4, 2, 60, 'w'), -60),
            (DateTuple(2006, 10, 29, 60, 'w'), -60),
            (DateTuple(2007, 3, 11, 60, 'w'), -60),
            (DateTuple(2007, 11, 4, 60, 'w'), -60),
            (DateTuple(2008, 3, 9, 60, 'w'), -60),
            (DateTuple(2008, 11, 2, 60, 'w'), -60),
            (DateTuple(2009, 3, 8, 60, 'w'), -60),
            (DateTuple(2009, 11, 1, 60, 'w'), -60),
            (DateTuple(2010, 3, 14, 60, 'w'), -60),
            (DateTuple(2010, 11, 7, 60, 'w'), -60),
            (DateTuple(2011, 3, 13, 60, 'w'), -60),
        ],
    }

    def __init__(self, zone_infos, zone_policies):
        """
        Args:
            zone_infos: (dict) of {name -> zone_info{} }
            zone_policies: (dict) of {name ->zone_policy{} }
        """
        self.zone_infos = zone_infos
        self.zone_policies = zone_policies
        self.zone_name = ''
        self.viewing_months = 14
        self.start_year = 2000
        self.end_year = 2019

    def create_test_data(self):
        """Create a map of {
            zone_short_name: [ TestItem() ]
        }
        Return (test_data, num_items).
        """
        test_data = {}
        num_items = 0
        for zone_short_name, zone_info in sorted(self.zone_infos.items()):
            if self.zone_name != '' and zone_short_name != self.zone_name:
                continue
            test_items = self._create_test_data_for_zone(
                zone_short_name, zone_info)
            if test_items:
                test_data[zone_short_name] = test_items
                num_items += len(test_items)
        return (test_data, num_items)

    def _create_test_data_for_zone(self, zone_short_name, zone_info):
        """Create the TestItems for a specific zone.
        """
        zone_specifier = ZoneSpecifier(zone_info)
        zone_full_name = zone_info['name']
        try:
            tz = pytz.timezone(zone_full_name)
        except:
            logging.error("Zone '%s' not found in Python pytz package",
                          zone_full_name)
            return None

        return self._create_transition_test_items(tz, zone_specifier)

    @staticmethod
    def _add_test_item(items_map, item):
        current = items_map.get(item.epoch)
        if current:
            # If a duplicate TestItem exists for epoch, then check that the
            # data is exactly the same.
            if (current.total_offset != item.total_offset
                    or current.dst_offset != item.dst_offset
                    or current.y != item.y or current.M != item.M
                    or current.d != item.d or current.h != item.h
                    or current.m != item.m or current.s != item.s):
                raise Exception('Item %s does not match item %s' % (current,
                                                                    item))
            # 'A' and 'B' takes precedence over 'S' or 'Y'
            if item.type in ['A', 'B']:
                items_map[item.epoch] = item
        else:
            items_map[item.epoch] = item

    def _create_transition_test_items(self, tz, zone_specifier):
        """Create a TestItem for the tz for each zone, for each year from
        start_year to end_year, inclusive. The following test samples are
        created:

        * One test point for each month, on the first of the month.
        * One test point for Dec 31, 23:00 for each year.
        * A test point at the transition from DST to Standard, or vise versa.
        * A test point one second before the transition.

        Each TestData is annotated as:
        * 'A': pre-transition
        * 'B': post-transition
        * 'S': a monthly test sample
        * 'Y': end of year test sample

        For [2000, 2038], this generates about 100,000 data points.
        """
        items_map = {}
        for year in range(self.start_year, self.end_year + 1):
            # Skip start_year when viewing months is 36, because it needs data
            # for (start_year - 3), but ZoneSpecifier won't generate data for
            # years that old.
            if self.viewing_months == 36 and year == self_start_year: continue

            # Add samples just before and just after the DST transition.
            zone_specifier.init_for_year(year)
            for transition in zone_specifier.transitions:
                # Some Transitions from ZoneSpecifier are in previous or post
                # years (e.g. viewing_months = [14, 36]), so skip those.
                start = transition.startDateTime
                transition_year = start.y
                if transition_year != year: continue

                # If viewing_months== (13 or 36), don't look at Transitions at
                # the beginning of the year since those have been already added.
                if self.viewing_months in [13, 36]:
                    if start.M == 1 and start.d == 1 and start.ss == 0:
                        continue

                epoch_seconds = transition.startEpochSecond
                test_item = self._create_test_item_from_epoch_seconds(
                    tz, epoch_seconds - 1, 'A')
                self._add_test_item(items_map, test_item)

                test_item = self._create_test_item_from_epoch_seconds(
                    tz, epoch_seconds, 'B')
                self._add_test_item(items_map, test_item)

            # Add one sample test point on the first of each month
            for month in range(1, 13):
                test_item = self._create_test_item_from_datetime(
                    tz, year, month=month, day=1, hour=0, type='S')
                self._add_test_item(items_map, test_item)

            # Add a sample test point at the end of the year.
            test_item = self._create_test_item_from_datetime(
                tz, year, month=12, day=31, hour=23, type='Y')
            self._add_test_item(items_map, test_item)

        # Return the TestItems ordered by epoch
        return [items_map[x] for x in sorted(items_map)]

    def _create_test_item_from_datetime(self, tz, year, month, day, hour,
                                        type):
        """Create a TestItem for the given year, month, day, hour in local
        time zone.
        """
        # Can't use the normal datetime constructor for pytz. Must use
        # timezone.localize(). See https://stackoverflow.com/questions/18541051
        dt = tz.localize(datetime.datetime(year, month, day, hour))
        unix_seconds = int(dt.timestamp())
        epoch_seconds = unix_seconds - SECONDS_SINCE_UNIX_EPOCH
        return self._create_test_item_from_epoch_seconds(
            tz, epoch_seconds, type)

    def _create_test_item_from_epoch_seconds(self, tz, epoch_seconds, type):
        """Return the TestItem fro the epoch_seconds.
            total_offset: the total UTC offset
            dst_offset: the DST offset
        The base offset is (total_offset - dst_offset).
        """
        unix_seconds = epoch_seconds + SECONDS_SINCE_UNIX_EPOCH
        utc_dt = datetime.datetime.fromtimestamp(
            unix_seconds, tz=datetime.timezone.utc)
        dt = utc_dt.astimezone(tz)
        total_offset = int(dt.utcoffset().total_seconds())
        dst_offset = int(dt.dst().total_seconds())

        return TestItem(
            epoch=epoch_seconds,
            total_offset=total_offset,
            dst_offset=dst_offset,
            y=dt.year,
            M=dt.month,
            d=dt.day,
            h=dt.hour,
            m=dt.minute,
            s=dt.second,
            type=type)
