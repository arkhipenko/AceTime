#line 2 "BasicZoneRegistrarTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using aunit::TestRunner;
using namespace ace_time;
using ace_time::basic::ZoneRegistrar;

//---------------------------------------------------------------------------
// Define some registries used later on.
//---------------------------------------------------------------------------

const uint16_t kNumSortedEntries = 4;

const basic::ZoneInfo* const kSortedRegistry[kNumSortedEntries]
    ACE_TIME_PROGMEM = {
  &zonedb::kZoneAmerica_New_York,     // 0x1e2a7654
  &zonedb::kZoneAmerica_Chicago,      // 0x4b92b5d4
  &zonedb::kZoneAmerica_Denver,       // 0x97d10b2a
  &zonedb::kZoneAmerica_Los_Angeles,  // 0xb7f7e8f2
};

const uint16_t kNumUnsortedEntries = 4;

const basic::ZoneInfo* const kUnsortedRegistry[kNumUnsortedEntries]
    ACE_TIME_PROGMEM = {
  &zonedb::kZoneAmerica_Chicago,      // 0x4b92b5d4
  &zonedb::kZoneAmerica_Denver,       // 0x97d10b2a
  &zonedb::kZoneAmerica_Los_Angeles,  // 0xb7f7e8f2
  &zonedb::kZoneAmerica_New_York,     // 0x1e2a7654
};

//---------------------------------------------------------------------------

// Verify that we can use kZoneIdAmerica_Los_Angeles everywhere.
test(BasicZoneRegistrarTest, kZoneId) {
  assertEqual((uint32_t) 0xb7f7e8f2, zonedb::kZoneIdAmerica_Los_Angeles);
}

//---------------------------------------------------------------------------
// ZoneRegistrar public methods
//---------------------------------------------------------------------------

test(BasicZoneRegistrarTest, zoneRegistrySize) {
  ZoneRegistrar zoneRegistrar(zonedb::kZoneRegistrySize, zonedb::kZoneRegistry);
  assertEqual(zonedb::kZoneRegistrySize, zoneRegistrar.zoneRegistrySize());
}

test(BasicZoneRegistrarTest, getZoneInfo_Los_Angeles) {
  ZoneRegistrar zoneRegistrar(zonedb::kZoneRegistrySize, zonedb::kZoneRegistry);

  const basic::ZoneInfo* zoneInfo =
      zoneRegistrar.getZoneInfoForName("America/Los_Angeles");
  assertNotEqual(zoneInfo, nullptr);

  ace_common::PrintStr<32> printStr;
  BasicZone(zoneInfo).printNameTo(printStr);
  assertEqual(F("America/Los_Angeles"), printStr.getCstr());

  printStr.flush();
  BasicZone(zoneInfo).printShortNameTo(printStr);
  assertEqual(F("Los_Angeles"), printStr.getCstr());
}

// Test a zone without separators, "EST".
test(BasicZoneRegistrarTest, getZoneInfo_EST) {
  ZoneRegistrar zoneRegistrar(zonedb::kZoneRegistrySize, zonedb::kZoneRegistry);

  const basic::ZoneInfo* zoneInfo = zoneRegistrar.getZoneInfoForName("EST");
  assertNotEqual(zoneInfo, nullptr);

  ace_common::PrintStr<32> printStr;
  BasicZone(zoneInfo).printNameTo(printStr);
  assertEqual(F("EST"), printStr.getCstr());

  printStr.flush();
  BasicZone(zoneInfo).printShortNameTo(printStr);
  assertEqual(F("EST"), printStr.getCstr());
}

test(BasicZoneRegistrarTest, getZoneInfo_not_found) {
  ZoneRegistrar zoneRegistrar(zonedb::kZoneRegistrySize, zonedb::kZoneRegistry);
  const basic::ZoneInfo* zoneInfo = zoneRegistrar.getZoneInfoForName(
      "doesNotExist");
  assertEqual(zoneInfo, nullptr);
}

test(BasicZoneRegistrarTest, getZoneInfo_Index_not_found) {
  ZoneRegistrar zoneRegistrar(zonedb::kZoneRegistrySize, zonedb::kZoneRegistry);
  const basic::ZoneInfo* zoneInfo = zoneRegistrar.getZoneInfoForIndex(
      zonedb::kZoneRegistrySize);
  assertEqual(zoneInfo, nullptr);
}

test(BasicZoneRegistrarTest, getZoneInfoForId) {
  ZoneRegistrar zoneRegistrar(zonedb::kZoneRegistrySize, zonedb::kZoneRegistry);
  const basic::ZoneInfo* zoneInfo = zoneRegistrar.getZoneInfoForId(
      zonedb::kZoneIdAmerica_Los_Angeles);
  const basic::ZoneInfo* zoneInfoExpected = zoneRegistrar.getZoneInfoForName(
      "America/Los_Angeles");
  assertEqual(zoneInfo, zoneInfoExpected);
}

test(BasicZoneRegistrarTest, getZoneInfoForId_not_found) {
  ZoneRegistrar zoneRegistrar(zonedb::kZoneRegistrySize, zonedb::kZoneRegistry);
  const basic::ZoneInfo* zoneInfo = zoneRegistrar.getZoneInfoForId(
      0x0 /* should not exist */);
  assertEqual(zoneInfo, nullptr);
}

//---------------------------------------------------------------------------
// Tests for findIndexFor*() need a stable registry.
//---------------------------------------------------------------------------

test(BasicZoneRegistrarTest, findIndexForName) {
  ZoneRegistrar zoneRegistrar(kNumSortedEntries, kSortedRegistry);
  uint16_t index = zoneRegistrar.findIndexForName("America/Los_Angeles");
  assertEqual((uint16_t) 3, index);
}

test(BasicZoneRegistrarTest, findIndexForName_not_found) {
  ZoneRegistrar zoneRegistrar(kNumSortedEntries, kSortedRegistry);
  uint16_t index = zoneRegistrar.findIndexForName("America/not_found");
  assertEqual(ZoneRegistrar::kInvalidIndex, index);
}

test(BasicZoneRegistrarTest, findIndexForId) {
  ZoneRegistrar zoneRegistrar(kNumSortedEntries, kSortedRegistry);
  uint16_t index = zoneRegistrar.findIndexForId(
      zonedb::kZoneIdAmerica_Los_Angeles);
  assertEqual((uint16_t) 3, index);
}

test(BasicZoneRegistrarTest, findIndexForId_not_found) {
  ZoneRegistrar zoneRegistrar(kNumSortedEntries, kSortedRegistry);
  uint16_t index = zoneRegistrar.findIndexForId(0x0);
  assertEqual(ZoneRegistrar::kInvalidIndex, index);
}

//---------------------------------------------------------------------------
// Test ZoneRegistrar::isSorted().
//---------------------------------------------------------------------------

test(BasicZoneRegistrarTest_Sorted, isSorted) {
  assertTrue(ZoneRegistrar::isSorted(kSortedRegistry, kNumSortedEntries));
}

test(BasicZoneRegistrarTest_Unsorted, isSorted) {
  assertFalse(ZoneRegistrar::isSorted(
      kUnsortedRegistry, kNumUnsortedEntries));
}

//---------------------------------------------------------------------------
// ZoneRegistrar::linearSearchById() w/ sorted registry.
//---------------------------------------------------------------------------

test(BasicZoneRegistrarTest_Sorted, linearSearchById) {
  uint16_t index = ZoneRegistrar::linearSearchById(
      kSortedRegistry, kNumSortedEntries, zonedb::kZoneIdAmerica_Los_Angeles);
  assertEqual((uint16_t) 3, index);
}

test(BasicZoneRegistrarTest_Sorted, linearSearchById_not_found) {
  uint16_t index = ZoneRegistrar::linearSearchById(
      kSortedRegistry, kNumSortedEntries, 0);
  assertEqual(ZoneRegistrar::kInvalidIndex, index);
}

//---------------------------------------------------------------------------
// ZoneRegistrar::binarySearchById() w/ sorted registry.
//---------------------------------------------------------------------------

test(BasicZoneRegistrarTest_Sorted, binarySearchById_zeroEntries) {
  uint16_t index = ZoneRegistrar::binarySearchById(
      kSortedRegistry,
      0 /* kNumSortedEntries */,
      zonedb::kZoneIdAmerica_Los_Angeles);
  assertEqual(ZoneRegistrar::kInvalidIndex, index);
}

test(BasicZoneRegistrarTest_Sorted, binarySearchById) {
  uint16_t index;

  index = ZoneRegistrar::binarySearchById(
      kSortedRegistry, kNumSortedEntries, zonedb::kZoneIdAmerica_New_York);
  assertEqual((uint16_t) 0, index);

  index = ZoneRegistrar::binarySearchById(
      kSortedRegistry, kNumSortedEntries, zonedb::kZoneIdAmerica_Chicago);
  assertEqual((uint16_t) 1, index);

  index = ZoneRegistrar::binarySearchById(
      kSortedRegistry, kNumSortedEntries, zonedb::kZoneIdAmerica_Denver);
  assertEqual((uint16_t) 2, index);

  index = ZoneRegistrar::binarySearchById(
      kSortedRegistry, kNumSortedEntries, zonedb::kZoneIdAmerica_Los_Angeles);
  assertEqual((uint16_t) 3, index);
}

test(BasicZoneRegistrarTest_Sorted, binarySearchById_not_found) {
  uint16_t index = ZoneRegistrar::binarySearchById(
      kSortedRegistry, kNumSortedEntries, 0);
  assertEqual(ZoneRegistrar::kInvalidIndex, index);
}

//---------------------------------------------------------------------------
// ZoneRegistrar::linearSearchById() with *unsorted* registry.
//---------------------------------------------------------------------------

test(BasicZoneRegistrarTest_Unsorted, linearSearchById) {
  uint16_t index = ZoneRegistrar::linearSearchById(
      kUnsortedRegistry, kNumUnsortedEntries,
      zonedb::kZoneIdAmerica_Los_Angeles);
  assertEqual((uint16_t) 2, index);
}

test(BasicZoneRegistrarTest_Unsorted, linearSearchById_not_found) {
  uint16_t index = ZoneRegistrar::linearSearchById(
      kUnsortedRegistry, kNumUnsortedEntries, 0);
  assertEqual(ZoneRegistrar::kInvalidIndex, index);
}

//---------------------------------------------------------------------------

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000); // wait to prevent garbage on SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Leonardo/Micro
}

void loop() {
  TestRunner::run();
}
