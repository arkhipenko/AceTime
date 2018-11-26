#include <string.h> // strlen()
#include "common/Util.h"
#include "common/DateStrings.h"
#include "TimeZone.h"

namespace ace_time {

using common::printPad2;

const TimeZone TimeZone::sUtc;

void TimeZone::printTo(Print& printer) const {
  if (mType == kTypeFixed) {
    printer.print(F("UTC"));
    mUtcOffset.printTo(printer);
    printer.print(mIsDst ? F(" (DST)") : F(" (STD)"));
  } else {
    printer.print('[');
    printer.print(mZoneManager.getZoneInfo()->name);
    printer.print(']');
  }
}

void TimeZone::parseFromOffsetString(const char* ts,
    uint8_t* offsetCode, bool* isDst) {

  // verify exact ISO 8601 string length
  if (strlen(ts) != kUtcOffsetStringLength) {
    *offsetCode = UtcOffset::kErrorCode;
    *isDst = false;
    return;
  }

  // '+' or '-'
  char utcSign = *ts++;
  int8_t sign;
  if (utcSign == '-') {
    sign = -1;
  } else if (utcSign == '+') {
    sign = 1;
  } else {
    *offsetCode = UtcOffset::kErrorCode;
    *isDst = false;
    return;
  }

  // hour
  uint8_t hour = (*ts++ - '0');
  hour = 10 * hour + (*ts++ - '0');

  // ':'
  ts++;

  // minute
  uint8_t minute = (*ts++ - '0');
  minute = 10 * minute + (*ts++ - '0');

  *offsetCode = UtcOffset::forHourMinute(sign, hour, minute).toOffsetCode();

  // TODO: parse the DST from the string
  *isDst = false;
}

}
