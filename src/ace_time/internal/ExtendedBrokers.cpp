/*
 * MIT License
 * Copyright (c) 2021 Brian T. Park
 */

#include <Arduino.h>
#include <AceCommon.h> // KString
#include "BrokerCommon.h" // findShortName()
#include "ExtendedBrokers.h"

using ace_common::KString;
using ace_time::internal::findShortName;
using ace_time::internal::ZoneContext;

namespace ace_time {
namespace extended {

void ZoneInfoBroker::printNameTo(Print& printer) const {
  const ZoneContext* zc = zoneContext();
  KString kname(name(), zc->fragments, zc->numFragments);
  kname.printTo(printer);
}

void ZoneInfoBroker::printShortNameTo(Print& printer) const {
  printer.print(findShortName(name()));
}

} // extended
} // ace_time
