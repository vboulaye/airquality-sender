
#ifndef __wifi_h
#define __wifi_h

#include <Arduino.h>

#define CONNECTION_TIMEOUT 10000

class PmsWifi
{
public:
  void setupWifi(byte configPin);
  void loopWifi();

private:
  byte configPin;
};

static bool doSave;

#endif
