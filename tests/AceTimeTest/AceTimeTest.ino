#line 2 "AceTimeTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;
using namespace ace_time::common;

// --------------------------------------------------------------------------
// OffsetDateTime
// --------------------------------------------------------------------------

test(daysAndSecondsSinceEpochAt2000_01_01) {
  // 2000-01-01 00:00:00Z
  OffsetDateTime dt = OffsetDateTime::forComponents(0, 1, 1, 0, 0, 0);

  uint32_t daysSinceEpoch = dt.toDaysSinceEpoch();
  assertEqual((uint32_t) 0, daysSinceEpoch);

  uint32_t secondsSinceEpoch = dt.toSecondsSinceEpoch();
  assertEqual((uint32_t) 0, secondsSinceEpoch);

  assertEqual(7, dt.dayOfWeek());
}

// 2000-02-29 was a leap year, due to the every 400 year rule
// 2100-02-29 is *not* a leap year, due to the every 100 year rule
test(daysAndSecondsSinceEpochAt2000_02_29) {
  // 2000-02-29 00:00:00Z
  OffsetDateTime dt = OffsetDateTime::forComponents(0, 2, 29, 0, 0, 0);

  uint32_t daysSinceEpoch = dt.toDaysSinceEpoch();
  assertEqual((uint32_t) 59, daysSinceEpoch);

  uint32_t secondsSinceEpoch = dt.toSecondsSinceEpoch();
  assertEqual((uint32_t) 86400 * 59, secondsSinceEpoch);

  assertEqual(3, dt.dayOfWeek());
}

test(daysAndSecondsSinceEpochAt2000_01_02) {
  // 2000-01-02 00:00:00Z
  OffsetDateTime dt = OffsetDateTime::forComponents(0, 1, 2, 0, 0, 0);

  uint32_t daysSinceEpoch = dt.toDaysSinceEpoch();
  assertEqual((uint32_t) 1, daysSinceEpoch);

  uint32_t secondsSinceEpoch = dt.toSecondsSinceEpoch();
  assertEqual((uint32_t) 86400, secondsSinceEpoch);

  assertEqual(1, dt.dayOfWeek());
}

test(daysAndSecondsSinceEpochAt2018_01_01) {
  // 2018-01-01 00:00:00Z
  OffsetDateTime dt = OffsetDateTime::forComponents(18, 1, 1, 0, 0, 0);

  uint32_t daysSinceEpoch = dt.toDaysSinceEpoch();
  assertEqual((uint32_t) 6575, daysSinceEpoch);

  uint32_t secondsSinceEpoch = dt.toSecondsSinceEpoch();
  assertEqual(6575 * (uint32_t) 86400, secondsSinceEpoch);

  assertEqual(2, dt.dayOfWeek());
}

test(daysAndSecondsSinceEpochAt2018_01_01WithTimeZone) {
  // 2018-01-01 00:00:00+00:15
  OffsetDateTime dt = OffsetDateTime::forComponents(18, 1, 1, 0, 0, 0,
      ZoneOffset::forOffsetCode(1));

  uint32_t daysSinceEpoch = dt.toDaysSinceEpoch();
  assertEqual((uint32_t) 6574, daysSinceEpoch);

  uint32_t secondsSinceEpoch = dt.toSecondsSinceEpoch();
  assertEqual(6575 * (uint32_t) 86400 - 15*60, secondsSinceEpoch);

  assertEqual(2, dt.dayOfWeek()); // 2018-01-01 is a Monday
}

test(daysAndSecondsSinceEpochAt2049_12_31) {
  // 2049-12-31 23:59:59Z
  OffsetDateTime dt = OffsetDateTime::forComponents(49, 12, 31, 23, 59, 59);

  uint32_t daysSinceEpoch = dt.toDaysSinceEpoch();
  assertEqual((uint32_t) 18262, daysSinceEpoch);

  uint32_t secondsSinceEpoch = dt.toSecondsSinceEpoch();
  assertEqual(18263 * (uint32_t) 86400 - 1, secondsSinceEpoch);

  assertEqual(6, dt.dayOfWeek());
}

test(daysAndSecondsSinceEpochAt2049_12_31WithTimeZone) {
  // 2049-12-31 23:59:59-00:15
  OffsetDateTime dt = OffsetDateTime::forComponents(49, 12, 31, 23, 59, 59,
      ZoneOffset::forOffsetCode(-1));

  uint32_t daysSinceEpoch = dt.toDaysSinceEpoch();
  assertEqual((uint32_t) 18263, daysSinceEpoch);

  uint32_t secondsSinceEpoch = dt.toSecondsSinceEpoch();
  assertEqual(18263 * (uint32_t) 86400 + 15*60 - 1, secondsSinceEpoch);

  assertEqual(6, dt.dayOfWeek()); // 2049-12-31 is a Friday
}

test(daysAndSecondsSinceEpochAt2050_01_01) {
  // 2050-01-01 00:00:00Z
  OffsetDateTime dt = OffsetDateTime::forComponents(50, 1, 1, 0, 0, 0);

  uint32_t daysSinceEpoch = dt.toDaysSinceEpoch();
  assertEqual((uint32_t) 18263, daysSinceEpoch);

  uint32_t secondsSinceEpoch = dt.toSecondsSinceEpoch();
  assertEqual(18263 * (uint32_t) 86400, secondsSinceEpoch);

  assertEqual(7, dt.dayOfWeek()); // Saturday
}

test(daysAndSecondsSinceEpochAt2099_12_31) {
  // 2099-12-31 23:59:59Z
  OffsetDateTime dt = OffsetDateTime::forComponents(99, 12, 31, 23, 59, 59);

  uint32_t daysSinceEpoch = dt.toDaysSinceEpoch();
  assertEqual((uint32_t) 36524, daysSinceEpoch);

  uint32_t secondsSinceEpoch = dt.toSecondsSinceEpoch();
  assertEqual(36525 * (uint32_t) 86400 - 1, secondsSinceEpoch);

  assertEqual(5, dt.dayOfWeek()); // Thursday
}

test(unixSeconds) {
  // 2000-01-01 00:00:00Z
  OffsetDateTime dt = OffsetDateTime::forComponents(0, 1, 1, 0, 0, 0);
  assertEqual((uint32_t) 946684800, dt.toUnixSeconds());

  // 2018-01-01 00:00:00Z
  dt = OffsetDateTime::forComponents(18, 1, 1, 0, 0, 0);
  assertEqual((uint32_t) 1514764800, dt.toUnixSeconds());

  // 2018-08-30T06:45:01-07:00
  dt = OffsetDateTime::forComponents(18, 8, 30, 6, 45, 1,
      ZoneOffset::forHour(-7));
  assertEqual((uint32_t) 1535636701, dt.toUnixSeconds());

  // 2038-01-01 00:00:00Z
  dt = OffsetDateTime::forComponents(38, 1, 1, 0, 0, 0);
  assertEqual((uint32_t) 2145916800, dt.toUnixSeconds());

  // 2099-12-31 23:59:59-16:00
  dt = OffsetDateTime::forComponents(99, 12, 31, 23, 59, 59,
      ZoneOffset::forHour(-16));
  assertEqual((uint32_t) 4102502399, dt.toUnixSeconds());
}

test(constructFromSecondsSinceEpochAt2049_12_31) {
  // 2049-12-31 23:59:59Z
  OffsetDateTime dt = OffsetDateTime::forSeconds(18263 * (int32_t) 86400 - 1);

  assertEqual((uint16_t) 2049, dt.yearFull());
  assertEqual(49, dt.year());
  assertEqual(12, dt.month());
  assertEqual(31, dt.day());
  assertEqual(23, dt.hour());
  assertEqual(59, dt.minute());
  assertEqual(59, dt.second());
  assertEqual(6, dt.dayOfWeek());

  // 2049-12-31 15:59:59-08:00
  ZoneOffset offset = ZoneOffset::forOffsetCode(-32); // UTC-08:00
  dt = OffsetDateTime::forSeconds(18263 * (int32_t) 86400 - 1, offset);
  assertEqual((uint16_t) 2049, dt.yearFull());
  assertEqual(49, dt.year());
  assertEqual(12, dt.month());
  assertEqual(31, dt.day());
  assertEqual(15, dt.hour());
  assertEqual(59, dt.minute());
  assertEqual(59, dt.second());
  assertEqual(6, dt.dayOfWeek());
}

test(convertToZoneOffset) {
  OffsetDateTime a = OffsetDateTime::forComponents(18, 1, 1, 12, 0, 0);
  OffsetDateTime b = a.convertToZoneOffset(ZoneOffset::forHour(-7));

  assertEqual((uint16_t) 2018, b.yearFull());
  assertEqual(18, b.year());
  assertEqual(1, b.month());
  assertEqual(1, b.day());
  assertEqual(5, b.hour());
  assertEqual(0, b.minute());
  assertEqual(0, b.second());
  assertEqual(-28, b.zoneOffset().offsetCode());
}

test(dateTimeCompareAndEquals) {
  OffsetDateTime a = OffsetDateTime::forComponents(18, 1, 1, 12, 0, 0);
  OffsetDateTime b = OffsetDateTime::forComponents(18, 1, 1, 12, 0, 0);
  assertEqual(a.compareTo(b), 0);
  assertTrue(a == b);
  assertFalse(a != b);

  a = OffsetDateTime::forComponents(18, 1, 1, 12, 0, 0);
  b = OffsetDateTime::forComponents(18, 1, 1, 12, 0, 1);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = OffsetDateTime::forComponents(18, 1, 1, 12, 0, 0);
  b = OffsetDateTime::forComponents(18, 1, 1, 12, 1, 0);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = OffsetDateTime::forComponents(18, 1, 1, 11, 0, 0);
  b = OffsetDateTime::forComponents(18, 1, 1, 12, 0, 0,
      ZoneOffset::forOffsetCode(1));
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = OffsetDateTime::forComponents(18, 1, 1, 12, 0, 0);
  b = OffsetDateTime::forComponents(18, 1, 2, 12, 0, 0);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = OffsetDateTime::forComponents(18, 1, 1, 12, 0, 0);
  b = OffsetDateTime::forComponents(18, 2, 1, 12, 0, 0);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = OffsetDateTime::forComponents(18, 1, 1, 12, 0, 0);
  b = OffsetDateTime::forComponents(19, 1, 1, 12, 0, 0);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  // 2018-1-1 12:00:00+01:00
  a = OffsetDateTime::forComponents(18, 1, 1, 12, 0, 0, ZoneOffset::forHour(1));
  // 2018-1-1 12:00:00-08:00
  b = OffsetDateTime::forComponents(18, 1, 1, 12, 0, 0,
      ZoneOffset::forHour(-8));
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);
}

test(calculateAndCacheDayOfWeek) {
  // 2018-01-01 00:00:00Z
  OffsetDateTime dt = OffsetDateTime::forComponents(18, 1, 1, 0, 0, 0);
  assertEqual(2, dt.dayOfWeek()); // Monday

  dt.hour(23); // 2018-01-01 23:00:00Z, no change to dayOfWeek
  assertEqual(2, dt.dayOfWeek());

  dt.minute(40); // 2018-01-01 23:40:00Z, no change to dayOfWeek
  assertEqual(2, dt.dayOfWeek());

  dt.second(3); // 2018-01-01 23:40:03Z, no change to dayOfWeek
  assertEqual(2, dt.dayOfWeek());

  // 2018-01-01 23:40:03+00:45, no change to dayOfWeek
  dt.zoneOffset(ZoneOffset::forOffsetCode(3));
  assertEqual(2, dt.dayOfWeek());

  dt.day(2); // 2018-01-02 23:40:03+00:45, changes dayOfWeek
  assertEqual(3, dt.dayOfWeek());

  dt.month(2); // 2018-02-02 23:40:03+00:45, changes dayOfWeek
  assertEqual(6, dt.dayOfWeek()); // Friday

  dt.year(19); // 2019-02-02 23:40:03+00:45, changes dayOfWeek
  assertEqual(7, dt.dayOfWeek());

  dt.yearFull(2020); // 2020-02-02 23:40:03+00:45, changes dayOfWeek
  assertEqual(1, dt.dayOfWeek()); // Sunday
}

test(dateTimeErrorForZeroValue) {
  OffsetDateTime dt = OffsetDateTime::forSeconds(0);
  assertTrue(dt.isError());
}

test(dateTimeSetError) {
  OffsetDateTime dt = OffsetDateTime().setError();
  assertTrue(dt.isError());
}

test(dateTimeIsError) {
  // 2018-01-01 00:00:00Z
  OffsetDateTime dt = OffsetDateTime::forComponents(18, 1, 1, 0, 0, 0);
  assertFalse(dt.isError());

  dt = OffsetDateTime::forComponents(18, 0, 1, 0, 0, 0);
  assertTrue(dt.isError());

  dt = OffsetDateTime::forComponents(18, 255, 1, 0, 0, 0);
  assertTrue(dt.isError());

  dt = OffsetDateTime::forComponents(18, 1, 0, 0, 0, 0);
  assertTrue(dt.isError());

  dt = OffsetDateTime::forComponents(18, 1, 255, 0, 0, 0);
  assertTrue(dt.isError());

  dt = OffsetDateTime::forComponents(18, 1, 1, 255, 0, 0);
  assertTrue(dt.isError());

  dt = OffsetDateTime::forComponents(18, 1, 1, 0, 255, 0);
  assertTrue(dt.isError());

  dt = OffsetDateTime::forComponents(18, 1, 1, 0, 0, 255);
  assertTrue(dt.isError());
}

test(dateTimeForDateString_errors) {
  // empty string, too short
  OffsetDateTime dt = OffsetDateTime::forDateString("");
  assertTrue(dt.isError());

  // not enough components
  dt = OffsetDateTime::forDateString(F("2018-08-31"));
  assertTrue(dt.isError());

  // too long
  dt = OffsetDateTime::forDateString(F("2018-08-31T13:48:01-07:00X"));
  assertTrue(dt.isError());

  // too short
  dt = OffsetDateTime::forDateString(F("2018-08-31T13:48:01-07:0"));
  assertTrue(dt.isError());

  // missing UTC
  dt = OffsetDateTime::forDateString(F("2018-08-31T13:48:01"));
  assertTrue(dt.isError());

  // parser cares about UTC+/- offset
  dt = OffsetDateTime::forDateString(F("2018-08-31 13:48:01&07:00"));
  assertTrue(dt.isError());
}

test(dateTimForDateString) {
  // exact ISO8601 format
  OffsetDateTime dt = OffsetDateTime::forDateString(
      F("2018-08-31T13:48:01-07:00"));
  assertFalse(dt.isError());
  assertEqual((uint16_t) 2018, dt.yearFull());
  assertEqual(18, dt.year());
  assertEqual(8, dt.month());
  assertEqual(31, dt.day());
  assertEqual(13, dt.hour());
  assertEqual(48, dt.minute());
  assertEqual(1, dt.second());
  assertEqual(-28, dt.zoneOffset().offsetCode());
  assertEqual(6, dt.dayOfWeek());

  // parser does not care about most separators, this may change in the future
  dt = OffsetDateTime::forDateString(F("2018/08/31 13#48#01+07#00"));
  assertFalse(dt.isError());
  assertEqual(18, dt.year());
  assertEqual(8, dt.month());
  assertEqual(31, dt.day());
  assertEqual(13, dt.hour());
  assertEqual(48, dt.minute());
  assertEqual(1, dt.second());
  assertEqual(28, dt.zoneOffset().offsetCode());
  assertEqual(6, dt.dayOfWeek());
}

// --------------------------------------------------------------------------
// TimePeriod
// --------------------------------------------------------------------------

test(timePeriodFromComponents) {
  TimePeriod t(0, 16, 40, 1);
  assertEqual(t.toSeconds(), (int32_t) 1000);

  TimePeriod u(0, 16, 40, -1);
  assertEqual(u.toSeconds(), (int32_t) -1000);
}

test(timePeriodFromSeconds) {
  TimePeriod t(1000);
  assertEqual(0, t.hour());
  assertEqual(16, t.minute());
  assertEqual(40, t.second());
  assertEqual(1, t.sign());
  assertEqual((int32_t) 1000, t.toSeconds());

  TimePeriod large((int32_t) 100000);
  assertEqual(27, large.hour());
  assertEqual(46, large.minute());
  assertEqual(40, large.second());
  assertEqual(1, t.sign());
  assertEqual((int32_t) 100000, large.toSeconds());
}

test(timePeriodCompareAndEquals) {
  TimePeriod a(3, 2, 1, 1);
  TimePeriod b(3, 2, 1, 1);
  assertEqual(a.compareTo(b), 0);
  assertTrue(a == b);
  assertFalse(a != b);

  a = TimePeriod(3, 2, 1, 1);
  b = TimePeriod(3, 2, 2, 1);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = TimePeriod(3, 2, 1, 1);
  b = TimePeriod(3, 3, 1, 1);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = TimePeriod(3, 2, 1, 1);
  b = TimePeriod(4, 2, 1, 1);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = TimePeriod(3, 2, 1, -1);
  b = TimePeriod(3, 2, 1, 1);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);
}

test(timePeriodIncrementHour) {
  TimePeriod a(3, 2, 1, 1);
  a.incrementHour();
  TimePeriod expected(4, 2, 1, 1);
  assertTrue(expected == a);

  a = TimePeriod(23, 2, 1, -1);
  a.incrementHour();
  expected = TimePeriod(0, 2, 1, -1);
  assertTrue(expected == a);
}

test(timePeriodIncrementMinute) {
  TimePeriod a(3, 2, 1, 1);
  a.incrementMinute();
  TimePeriod expected(3, 3, 1, 1);
  assertTrue(expected == a);

  a = TimePeriod(3, 59, 1, 1);
  a.incrementMinute();
  expected = TimePeriod(3, 0, 1, 1);
  assertTrue(expected == a);
}

test(timePeriodNegate) {
  TimePeriod a(3, 2, 1, 1);
  assertEqual((int8_t) 1, a.sign());
  a.negate();
  assertEqual((int8_t) -1, a.sign());
}

// --------------------------------------------------------------------------
// ZoneOffset
// --------------------------------------------------------------------------

test(zoneOffsetAsMinutesAsSeconds) {
  ZoneOffset offset = ZoneOffset::forOffsetCode(-1);
  assertEqual((int16_t) -15, offset.asMinutes());
  assertEqual((int32_t) -900, offset.asSeconds());

  offset = ZoneOffset::forOffsetCode(1);
  assertEqual((int16_t) 15, offset.asMinutes());
  assertEqual((int32_t) 900, offset.asSeconds());
}

test(zoneOffsetForHour) {
  assertEqual(ZoneOffset::forHour(-8).offsetCode(), -32);
  assertEqual(ZoneOffset::forHour(1).offsetCode(), 4);
}

test(zoneOffsetForHourMinute) {
  assertEqual(ZoneOffset::forHourMinute(-1, 8, 0).offsetCode(), -32);
  assertEqual(ZoneOffset::forHourMinute(-1, 8, 15).offsetCode(), -33);
  assertEqual(ZoneOffset::forHourMinute(1, 1, 0).offsetCode(), 4);
  assertEqual(ZoneOffset::forHourMinute(1, 1, 15).offsetCode(), 5);
}

test(zoneOffsetForOffsetString) {
  assertTrue(ZoneOffset::forOffsetString("").isError());
  assertEqual(ZoneOffset::forOffsetString("-07:00").offsetCode(), -28);
  assertEqual(ZoneOffset::forOffsetString("-07:45").offsetCode(), -31);
  assertEqual(ZoneOffset::forOffsetString("+01:00").offsetCode(), 4);
  assertEqual(ZoneOffset::forOffsetString("+01:15").offsetCode(), 5);
  assertEqual(ZoneOffset::forOffsetString("+01:16").offsetCode(), 5);
}

test(zoneOffsetError) {
  ZoneOffset offset;
  assertFalse(offset.isError());

  offset.setError();
  assertTrue(offset.isError());
}

test(zoneOffsetIncrementHour) {
  ZoneOffset offset = ZoneOffset::forOffsetCode(-1);
  offset.incrementHour();
  assertEqual((int8_t) 3, offset.offsetCode());

  offset = ZoneOffset::forOffsetCode(63);
  offset.incrementHour();
  assertEqual((int8_t) -61, offset.offsetCode());

  offset = ZoneOffset::forOffsetCode(60);
  offset.incrementHour();
  assertEqual((int8_t) -64, offset.offsetCode());
}

test(zoneOffsetIncrement15Minutes) {
  ZoneOffset offset = ZoneOffset::forOffsetCode(3);

  offset.increment15Minutes();
  assertEqual((int8_t) 0, offset.offsetCode());

  offset.increment15Minutes();
  assertEqual((int8_t) 1, offset.offsetCode());

  offset.increment15Minutes();
  assertEqual((int8_t) 2, offset.offsetCode());

  offset.increment15Minutes();
  assertEqual((int8_t) 3, offset.offsetCode());

  offset = ZoneOffset::forOffsetCode(-4);
  offset.increment15Minutes();
  assertEqual((int8_t) -5, offset.offsetCode());

  offset.increment15Minutes();
  assertEqual((int8_t) -6, offset.offsetCode());

  offset.increment15Minutes();
  assertEqual((int8_t) -7, offset.offsetCode());

  offset.increment15Minutes();
  assertEqual((int8_t) -4, offset.offsetCode());
}

test(zoneOffsetConvertOffsetCode) {
  ZoneOffset zoneOffset = ZoneOffset::forOffsetCode(-29);
  int8_t sign;
  uint8_t hour;
  uint8_t minute;
  zoneOffset.asHourMinute(sign, hour, minute);
  assertEqual(-1, sign);
  assertEqual(7, hour);
  assertEqual(15, minute);
}

// --------------------------------------------------------------------------
// TimeZone
// --------------------------------------------------------------------------

test(timeZoneDstOffset) {
  TimeZone pdt = TimeZone::forZoneOffset(ZoneOffset::forHour(-8)).isDst(true);
  assertTrue(pdt.isDst());
  assertEqual(pdt.zoneOffset().offsetCode(), -32);
  assertEqual(pdt.effectiveZoneOffset(0).offsetCode(), -28);
}

test(timeZoneSetError) {
  TimeZone error = TimeZone::forZoneOffset(ZoneOffset::forHour(-8)).setError();
  assertTrue(error.isError());
}

// --------------------------------------------------------------------------
// DateStrings
// --------------------------------------------------------------------------

test(monthString) {
  DateStrings ds;

  assertEqual(F("Error"), ds.monthLongString(0));
  assertEqual(F("January"), ds.monthLongString(1));
  assertEqual(F("February"), ds.monthLongString(2));
  assertEqual(F("March"), ds.monthLongString(3));
  assertEqual(F("April"), ds.monthLongString(4));
  assertEqual(F("May"), ds.monthLongString(5));
  assertEqual(F("June"), ds.monthLongString(6));
  assertEqual(F("July"), ds.monthLongString(7));
  assertEqual(F("August"), ds.monthLongString(8));
  assertEqual(F("September"), ds.monthLongString(9));
  assertEqual(F("October"), ds.monthLongString(10));
  assertEqual(F("November"), ds.monthLongString(11));
  assertEqual(F("December"), ds.monthLongString(12));
  assertEqual(F("Error"), ds.monthLongString(13));

  assertEqual(F("Err"), ds.monthShortString(0));
  assertEqual(F("Jan"), ds.monthShortString(1));
  assertEqual(F("Feb"), ds.monthShortString(2));
  assertEqual(F("Mar"), ds.monthShortString(3));
  assertEqual(F("Apr"), ds.monthShortString(4));
  assertEqual(F("May"), ds.monthShortString(5));
  assertEqual(F("Jun"), ds.monthShortString(6));
  assertEqual(F("Jul"), ds.monthShortString(7));
  assertEqual(F("Aug"), ds.monthShortString(8));
  assertEqual(F("Sep"), ds.monthShortString(9));
  assertEqual(F("Oct"), ds.monthShortString(10));
  assertEqual(F("Nov"), ds.monthShortString(11));
  assertEqual(F("Dec"), ds.monthShortString(12));
  assertEqual(F("Err"), ds.monthShortString(13));
}

test(monthStringsFitInBuffer) {
  DateStrings ds;
  uint8_t maxLength = 0;
  for (uint8_t month = 0; month <= 12; month++) {
    const char* monthString = ds.monthLongString(month);
    uint8_t length = strlen(monthString);
    if (length > maxLength) { maxLength = length; }
  }
  assertLessOrEqual(maxLength, DateStrings::kBufferSize - 1);
}

test(weekDayStrings) {
  DateStrings ds;

  assertEqual(F("Error"), ds.weekDayLongString(0));
  assertEqual(F("Sunday"), ds.weekDayLongString(1));
  assertEqual(F("Monday"), ds.weekDayLongString(2));
  assertEqual(F("Tuesday"), ds.weekDayLongString(3));
  assertEqual(F("Wednesday"), ds.weekDayLongString(4));
  assertEqual(F("Thursday"), ds.weekDayLongString(5));
  assertEqual(F("Friday"), ds.weekDayLongString(6));
  assertEqual(F("Saturday"), ds.weekDayLongString(7));
  assertEqual(F("Error"), ds.weekDayLongString(8));

  assertEqual(F("Err"), ds.weekDayShortString(0));
  assertEqual(F("Sun"), ds.weekDayShortString(1));
  assertEqual(F("Mon"), ds.weekDayShortString(2));
  assertEqual(F("Tue"), ds.weekDayShortString(3));
  assertEqual(F("Wed"), ds.weekDayShortString(4));
  assertEqual(F("Thu"), ds.weekDayShortString(5));
  assertEqual(F("Fri"), ds.weekDayShortString(6));
  assertEqual(F("Sat"), ds.weekDayShortString(7));
  assertEqual(F("Err"), ds.weekDayShortString(8));
}

test(weekDayStringsFitInBuffer) {
  DateStrings ds;
  uint8_t maxLength = 0;
  for (uint8_t weekDay = 0; weekDay <= 7; weekDay++) {
    const char* weekDayString = ds.weekDayLongString(weekDay);
    uint8_t length = strlen(weekDayString);
    if (length > maxLength) { maxLength = length; }
  }
  assertLessOrEqual(maxLength, DateStrings::kBufferSize - 1);
}

// --------------------------------------------------------------------------

void setup() {
  delay(1000); // wait for stability on some boards to prevent garbage Serial
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while(!Serial); // for the Arduino Leonardo/Micro only
}

void loop() {
  TestRunner::run();
}
