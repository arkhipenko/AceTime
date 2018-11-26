/*
 * A program to determine how long it takes to execute some of the more complex
 * methods of DateTime and LocalDate.
 *
 * This should compile on all microcontrollers supported by the Arduino IDE.
 */

#include <AceRoutine.h> // activate SystemTime coroutines
#include <AceTime.h>
#include "Benchmark.h"

using namespace ace_time;
using namespace ace_time::provider;
using namespace ace_time::common;

void setup() {
  delay(1000);
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro
  pinMode(LED_BENCHMARK, OUTPUT);

  // ace_time primitives

  Serial.print(F("sizeof(ZoneEntry): "));
  Serial.println(sizeof(ZoneEntry));

  Serial.print(F("sizeof(ZoneInfo): "));
  Serial.println(sizeof(ZoneInfo));

  Serial.print(F("sizeof(ZoneRule): "));
  Serial.println(sizeof(ZoneRule));

  Serial.print(F("sizeof(ZonePolicy): "));
  Serial.println(sizeof(ZonePolicy));

  Serial.print(F("sizeof(LocalDate): "));
  Serial.println(sizeof(LocalDate));

  Serial.print(F("sizeof(LocalTime): "));
  Serial.println(sizeof(LocalTime));

  Serial.print(F("sizeof(UtcOffset): "));
  Serial.println(sizeof(UtcOffset));

  Serial.print(F("sizeof(ZoneMatch): "));
  Serial.println(sizeof(ZoneMatch));

  Serial.print(F("sizeof(ZoneManager): "));
  Serial.println(sizeof(ZoneManager));

  Serial.print(F("sizeof(TimeZone): "));
  Serial.println(sizeof(TimeZone));

  Serial.print(F("sizeof(OffsetDateTime): "));
  Serial.println(sizeof(OffsetDateTime));

  Serial.print(F("sizeof(DateTime): "));
  Serial.println(sizeof(DateTime));

  Serial.print(F("sizeof(TimePeriod): "));
  Serial.println(sizeof(TimePeriod));

  // ace_time::provider classes

  Serial.print(F("sizeof(SystemTimeKeeper): "));
  Serial.println(sizeof(SystemTimeKeeper));

  Serial.print(F("sizeof(DS3231TimeKeeper): "));
  Serial.println(sizeof(DS3231TimeKeeper));

#if defined(ESP8266) || defined(ESP32)
  Serial.print(F("sizeof(NtpTimeProvider): "));
  Serial.println(sizeof(NtpTimeProvider));
#endif

  Serial.print(F("sizeof(SystemTimeSyncLoop): "));
  Serial.println(sizeof(SystemTimeSyncLoop));

  Serial.print(F("sizeof(SystemTimeHeartbeatLoop): "));
  Serial.println(sizeof(SystemTimeHeartbeatLoop));

  Serial.print(F("sizeof(SystemTimeSyncCoroutine): "));
  Serial.println(sizeof(SystemTimeSyncCoroutine));

  Serial.print(F("sizeof(SystemTimeHeartbeatCoroutine): "));
  Serial.println(sizeof(SystemTimeHeartbeatCoroutine));

  runBenchmarks();
}

void loop() {
}
