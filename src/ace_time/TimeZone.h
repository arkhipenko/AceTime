#ifndef ACE_TIME_TIME_ZONE_H
#define ACE_TIME_TIME_ZONE_H

#include <stdint.h>

class Print;

namespace ace_time {

/**
 * A thin object wrapper around an integer (int8_t) time zone code which
 * represents the time offset from UTC in 15 minute increments, and a daylight
 * saving time flag (bool) that indicates whether the time zone is currently in
 * DST mode.
 *
 * For example, Pacific Standard Time is UTC-08:00, which is encoded
 * internally as an offset code of -32 and the TimeZone object can be created
 * using the constructor TimeZone(-32) or the factory method
 * TimeZone::forHour(-8). When the PST time goes into DST mode, setting the
 * isDst(true) causes the time zone object to return UTC offsets which includes
 * the DST shift. When the isDst flag is set, the "effective" UTC offset
 * returned by effectiveOffsetCode(), asEffectiveOffsetMinutes() and
 * asEffectiveOffsetSeconds() will include the DST shift.
 *
 * Here is the TimeZone object that represents Pacific Daylight Time:
 * @code
 * TimeZone tz = TimeZone::forHour(-8).isDst(true);
 * int16_t minutes = tz.asEffectiveOffsetMinutes(); // returns -420, not -480
 * @endcode
 *
 * According to https://en.wikipedia.org/wiki/List_of_UTC_time_offsets, all
 * time zones currently in use occur at 15 minute boundaries, and the smallest
 * time zone is UTC-12:00 and the biggest time zone is UTC+14:00. Therefore,
 * we can encode all currently used time zones as integer multiples of
 * 15-minute offsets from UTC. Some locations may observe daylight saving time,
 * so the actual range of the offset in practice may be UTC-12:00 to UTC+15:00.
 *
 * This class does NOT know about the "tz database" (aka Olson database)
 * https://en.wikipedia.org/wiki/Tz_database. Therefore, it does not know about
 * symbolic time zones (e.g. "America/Los_Angeles"). It also does not know
 * about when daylight saving time (DST) starts and ends for specific time
 * zones.
 *
 * It may be relatively easy to extend this class to accept an object that
 * encodes the DST rule for one specific time zone (instead of using the Olson
 * database which includes every time zone ever used). This would allow us to
 * avoid forcing the user to change the DST flag manually twice a year.
 */
class TimeZone {
  public:
    /**
     * Create TimeZone from the offset code.
     *
     * @param offsetCode the number of 15-minute offset from UTC. 0 means UTC.
     */
    static TimeZone forOffsetCode(int8_t offsetCode) {
      return TimeZone().initFromOffsetCode(offsetCode);
    }

    /**
     * Create TimeZone from integer hour offset from UTC. For example,
     * UTC-08:00 is 'forHour(-8)'.
     */
    static TimeZone forHour(int8_t hour) {
      return TimeZone::forOffsetCode(hour * 4);
    }

    /**
     * Create TimeZone from (sign, hour, minute) offset from UTC, where 'sign'
     * is either -1 or +1. The 'minute' must be in multiples of 15-minutes. For
     * example, UTC-07:30 is 'forHourMinute(-1, 7, 30)'.
     */
    static TimeZone forHourMinute(int8_t sign, uint8_t hour, uint8_t minute) {
      uint8_t code = hour * 4 + minute / 15;
      return (sign < 0)
          ? TimeZone::forOffsetCode(-code)
          : TimeZone::forOffsetCode(code);
    }

    /**
     * Create from UTC offset string ("-07:00" or "+01:00"). Intended mostly
     * for testing purposes.
     */
    static TimeZone forOffsetString(const char* tzString) {
      return TimeZone().initFromOffsetString(tzString);
    }

    /**
     * Constructor. Create a time zone corresponding to UTC with no offset.
     */
    explicit TimeZone() {}

    /**
     * Return the UTC offset as the number of 15 minute increments, excluding
     * DST shift.
     */
    int8_t offsetCode() const { return mOffsetCode; }

    /**
     * Return the effective UTC offset as the number of 15 minute offset,
     * including DST shift.
     */
    int8_t effectiveOffsetCode() const {
      return mOffsetCode + (mIsDst ? 4 : 0);
    }

    /** Return the DST mode setting. */
    bool isDst() const { return mIsDst; }

    /**
     * Set the current DST mode. Returns the reference to 'this' pointer,
     * which allows chaining to set the DST flag when creating this object.
     * For example, to create a time zone for Pacific Daylight Time, you
     * can write 'TimeZone tz = TimeZone::forHour(-8).isDst(true);'.
     */
    TimeZone& isDst(bool status) {
      mIsDst = status;
      return *this;
    }

    /** Return the number of minutes offset from UTC, excluding the DST mode. */
    int16_t asStandardOffsetMinutes() const {
      return (int16_t) 15 * mOffsetCode;
    }

    /** Return the number of minutes offset from UTC, excluding the DST mode. */
    int32_t asStandardOffsetSeconds() const {
      return (int32_t) 60 * asStandardOffsetMinutes();
    }

    /** Return the number of minutes offset from UTC, including the DST mode. */
    int16_t asEffectiveOffsetMinutes() const {
      return (int16_t) 15 * effectiveOffsetCode();
    }

    /** Return the number of seconds offset from UTC, including the DST mode. */
    int32_t asEffectiveOffsetSeconds() const {
      return (int32_t) 60 * asEffectiveOffsetMinutes();
    }

    /**
     * Increment the time zone by one hour (+4 in offsetCode). For usability,
     * incrementing a time zone code of +63 (UTC+15:45) by one wraps to -64
     * (UTC-16:00).
     */
    void incrementHour() {
      mOffsetCode += 4;
      if (mOffsetCode >= 64) {
        mOffsetCode = mOffsetCode - 128;
      }
    }

    /**
     * Increment time zone by one zone (i.e. 15 minutes) keeping the hour
     * component unchanged. If the offsetCode is negative, the cycle looks like:
     * (-01:00, -01:15, -01:30, -01:45, -01:00, ...).
     */
    void increment15Minutes() {
      uint8_t offsetCode = (mOffsetCode < 0) ? -mOffsetCode : mOffsetCode;
      offsetCode = (offsetCode & 0xFC) | (((offsetCode & 0x03) + 1) & 0x03);
      mOffsetCode = (mOffsetCode < 0) ? -offsetCode : offsetCode;
    }

    /**
     * Extract the (sign, hour, minute) components of the time zone, excluding
     * the DST mode.
     */
    void extractStandardHourMinute(int8_t& sign, uint8_t& hour,
        uint8_t& minute) const {
      convertOffsetCodeToHourMinute(mOffsetCode, sign, hour, minute);
    }

    /**
     * Extract the (sign, hour, minute) components of the time zone, including
     * the DST mode.
     */
    void extractEffectiveHourMinute(int8_t& sign, uint8_t& hour,
        uint8_t& minute) const {
      convertOffsetCodeToHourMinute(effectiveOffsetCode(), sign, hour, minute);
    }

    /**
     * Mark the TimeZone so that isError() returns true. An invalid TimeZone can
     * be returned using 'return TimeZone().setError()'. The compiler will
     * optimize away all the apparent method calls.
     */
    TimeZone& setError() {
      mOffsetCode = kTimeZoneErrorCode;
      return *this;
    }

    /** Return true if this TimeZone represents an error. */
    bool isError() const {
      return mOffsetCode == kTimeZoneErrorCode;
    }

    /**
     * Print the effective UTC offset (including DST shift) in a human-readable
     * form that looks like "+08:00". This is used mostly by the
     * DateTime.printTo() method when printing the DateTime in ISO8601 format.
     */
    void printEffectiveOffsetTo(Print& printer) const;

    /**
     * Print the human readable representation of the time zone as offset from
     * UTC, with an indicator of the current DST mode. The printed UTC offset
     * is the standard (i.e. base) UTC offset of the time zone, not the UTC
     * offset that includes the DST shift. In other words, a time zone of
     * UTC-08:00 whose DST is enabled (i.e. Pacific Daylight Time) is printed
     * as "UTC-08:00 DST", instead of "UTC-07:00 DST".
     *
     * In some ways, displaying the Standard UTC offset could be confusing
     * because the actual UTC offset used for DateTime calculations is not what
     * is displayed. On the other hand, displaying the effective UTC offset
     * causes confusion in other ways, for example, when the user is prompted
     * to set their current time zone, because it is not clear if the DST flag
     * describes the UTC offset that they just entered, or whether it shifts
     * the UTC offset that they just entered.
     *
     * Use the printEffectiveOffsetTo() method to print the UTC offset that
     * includes the DST shift.
     */
    void printTo(Print& printer) const;

  private:
    /** Sentinel value that represents an error. */
    static const int8_t kTimeZoneErrorCode = -128;

    /** Length of UTC offset string (e.g. "-07:00", "+01:30"). */
    static const uint8_t kTimeZoneLength = 6;

    friend bool operator==(const TimeZone& a, const TimeZone& b);
    friend bool operator!=(const TimeZone& a, const TimeZone& b);

    /** Set time zone from the given UTC offset string. */
    TimeZone& initFromOffsetString(const char* offsetString);

    /** Set time zone from offset code. */
    TimeZone& initFromOffsetCode(int8_t offsetCode) {
      mOffsetCode = offsetCode;
      return *this;
    }

    /** Helper method to convert a offsetCode to (sign, hour, minute) triple. */
    static void convertOffsetCodeToHourMinute(int8_t offsetCode, int8_t& sign,
        uint8_t& hour, uint8_t& minute) {
      sign = (offsetCode < 0) ? -1 : 1;
      uint8_t code = (offsetCode < 0) ? -offsetCode : offsetCode;
      hour = code / 4;
      minute = (code & 0x03) * 15;
    }

    /**
     * Time zone code, offset from UTC in 15 minute increments from UTC. In
     * theory, the code can range from [-128, 127]. But the value of -128 is
     * used to represent an internal error, causing isError() to return true
     * so the valid range is [-127, 127].
     *
     * The actual range of time zones used in real life values are expected to
     * be smaller, probably smaller than the range of [-64, 63], i.e. [-16:00,
     * +15:45].
     */
    int8_t mOffsetCode = 0;

    /**
     * Indicate whether Daylight Saving Time is in effect. If true, then
     * asEffectiveOffsetMinutes() and asEffectiveOffsetSeconds() will be
     * increased by 1h.
     */
    bool mIsDst = false;
};

inline bool operator==(const TimeZone& a, const TimeZone& b) {
  return a.mOffsetCode == b.mOffsetCode
      && a.mIsDst == b.mIsDst;
}

inline bool operator!=(const TimeZone& a, const TimeZone& b) {
  return ! (a == b);
}

}

#endif
