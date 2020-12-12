/*
 * Generate the validation_data.json file for the zones given on the STDIN. The
 * transition time and UTC offsets are calculated using the C library <time.h>.
 * I use C++ as the driver program to preserve sanity while doing string,
 * vector and map manipulations, but timezone and UTC offset calculations are
 * done using the <time.h> library.
 *
 * Borrows heavily from compare_cpp/test_data_generator.cpp.
 *
 * Usage:
 * $ ./test_data_generator.out \
 *    [--start_year start] [--until_year until] \
 *    < zones.txt
 *
 * Produces a 'validation_data.json' file in the current directory.
 *
 * 2020-12-12: I cannot get this to work. Each time I run the program,
 * toEpochSeconds() returns -1 for different, random date/time records. But
 * that's supposed to be impossible, since each invocation of the program is a
 * separate process. This is a single threaded program, so I am not able to see
 * why the errors are random. I probably don't understand the old <time.h>
 * library well enough. Giving up for now.
 */

#include <iostream> // getline()
#include <string> // string
#include <map> // map<>
#include <vector> // vector<>
#include <algorithm> // sort
#include <stdio.h> // fprintf()
#include <string.h> // strcmp(), strncmp()
#include <time.h> // time(), localtime_r()
#include <stdlib.h> // setenv()

#define ENABLE_DEBUG 0

using namespace std;

/** Sample every 22 hours when looking for transitions. */
const time_t SAMPLING_INTERVAL = 22 * 3600;

/** Difference between Unix epoch (1970-01-1) and AceTime Epoch (2000-01-01). */
const long SECONDS_SINCE_UNIX_EPOCH = 946684800;

/** Output file name for the JSON data. */
const char VALIDATION_DATA_JSON[] = "validation_data.json";

// Command line arguments
int startYear = 2000;
int untilYear = 2050;

/** DateTime components. */
struct DateTime {
  int year;
  unsigned month;
  unsigned day;
  int hour;
  int minute;
  int second;
  int utcOffset; // total UTC offset including DST shift

  void print() {
    printf("%04d-%02d-%02dT%02d:%02d:%02d",
        year,
        month,
        day,
        hour,
        minute,
        second
    );
  }
};

time_t toEpochSeconds(const DateTime& dt) {
  struct tm tms;
  tms.tm_year = dt.year - 1900;
  tms.tm_mon = dt.month - 1;
  tms.tm_mday = dt.day;
  tms.tm_hour = dt.hour;
  tms.tm_min = dt.minute;
  tms.tm_sec = dt.second;
  return mktime(&tms);
}

DateTime toDateTime(time_t epochSeconds) {
  struct tm tms;
  localtime_r(&epochSeconds, &tms);
  return {
    tms.tm_year + 1900,
    (unsigned) (tms.tm_mon + 1),
    (unsigned) tms.tm_mday,
    tms.tm_hour,
    tms.tm_min,
    tms.tm_sec,
    (int) tms.tm_gmtoff
  };
}

/**
 * A test item, containing the epochSeconds with its expected DateTime
 * components.
 */
struct TestItem {
  long epochSeconds;
  int utcOffset; // seconds
  int dstOffset; // seconds
  string abbrev;
  int year;
  unsigned month;
  unsigned day;
  int hour;
  int minute;
  int second;
  char type; //'A', 'B', 'S', or 'Y'
};

// Map<ZoneName -> TestItem[]>
typedef map<string, vector<TestItem>> TestData;

/** Convert seconds to TestItem. */
TestItem toTestItem(time_t epochSeconds, char type) {
  DateTime dt = toDateTime(epochSeconds);
  return TestItem{
    epochSeconds - SECONDS_SINCE_UNIX_EPOCH,
    dt.utcOffset,
    0 /* dstOffset */,
    "" /* abbrev */,
    dt.year,
    dt.month,
    dt.day,
    dt.hour,
    dt.minute,
    dt.second,
    type,
  };
}

/**
 * A record of a UtcOffset or DstOffse transition. The C-library does not
 * provide access to the DST offsets, so all transitions are UTC offset
 * transitions (only_dst=false).
 */
struct Transition {
  time_t left; // one second (minute?) before the transition
  time_t right; // at the transition
};

void addTestItem(TestData& testData, const string& zoneName,
    const TestItem& item) {
  auto it = testData.find(zoneName);
  if (it == testData.end()) {
    testData[zoneName] = vector<TestItem>();
  }
  // Argh: There's probably a way to reuse the 'it' iterator and avoid
  // a second lookup but I don't have the time and patience to figure out the
  // C++ map<> API, and this is good enough for this program.
  auto &v = testData[zoneName];
  v.push_back(item);
}

/**
 * Search for the adjacent leftSeconds and rightSeconds of the detected UTC
 * offset transition given by 'left' and 'right', and return the results in
 * leftSeconds and rightSeconds.
 */
void binarySearchTransition(time_t& leftSeconds, time_t& rightSeconds,
    time_t left, time_t right) {
  while (true) {
    time_t deltaMinutes = (right - left) / 60;
    deltaMinutes /= 2;
    if (deltaMinutes == 0) break;

    time_t mid = left + deltaMinutes * 60;
    DateTime leftDateTime = toDateTime(left);
    DateTime midDateTime = toDateTime(mid);
    if (leftDateTime.utcOffset != midDateTime.utcOffset) {
      right = mid;
    } else {
      left = mid;
    }
  }
  leftSeconds = left;
  rightSeconds = right;
}

/** Find the UTC offset transitions . */
void addTransitions(vector<TestItem>& testItems, const string& zoneName,
    int startYear, int untilYear) {
  time_t beginSeconds = toEpochSeconds({startYear, 1, 1, 4, 0, 0, 0 /*utc*/});
  if (beginSeconds == -1) {
    fprintf(stderr, "%s: Error calculating beginSeconds\n", zoneName.c_str());
  }
  time_t untilSeconds = toEpochSeconds({untilYear, 1, 1, 4, 0, 0, 0 /*utc*/});
  if (untilSeconds == -1) {
    fprintf(stderr, "%s: Error calculating untilSeconds\n", zoneName.c_str());
  }

  time_t prevSeconds = beginSeconds;
  DateTime prevDt = toDateTime(prevSeconds);
  for (time_t currentSeconds = beginSeconds;
      currentSeconds < untilSeconds;
      currentSeconds += SAMPLING_INTERVAL
  ) {
    DateTime currentDt = toDateTime(currentSeconds);
    if (prevDt.utcOffset != currentDt.utcOffset) {
      time_t leftSeconds;
      time_t rightSeconds;
      binarySearchTransition(
          leftSeconds, rightSeconds,
          prevSeconds, currentSeconds);

      if (ENABLE_DEBUG) {
        DateTime leftDt = toDateTime(leftSeconds);
        DateTime rightDt = toDateTime(rightSeconds);
        printf("Left: ");
        leftDt.print();
        printf("; Right: ");
        rightDt.print();
        printf("\n");
      }

      testItems.push_back(toTestItem(leftSeconds, 'A'));
      testItems.push_back(toTestItem(rightSeconds, 'B'));
    }
    prevSeconds = currentSeconds;
    prevDt = currentDt;
  }
}

/**
 * Add a TestItem for the 1st of each month (using the local time)
 * as a sanity sample, to make sure things are working, even for timezones with
 * no DST transitions.
 */
void addMonthlySamples(vector<TestItem>& testItems, const string& zoneName,
    int startYear, int untilYear) {
  for (int y = startYear; y < untilYear; y++) {
    // Add the 1st of every month...
    for (unsigned m = 1; m <= 12; m++) {
      time_t seconds = toEpochSeconds({y, m, 2 /* 2nd */, 0, 0, 0, 0 /*utc*/});
      if (seconds == -1) {
        fprintf(stderr, "%s: Error calculating month seconds\n",
            zoneName.c_str());
      }
      testItems.push_back(toTestItem(seconds, 'S'));
    }
  }
}

/**
 * Set the timezone as currently specified by the "TZ" environment variable.
 * Returns true if the time zone is valid, false otherwise.
 */
bool setTimeZone(const string& zoneName) {
  setenv("TZ", zoneName.c_str(), 1 /*overwrite*/);

  // Update the following **global** variables:
  // * timezone: the UTC offset in seconds with the opposite sign
  // * daylight: is 1 if the time zone has ever had daylight savings
  // * tzname[0]: the abbreviation in standard time
  // * tzname[1]: the abbreviation in DST time
  // (What a mess.)
  tzset();

  // tzset() does not set an error status, so we don't know if the ZONE_NAME is
  // valid or not. So we use the following heuristics: If the zone does not
  // exist, then tzset() will set the zone to UTC, so daylight offset will be
  // 0. But there are legitimate timezones which track UTC. But when the zone
  // is invalid, it seems like tzname[0] is set to a truncated version of the
  // original zone name, and tzname[1] is set to an empty string.
  bool invalid = timezone == 0
      && daylight == 0
      && strncmp(tzname[0], zoneName.c_str(), strlen(tzname[0])) == 0
      && tzname[1][0] == '\0';

  return !invalid;
}

/** Insert TestItems for the given 'zoneName' into testData. */
void processZone(TestData& testData, const string& zoneName,
    int startYear, int untilYear) {
  bool valid = setTimeZone(zoneName);
  if (! valid) {
    fprintf(stderr, "zone: %s; %s\n",
        zoneName.c_str(),
        (valid) ? "valid" : "NOT valid"
    );
    return;
  }

  // Create a vector for the current zone to hold the TestItems.
  testData[zoneName] = vector<TestItem>();
  vector<TestItem>& testItems = testData[zoneName];

  // Add UTC offset transitions.
  addTransitions(testItems, zoneName, startYear, untilYear);
  //if (ENABLE_DEBUG) {
    fprintf(stderr, "%s: found %d transitions\n",
        zoneName.c_str(), (int) testItems.size());
  //}

  // Add monthly samples
  addMonthlySamples(testItems, zoneName, startYear, untilYear);
}

/**
 * Trim from start (in place). See https://stackoverflow.com/questions/216823
 */
inline void ltrim(string &s) {
	s.erase(s.begin(), find_if(s.begin(), s.end(), [](int ch) {
			return !isspace(ch);
	}));
}

/** Process each zoneName in zones and insert into testData map. */
map<string, vector<TestItem>> processZones(const vector<string>& zones) {
  TestData testData;
  for (string zoneName : zones) {
    processZone(testData, zoneName, startYear, untilYear);
  }
  return testData;
}

/** Read the 'zones.txt' from the stdin, and process each zone. */
vector<string> readZones() {
  vector<string> zones;
  string line;
  while (getline(cin, line)) {
		ltrim(line);
    if (line.empty()) continue;
    if (line[0] == '#') continue;
    zones.push_back(line);
  }

  return zones;
}

/** Sort the TestItems according to epochSeconds. */
void sortTestData(TestData& testData) {
  for (auto& p : testData) {
    sort(p.second.begin(), p.second.end(),
      [](const TestItem& a, const TestItem& b) {
        return a.epochSeconds < b.epochSeconds;
      }
    );
  }
}

/**
 * Generate the validation_data.json file. Adopted from TestDataGenerator.java.
 */
void printJson(const TestData& testData) {
  FILE* fp = fopen(VALIDATION_DATA_JSON, "w");

  string indentUnit = "  ";
  fprintf(fp, "{\n");
  string indent0 = indentUnit;
  fprintf(fp, "%s\"start_year\": %d,\n", indent0.c_str(), startYear);
  fprintf(fp, "%s\"until_year\": %d,\n", indent0.c_str(), untilYear);
  fprintf(fp, "%s\"source\": \"C Library\",\n", indent0.c_str());
  fprintf(fp, "%s\"version\": \"%s\",\n",
      indent0.c_str(), "Unknown");
  fprintf(fp, "%s\"has_valid_abbrev\": false,\n", indent0.c_str());
  fprintf(fp, "%s\"has_valid_dst\": false,\n", indent0.c_str());
  fprintf(fp, "%s\"test_data\": {\n", indent0.c_str());

  // Print each zone
  int zoneCount = 1;
  int numZones = testData.size();
  for (const auto& zoneEntry : testData) {
    string indent1 = indent0 + indentUnit;
    string zoneName = zoneEntry.first;
    fprintf(fp, "%s\"%s\": [\n", indent1.c_str(), zoneName.c_str());

    // Print each testItem
    int itemCount = 1;
    const vector<TestItem>& items = zoneEntry.second;
    for (const TestItem& item : items) {
      string indent2 = indent1 + indentUnit;
      fprintf(fp, "%s{\n", indent2.c_str());
      {
        string indent3 = indent2 + indentUnit;
        fprintf(fp, "%s\"epoch\": %ld,\n", indent3.c_str(), item.epochSeconds);
        fprintf(fp, "%s\"total_offset\": %d,\n",
            indent3.c_str(), item.utcOffset);
        fprintf(fp, "%s\"dst_offset\": %d,\n", indent3.c_str(), item.dstOffset);
        fprintf(fp, "%s\"y\": %d,\n", indent3.c_str(), item.year);
        fprintf(fp, "%s\"M\": %d,\n", indent3.c_str(), item.month);
        fprintf(fp, "%s\"d\": %d,\n", indent3.c_str(), item.day);
        fprintf(fp, "%s\"h\": %d,\n", indent3.c_str(), item.hour);
        fprintf(fp, "%s\"m\": %d,\n", indent3.c_str(), item.minute);
        fprintf(fp, "%s\"s\": %d,\n", indent3.c_str(), item.second);
        fprintf(fp, "%s\"abbrev\": \"%s\",\n",
            indent3.c_str(), item.abbrev.c_str());
        fprintf(fp, "%s\"type\": \"%c\"\n", indent3.c_str(), item.type);
      }
      fprintf(fp, "%s}%s\n", indent2.c_str(),
          (itemCount < (int)items.size()) ? "," : "");
      itemCount++;
    }

    fprintf(fp, "%s]%s\n", indent1.c_str(), (zoneCount < numZones) ? "," : "");
    zoneCount++;
  }

  fprintf(fp, "%s}\n", indent0.c_str());
  fprintf(fp, "}\n");

  fclose(fp);

  fprintf(stderr, "Created %s\n", VALIDATION_DATA_JSON);
}

void usageAndExit() {
  fprintf(stderr,
    "Usage: test_data_generator --tz_version {version}\n"
    "   [--start_year start] [--until_year until]\n"
    "   < zones.txt\n");
  exit(1);
}

#define SHIFT(argc, argv) do { argc--; argv++; } while(0)
#define ARG_EQUALS(s, t) (strcmp(s, t) == 0)

int main(int argc, const char* const* argv) {
  // Parse command line flags.
  string start = "2000";
  string until = "2050";
  string tzVersion = "";

  SHIFT(argc, argv);
  while (argc > 0) {
    if (ARG_EQUALS(argv[0], "--start_year")) {
      SHIFT(argc, argv);
      if (argc == 0) usageAndExit();
      start = argv[0];
    } else if (ARG_EQUALS(argv[0], "--until_year")) {
      SHIFT(argc, argv);
      if (argc == 0) usageAndExit();
      until = argv[0];
    } else if (ARG_EQUALS(argv[0], "--tz_version")) {
      SHIFT(argc, argv);
      if (argc == 0) usageAndExit();
      tzVersion = argv[0];
    } else if (ARG_EQUALS(argv[0], "--")) {
      SHIFT(argc, argv);
      break;
    } else if (strncmp(argv[0], "-", 1) == 0) {
      fprintf(stderr, "Unknonwn flag '%s'\n", argv[0]);
      usageAndExit();
    } else {
      break;
    }
    SHIFT(argc, argv);
  }

  if (tzVersion.empty()) {
    fprintf(stderr, "Must give --tz_version flag for Hinnant Date'\n");
    usageAndExit();
  }

  startYear = atoi(start.c_str());
  untilYear = atoi(until.c_str());

  // Process the zones on the STDIN
  vector<string> zones = readZones();
  TestData testData = processZones(zones);
  sortTestData(testData);
  printJson(testData);
  return 0;
}
