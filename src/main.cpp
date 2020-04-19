#include <Arduino.h>
#include <SoftwareSerial.h>
#include <PMS.h>

#define DEBUG_OUT Serial

// use a software seria to still see the debug messages
// Rx is 13, Tx is 15 so connect the Tx from PMS to 13 and Rx to 15
// (the second Rx is not mapped on the esp-xm 8285)
SoftwareSerial pmSensorSerial(13, 15);

// To use Deep Sleep connect RST to GPIO16 (D0) and uncomment below line.
// #define DEEP_SLEEP

// PMS_READ_INTERVAL (4:30 min) and PMS_READ_FIRST_DELAY (30 sec) CAN'T BE EQUAL! Values are also used to detect sensor state.
static const uint32_t PMS_READ_INTERVAL = 10000;
static const uint32_t PMS_READ_FIRST_DELAY = 30000;

// Default sensor state.
uint32_t timerInterval = PMS_READ_FIRST_DELAY;

PMS pms(pmSensorSerial);

#define NUMREADS 10

void readData()
{

  // Clear buffer (removes potentially old data) before read. Some data could have been also sent before switching to passive mode.
  while (pmSensorSerial.available())
  {
    pmSensorSerial.read();
  }

  double avg_1_0 = 0, avg_2_5 = 0, avg_10_0 = 0;
  double samples = 0;
  uint16_t previousChecksum = 0;

  DEBUG_OUT.println("Send read request...");
  for (int i = 0; i < NUMREADS; i++)
  {
    PMS::DATA data;

    pms.requestRead();

    if (pms.readUntil(data, 2000) 
      && previousChecksum != data.checksum)
    {
      previousChecksum = data.checksum;

      DEBUG_OUT.println();
      samples++;
      avg_1_0 += log10(data.PM_AE_UG_1_0);
      avg_2_5 += log10(data.PM_AE_UG_2_5);
      avg_10_0 += log10(data.PM_AE_UG_10_0);

      DEBUG_OUT.print("PM 1.0 (ug/m3): ");
      DEBUG_OUT.println(data.PM_AE_UG_1_0);

      DEBUG_OUT.print("PM 2.5 (ug/m3): ");
      DEBUG_OUT.println(data.PM_AE_UG_2_5);

      DEBUG_OUT.print("PM 10.0 (ug/m3): ");
      DEBUG_OUT.println(data.PM_AE_UG_10_0);

      DEBUG_OUT.println();
    }
    else
    {
      DEBUG_OUT.println("No data or duplicate data.");
    }
  }

  if (samples > 0)
  {

    DEBUG_OUT.println("Reading data...");
#define GEO_MEAN(a) round(pow10(a / samples))
    PMS::DATA avg;
    avg.PM_AE_UG_1_0 = GEO_MEAN(avg_1_0);
    avg.PM_AE_UG_2_5 = GEO_MEAN(avg_2_5);
    avg.PM_AE_UG_10_0 = GEO_MEAN(avg_10_0);

    DEBUG_OUT.print("avg PM 1.0 (ug/m3): ");
    DEBUG_OUT.println(avg.PM_AE_UG_1_0);

    DEBUG_OUT.print("avg PM 2.5 (ug/m3): ");
    DEBUG_OUT.println(avg.PM_AE_UG_2_5);

    DEBUG_OUT.print("avg PM 10.0 (ug/m3): ");
    DEBUG_OUT.println(avg.PM_AE_UG_10_0);
  }
  else
  {
    DEBUG_OUT.println("Really No data.");
  }
}

void setup()
{
  DEBUG_OUT.begin(9600);
  while (!DEBUG_OUT)
  {
    ; // wait for serial port to connect. Needed for native USB
  }

  DEBUG_OUT.println("starting pms sensor");
  pmSensorSerial.begin(PMS::BAUD_RATE);

  DEBUG_OUT.println("Switch to passive mode.");
  pms.passiveMode();

  // Default state after sensor power, but undefined after ESP restart e.g. by OTA flash, so we have to manually wake up the sensor for sure.
  // Some logs from bootloader is sent via Serial port to the sensor after power up. This can cause invalid first read or wake up so be patient and wait for next read cycle.
  DEBUG_OUT.println("wake up");
  pms.wakeUp();

#ifdef DEEP_SLEEP
  delay(PMS_READ_DELAY);
  readData();
  pms.sleep();
  ESP.deepSleep(PMS_READ_INTERVAL * 1000);
#endif // DEEP_SLEEP
}

#ifndef DEEP_SLEEP
void timerCallback()
{
  if (timerInterval == PMS_READ_FIRST_DELAY)
  {
    DEBUG_OUT.println("first read after wakup.");
    readData();
    DEBUG_OUT.println("Going to sleep.");
    pms.sleep();
  }
  else
  {
    DEBUG_OUT.println("Waking up, wait at least 30 seconds before measurement.");
    pms.wakeUp();
  }
}
#endif // DEEP_SLEEP

void loop()
{
#ifndef DEEP_SLEEP
  static unsigned long timerLast = 0;

  unsigned long timerNow = millis();
  if (timerNow - timerLast >= timerInterval)
  {
    timerLast = timerNow;
    timerCallback();
    timerInterval = timerInterval == PMS_READ_FIRST_DELAY ? PMS_READ_INTERVAL : PMS_READ_FIRST_DELAY;
  }

  // Do other stuff...
#endif // DEEP_SLEEP
}

#ifdef TEST_ADAFRUIT
#include "PmSensor.h"

PmSensor pmSensor;

uint16_t counter = 0;

void setup()
{
  // our debugging output
  Serial.begin(9600);
  pmSensorSerial.begin(9600);
  pmSensor.setupPmSensor(pmSensorSerial);

  // sensor baud rate is 9600
  //pmsSerial.begin(9600);
}

void loop()
{

  if (pmSensor.loopPmSensor())
  {
    pmSensor.printDebug();
    delay(100);
  }
}
#endif