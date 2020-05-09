#include "PmsNtp.h"

#include <ArduinoLog.h>

#define DEBUG_NTP

#ifdef DEBUG_NTP
#define NTP_F(string_literal) F("[ntp] " string_literal CR)
#define NTP_LOG(level, format, ...) Log.level(NTP_F(format), ##__VA_ARGS__)
#else
#define NTP_LOG(...) //now defines a blank line
#endif

void PmsNtp::setupNtp(const String &ntpServerName, int8_t timeZone, bool daylight)
{
  NTP_LOG(notice, "PmsNtp::setup()");

  // You can specify the time server pool and the offset (in seconds, can be
  // changed later with setTimeOffset() ). Additionaly you can specify the
  // update interval (in milliseconds, can be changed using setUpdateInterval() ).
  NTP.begin(ntpServerName, timeZone,daylight);

  NTP.onNTPSyncEvent ([this](NTPSyncEvent_t event) {
      sync = true;
  });
}

void PmsNtp::loopNtp()
{
}
