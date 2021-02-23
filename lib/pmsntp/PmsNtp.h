
#ifndef __ntp_h
#define __ntp_h

#include <Arduino.h>

// #include <TimeLib.h>      //TimeLib library is needed https://github.com/PaulStoffregen/Time
// #include <NtpClientLib.h> //Include NtpClient library header

class PmsNtp
{
public:
  void setupNtp(const String &ntpServerName, int8_t timeZone = 1, bool daylight = true);
  void loopNtp();
  bool isSynchronized() { return sync; }

private:
  bool sync;
};

#endif
