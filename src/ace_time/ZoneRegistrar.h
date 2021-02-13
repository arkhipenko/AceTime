/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_ZONE_REGISTRAR_H
#define ACE_TIME_ZONE_REGISTRAR_H

#include <stdint.h>
#include <AceCommon.h> // KString, binarySearchByKey()
#include "common/compat.h" // ACE_TIME_USE_PROGMEM
#include "internal/ZoneInfo.h"
#include "internal/BasicBrokers.h"
#include "internal/ExtendedBrokers.h"

void runIndexForZoneIdBinary();
void runIndexForZoneIdLinear();
class BasicZoneRegistrarTest_Sorted_isSorted;
class BasicZoneRegistrarTest_Unsorted_isSorted;
class BasicZoneRegistrarTest_Sorted_linearSearchById;
class BasicZoneRegistrarTest_Sorted_linearSearchById_not_found;
class BasicZoneRegistrarTest_Sorted_binarySearchById_zeroEntries;
class BasicZoneRegistrarTest_Sorted_binarySearchById;
class BasicZoneRegistrarTest_Sorted_binarySearchById_not_found;
class BasicZoneRegistrarTest_Unsorted_linearSearchById;
class BasicZoneRegistrarTest_Unsorted_linearSearchById_not_found;

class __FlashStringHelper;

namespace ace_time {

/**
 * Class that allows looking up the ZoneInfo (ZI) from its TZDB identifier
 * (e.g. "America/Los_Angeles"), zoneId (hash from its name), or the index in
 * the zone registry.
 *
 * @tparam ZI ZoneInfo type (e.g. basic::ZoneInfo)
 * @tparam ZIB ZoneInfoBroker type (e.g. basic::ZoneInfoBroker)
 * @tparam ZRGB ZoneRegistryBroker type (e.g. basic::ZoneRegistryBroker)
 */
template<typename ZI, typename ZIB, typename ZRGB>
class ZoneRegistrarTemplate {
  public:
    /** Invalid index to indicate error or not found. */
    static const uint16_t kInvalidIndex = 0xffff;

    /** Constructor. */
    ZoneRegistrarTemplate(
        uint16_t zoneRegistrySize,
        const ZI* const* zoneRegistry
    ):
        mZoneRegistrySize(zoneRegistrySize),
        mIsSorted(isSorted(zoneRegistry, zoneRegistrySize)),
        mZoneRegistry(zoneRegistry) {}

    /** Return the number of zones. */
    uint16_t zoneRegistrySize() const { return mZoneRegistrySize; }

    /**
     * Return true if zoneRegistry is sorted, and eligible to use a binary
     * search.
     */
    bool isSorted() const { return mIsSorted; }

    /** Return the ZoneInfo at index i. Return nullptr if i is out of range. */
    const ZI* getZoneInfoForIndex(uint16_t i) const {
      return (i < mZoneRegistrySize)
          ? ZRGB(mZoneRegistry).zoneInfo(i)
          : nullptr;
    }

    /**
     * Return the ZoneInfo corresponding to the given zone name. Return nullptr
     * if not found.
     */
    const ZI* getZoneInfoForName(const char* name) const {
      uint16_t index = findIndexForName(name);
      if (index == kInvalidIndex) return nullptr;
      return ZRGB(mZoneRegistry).zoneInfo(index);
    }

    /** Return the ZoneInfo using the zoneId. Return nullptr if not found. */
    const ZI* getZoneInfoForId(uint32_t zoneId) const {
      uint16_t index = findIndexForId(zoneId);
      if (index == kInvalidIndex) return nullptr;
      return ZRGB(mZoneRegistry).zoneInfo(index);
    }

    /** Find the index for zone name. Return kInvalidIndex if not found. */
    uint16_t findIndexForName(const char* name) const {
      uint32_t zoneId = ace_common::hashDjb2(name);
      uint16_t index = findIndexForId(zoneId);
      if (index == kInvalidIndex) return kInvalidIndex;

      // Verify that the zoneName actually matches, in case of hash collision.
      ZIB zoneInfoBroker(ZRGB(mZoneRegistry).zoneInfo(index));
      ace_common::KString kname(
        zoneInfoBroker.name(),
        zoneInfoBroker.zoneContext()->fragments,
        zoneInfoBroker.zoneContext()->numFragments
      );
      return (kname.compareTo(name) == 0) ? index : kInvalidIndex;
    }

    /** Find the index for zone id. Return kInvalidIndex if not found. */
    uint16_t findIndexForId(uint32_t zoneId) const {
      if (mIsSorted && mZoneRegistrySize >= kBinarySearchThreshold) {
        return binarySearchById(mZoneRegistry, mZoneRegistrySize, zoneId);
      } else {
        return linearSearchById(mZoneRegistry, mZoneRegistrySize, zoneId);
      }
    }

  protected:
    friend void ::runIndexForZoneIdBinary();
    friend void ::runIndexForZoneIdLinear();
    friend class ::BasicZoneRegistrarTest_Sorted_isSorted;
    friend class ::BasicZoneRegistrarTest_Unsorted_isSorted;
    friend class ::BasicZoneRegistrarTest_Sorted_linearSearchById;
    friend class ::BasicZoneRegistrarTest_Sorted_linearSearchById_not_found;
    friend class ::BasicZoneRegistrarTest_Sorted_binarySearchById_zeroEntries;
    friend class ::BasicZoneRegistrarTest_Sorted_binarySearchById;
    friend class ::BasicZoneRegistrarTest_Sorted_binarySearchById_not_found;
    friend class ::BasicZoneRegistrarTest_Unsorted_linearSearchById;
    friend class ::BasicZoneRegistrarTest_Unsorted_linearSearchById_not_found;

    /** Use binarySearchById() if zoneRegistrySize >= threshold. */
    static const uint8_t kBinarySearchThreshold = 8;

    /** Determine if the given zone registry is sorted by id. */
    static bool isSorted(const ZI* const* registry, uint16_t registrySize) {
      if (registrySize == 0) {
        return false;
      }

      const ZRGB zoneRegistry(registry);
      uint32_t prevId = ZIB(zoneRegistry.zoneInfo(0)).zoneId();
      for (uint16_t i = 1; i < registrySize; ++i) {
        uint32_t currentId = ZIB(zoneRegistry.zoneInfo(i)).zoneId();
        if (prevId > currentId) {
          return false;
        }
        prevId = currentId;
      }
      return true;
    }

    /**
     * Find the registry index corresponding to zoneId using linear search.
     * Returns kInvalidIndex if not found.
     */
    static uint16_t linearSearchById(const ZI* const* registry,
        uint16_t registrySize, uint32_t zoneId) {
      const ZRGB zoneRegistry(registry);
      for (uint16_t i = 0; i < registrySize; ++i) {
        const ZI* zoneInfo = zoneRegistry.zoneInfo(i);
        if (zoneId == ZIB(zoneInfo).zoneId()) {
          return i;
        }
      }
      return kInvalidIndex;
    }

    /**
     * Find the registry index corresponding to zoneId using a binary search.
     * Returns kInvalidIndex if not found.
     *
     * The largest registrySize is UINT16_MAX so the largest valid index is
     * UINT16_MAX - 1. This allows us to set kInvalidIndex to UINT16_MAX to
     * indicate "Not Found".
     */
#if 1
    static uint16_t binarySearchById(const ZI* const* registry,
        uint16_t registrySize, uint32_t zoneId) {
      const ZRGB zoneRegistry(registry);
      return (uint16_t) ace_common::binarySearchByKey(
          (size_t) registrySize,
          zoneId,
          [&zoneRegistry](size_t i) -> uint32_t {
            const ZI* zoneInfo = zoneRegistry.zoneInfo(i);
            return ZIB(zoneInfo).zoneId();
          } // lambda expression returns zoneId at index i
      );
    }
#else
    // Moved to ace_common::binarySearchByKey(). The templatized version
    // produces identical code sizes for 8-bit processors. On 32-bit
    // processors, the code size usually goes *down* by 4-60 bytes using the
    // templatized version.
    static uint16_t binarySearchById(const ZI* const* registry,
        uint16_t registrySize, uint32_t zoneId) {
      uint16_t a = 0;
      uint16_t b = registrySize;
      const ZRGB zoneRegistry(registry);
      while (true) {
        uint16_t diff = b - a;
        if (diff == 0) break;

        uint16_t c = a + diff / 2;
        const ZI* zoneInfo = zoneRegistry.zoneInfo(c);
        uint32_t currentId = ZIB(zoneInfo).zoneId();
        if (currentId == zoneId) return c;
        if (zoneId < currentId) {
          b = c;
        } else {
          a = c + 1;
        }
      }
      return kInvalidIndex;
    }
#endif

    /** Exposed only for benchmarking purposes. */
    uint16_t findIndexForIdLinear(uint32_t zoneId) const {
      return linearSearchById(mZoneRegistry, mZoneRegistrySize, zoneId);
    }

    /** Exposed only for benchmarking purposes. */
    uint16_t findIndexForIdBinary(uint32_t zoneId) const {
      return binarySearchById(mZoneRegistry, mZoneRegistrySize, zoneId);
    }

    uint16_t const mZoneRegistrySize;
    bool const mIsSorted;
    const ZI* const* const mZoneRegistry;
};

#if 1

/**
 * Concrete template instantiation of ZoneRegistrarTemplate for
 * basic::ZoneInfo, which can be used with BasicZoneProcessor.
 */
class BasicZoneRegistrar: public ZoneRegistrarTemplate<
    basic::ZoneInfo,
    basic::ZoneInfoBroker,
    basic::ZoneRegistryBroker
> {
  public:
    BasicZoneRegistrar(
        uint16_t zoneRegistrySize,
        const basic::ZoneInfo* const* zoneRegistry
    ) :
        ZoneRegistrarTemplate<
            basic::ZoneInfo,
            basic::ZoneInfoBroker,
            basic::ZoneRegistryBroker
        >(zoneRegistrySize, zoneRegistry)
    {}
};

/**
 * Concrete template instantiation of ZoneRegistrarTemplate for
 * extended::ZoneInfo, which can be used with ExtendedZoneProcessor.
 */
class ExtendedZoneRegistrar: public ZoneRegistrarTemplate<
    extended::ZoneInfo,
    extended::ZoneInfoBroker,
    extended::ZoneRegistryBroker
> {
  public:
    ExtendedZoneRegistrar(
        uint16_t zoneRegistrySize,
        const extended::ZoneInfo* const* zoneRegistry
    ) :
        ZoneRegistrarTemplate<
            extended::ZoneInfo,
            extended::ZoneInfoBroker,
            extended::ZoneRegistryBroker
        >(zoneRegistrySize, zoneRegistry)
    {}
};

#else

// Use subclassing instead of template typedef so that error messages are
// understandable. The compiler seems to optimize away the subclass overhead.

typedef ZoneRegistrarTemplate<
    basic::ZoneInfo,
    basic::ZoneRegistryBroker,
    basic::ZoneInfoBroker
>
    BasicZoneRegistrar;

typedef ZoneRegistrarTemplate<
    extended::ZoneInfo,
    extended::ZoneRegistryBroker,
    extended::ZoneInfoBroker
>
    ExtendedZoneRegistrar;

#endif

} // ace_time

#endif // ACE_TIME_ZONE_REGISTRAR_H
