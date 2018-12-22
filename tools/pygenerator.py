# Copyright 2018 Brian T. Park
#
# MIT License
"""
Generate the zone_info and zone_policies files for Python.
"""

import logging
import os

from transformer import short_name
from generator import normalize_name
from generator import normalize_raw

class PythonGenerator:
    """Generate Python files for zone infos and policies.
    """

    ZONE_POLICIES_FILE = """\
# This file was generated by the following script:
#
#  $ {invocation}
#
# using the TZ Database files
#
#  {tz_files}
#
# from https://github.com/eggert/tz/releases/tag/{tz_version}
#
# DO NOT EDIT

# numPolicies: {numPolicies}
# numRules: {numRules}

{policyItems}

# The following zone policies are not supported in the current version of
# AceTime.
#
# numPolicies: {numRemovedPolicies}
#
{removedPolicyItems}

"""

    ZONE_POLICY_ITEM = """\
#---------------------------------------------------------------------------
# Policy name: {policyName}
# Rule count: {numRules}
#---------------------------------------------------------------------------
ZONE_RULES_{policyName} = [
{ruleItems}
]
ZONE_POLICY_{policyName} = {{
    "name": "{policyName}",
    "rules": ZONE_RULES_{policyName}
}}

"""

    ZONE_REMOVED_POLICY_ITEM = """\
# {policyName} ({policyRemovalReason})
"""


    ZONE_RULE_ITEM = """\
    # {rawLine}
    {{
        "fromYearFull": {fromYearFull},
        "toYearFull": {toYearFull},
        "inMonth": {inMonth},
        "onDayOfWeek": {onDayOfWeek},
        "onDayOfMonth": {onDayOfMonth},
        "atHour": {atHour},
        "atHourModifier": '{atHourModifier}',
        "deltaCode": {deltaCode},
        "letter": '{letter}',
    }},
"""

    ZONE_INFOS_FILE = """\
# This file was generated by the following script:
#
#  $ {invocation}
#
# using the TZ Database files
#
#  {tz_files}
#
# from https://github.com/eggert/tz/releases/tag/{tz_version}
#
# DO NOT EDIT

from zone_policies import *

# numInfos: {numInfos}
# numEntries: {numEntries}

{infoItems}

ZONE_INFO_MAP = {{
{infoMapItems}
}}

# The following zones are not supported in the current version of AceTime.
#
# numInfos: {numRemovedInfos}
#
{removedInfoItems}
"""

    ZONE_INFO_ITEM = """\
#---------------------------------------------------------------------------
# Zone name: {infoFullName}
# Entry count: {numEntries}
#---------------------------------------------------------------------------

ZONE_ENTRIES_{infoShortName} = [
{entryItems}
]
ZONE_INFO_{infoShortName} = {{
    "name": "{infoFullName}",
    "entries": ZONE_ENTRIES_{infoShortName}
}}

"""

    ZONE_REMOVED_INFO_ITEM = """\
# {infoFullName} ({infoRemovalReason})
"""

    ZONE_ENTRY_ITEM = """\
    # {rawLine}
    {{
      "offsetCode": {offsetCode},
      "zonePolicy": {zonePolicy},
      "format": "{format}",
      "untilYearShort": {untilYearShort},
      "untilMonth": {untilMonth},
      "untilDay": {untilDay},
      "untilHour": {untilHour},
    }},
"""

    ZONE_INFO_MAP_ITEM = """\
    "{infoShortName}": ZONE_INFO_{infoShortName},
"""

    ZONE_INFOS_FILE_NAME = 'zone_infos.py'
    ZONE_POLICIES_FILE_NAME = 'zone_policies.py'

    EPOCH_YEAR = 2000
    YEAR_SHORT_MAX = 127
    YEAR_MAX = 9999

    def __init__(self, invocation, tz_version, tz_files,
                 zones_map, rules_map, removed_zones, removed_policies):
        self.invocation = invocation
        self.tz_version = tz_version
        self.tz_files = tz_files
        self.zones_map = zones_map
        self.rules_map = rules_map
        self.removed_zones = removed_zones
        self.removed_policies = removed_policies

    def generate_files(self, output_dir):
        self.write_file(output_dir,
            self.ZONE_POLICIES_FILE_NAME, self.generate_policies())

        self.write_file(output_dir,
            self.ZONE_INFOS_FILE_NAME, self.generate_infos())

    def write_file(self, output_dir, filename, content):
        full_filename = os.path.join(output_dir, filename)
        with open(full_filename, 'w', encoding='utf-8') as output_file:
            print(content, end='', file=output_file)
        logging.info("Created %s", full_filename)

    def generate_policies(self):
        (num_rules, policy_items) = self.generate_policy_items(self.rules_map)
        removed_policy_items = self.generate_removed_policy_items(
            self.rules_map)

        return self.ZONE_POLICIES_FILE.format(
            invocation=self.invocation,
            tz_version=self.tz_version,
            tz_files=', '.join(self.tz_files),
            numPolicies=len(self.rules_map),
            numRules=num_rules,
            policyItems=policy_items,
            numRemovedPolicies=len(self.removed_policies),
            removedPolicyItems=removed_policy_items)

    def generate_policy_items(self, rules_map):
        num_rules = 0
        policy_items = ''
        for name, rules in sorted(rules_map.items()):
            policy_items += self.generate_policy_item(name, rules)
            num_rules += len(rules)
        return (num_rules, policy_items)

    def generate_policy_item(self, name, rules):
        rule_items = ''
        for rule in rules:
            atHour = rule['atMinute'] // 60
            rule_items += self.ZONE_RULE_ITEM.format(
                policyName=normalize_name(name),
                rawLine=normalize_raw(rule['rawLine']),
                fromYearFull=rule['fromYear'],
                toYearFull=rule['toYear'],
                inMonth=rule['inMonth'],
                onDayOfWeek=rule['onDayOfWeek'],
                onDayOfMonth=rule['onDayOfMonth'],
                atHour=atHour,
                atHourModifier=rule['atHourModifier'],
                deltaCode=rule['deltaCode'],
                letter=rule['letter'])
        return self.ZONE_POLICY_ITEM.format(
            policyName=normalize_name(name),
            numRules=len(rules),
            ruleItems=rule_items);

    def generate_removed_policy_items(self, rules_map):
        removed_policy_items = ''
        for name, reason in sorted(self.removed_policies.items()):
            removed_policy_items += \
                self.ZONE_REMOVED_POLICY_ITEM.format(
                    policyName=normalize_name(name),
                    policyRemovalReason=reason)
        return removed_policy_items

    def generate_infos(self):
        (num_entries, info_items) = self.generate_info_items(self.zones_map)
        removed_info_items = self.generate_removed_info_items(
            self.removed_zones)
        info_map_items = self.generate_info_map_items(self.zones_map)

        return self.ZONE_INFOS_FILE.format(
            invocation=self.invocation,
            tz_version=self.tz_version,
            tz_files=', '.join(self.tz_files),
            numInfos=len(self.zones_map),
            numEntries=num_entries,
            infoItems=info_items,
            infoMapItems=info_map_items,
            numRemovedInfos=len(self.removed_zones),
            removedInfoItems=removed_info_items)

    def generate_info_items(self, zones_map):
        info_items = ''
        num_entries = 0
        for name, zones in sorted(self.zones_map.items()):
            info_items += self.generate_info_item(name, zones)
            num_entries += len(zones)
        return (num_entries, info_items)

    def generate_removed_info_items(self, removed_zones):
        removed_info_items = ''
        for name, reason in sorted(self.removed_zones.items()):
            removed_info_items += self.ZONE_REMOVED_INFO_ITEM.format(
                infoFullName=name,
                infoRemovalReason=reason)
        return removed_info_items

    def generate_info_item(self, name, entries):
        entry_items = ''
        for entry in entries:
            entry_items += self.generate_entry_item(entry)

        return self.ZONE_INFO_ITEM.format(
            infoFullName=normalize_name(name),
            infoShortName=normalize_name(short_name(name)),
            numEntries=len(entries),
            entryItems=entry_items)

    def generate_entry_item(self, entry):
        policy_name = entry['rules']
        if policy_name == '-':
            zone_policy = 'None'
        else:
            zone_policy = 'ZONE_POLICY_%s' % policy_name

        until_year = entry['untilYear']
        if until_year == self.YEAR_MAX:
            until_year_short = self.YEAR_SHORT_MAX
        else:
            until_year_short = until_year - self.EPOCH_YEAR

        until_month = entry['untilMonth']
        if not until_month:
            until_month = 1

        until_day = entry['untilDay']
        if not until_day:
            until_day = 1

        until_hour = entry['untilHour']
        if not until_hour:
            until_hour = 0

        return self.ZONE_ENTRY_ITEM.format(
            rawLine=normalize_raw(entry['rawLine']),
            offsetCode=entry['offsetCode'],
            zonePolicy=zone_policy,
            format=entry['format'],
            untilYearShort=until_year_short,
            untilMonth=until_month,
            untilDay=until_day,
            untilHour=until_hour)

    def generate_info_map_items(self, zones_map):
        info_map_items = ''
        for name, zones in zones_map.items():
            info_map_items += self.ZONE_INFO_MAP_ITEM.format(
                infoShortName=normalize_name(short_name(name)))
        return info_map_items
