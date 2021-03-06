

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <PMS.h>
#include <ArduinoLog.h>
#include <Time.h>

#include "AppConfig.h"
#include "PmsWifi.h"
#include "PmsNtp.h"

//To use Deep Sleep uncomment below line and (on 8265 only?) connect RST to GPIO16 (D0)
// #define DEEP_SLEEP

//#define WEBCOM
#define COUCHDB

#ifdef WEBCOM
#include "Webcom.h"
#endif

#ifdef COUCHDB
#include "Couchdb.h"
#endif

#define DEBUG_OUT Serial
#define PIN_CONFIG 2

// use a software seria to still see the debug messages
// Rx is 13, Tx is 15 so connect the Tx from PMS to 13 and Rx to 15
// (the second Rx is not mapped on the esp-xm 8285)
SoftwareSerial pmSensorSerial(13, 15);

PmsWifi pmsWifi;
PmsNtp pmsNtp;

#ifdef WEBCOM
Webcom webcom;
#endif

#ifdef COUCHDB
Couchdb couchdb;
#endif

// PMS_READ_INTERVAL (4:30 min) and PMS_READ_FIRST_DELAY (30 sec) CAN'T BE EQUAL!
// Values are also used to detect sensor state.

#define DEV

#ifdef DEV
static const uint32_t PMS_READ_INTERVAL = 10 * 1000;
static const uint32_t PMS_READ_FIRST_DELAY = 3 * 1000;
#define NUMREADS 3
#else
static const uint32_t PMS_READ_INTERVAL = 5 * 60 * 1000;
static const uint32_t PMS_READ_FIRST_DELAY = 30 * 1000;
#define NUMREADS 100
#endif

// Default sensor state.
uint32_t timerInterval = PMS_READ_FIRST_DELAY;

PMS pms(pmSensorSerial);

void readData()
{

  double avg_1_0 = 0, avg_2_5 = 0, avg_10_0 = 0;
  double samples = 0;
  uint16_t previousChecksum = 0;

  DEBUG_OUT.println("Send read request...");
  for (int i = 0; i < NUMREADS; i++)
  {
    PMS::DATA data;
    // Clear buffer (removes potentially old data) before read. Some data could have been also sent before switching to passive mode.
    DEBUG_OUT.println("clear buffer");
    while (pmSensorSerial.available())
    {
      pmSensorSerial.read();
    }

    DEBUG_OUT.println("requestread");
    pms.requestRead();

    if (!pms.readUntil(data, 3000))
    {
      DEBUG_OUT.println("No data");
      continue;
    }
    DEBUG_OUT.print("checksum: ");
    DEBUG_OUT.println(data.checksum);
    if (previousChecksum == data.checksum)
    {
      DEBUG_OUT.println("Duplicate data.");
      continue;
    }

    previousChecksum = data.checksum;

    samples++;
    avg_1_0 += log10(data.PM_AE_UG_1_0);
    avg_2_5 += log10(data.PM_AE_UG_2_5);
    avg_10_0 += log10(data.PM_AE_UG_10_0);

    DEBUG_OUT.print("PM 1.0 (ug/m3): ");
    DEBUG_OUT.print(data.PM_AE_UG_1_0);
    DEBUG_OUT.print(", ");

    // DEBUG_OUT.print("PM 2.5 (ug/m3): ");
    // DEBUG_OUT.println(data.PM_AE_UG_2_5);

    // DEBUG_OUT.print("PM 10.0 (ug/m3): ");
    // DEBUG_OUT.println(data.PM_AE_UG_10_0);

    DEBUG_OUT.print(samples);
    DEBUG_OUT.println(" samples.");
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

    // char dateStr[12];
    // time_t moment = now();
    // sprintf(dateStr, "%4d/%02d/%02d", year(moment), month(moment), day(moment));
    // String path(AppConfiguration::get().webcomLocation);
    // path.concat("/");
    // path.concat(dateStr);

    const int capacity = JSON_OBJECT_SIZE(20);
    StaticJsonBuffer<capacity> jb;
    JsonObject &doc = jb.createObject();
    doc["pm_1_0"] = avg.PM_AE_UG_1_0;
    doc["pm_2_5"] = avg.PM_AE_UG_2_5;
    doc["pm_10_0"] = avg.PM_AE_UG_10_0;

#ifdef WEBCOM

    JsonObject &ts = jb.createObject();
    ts[".sv"] = "timestamp";
    doc["ts"] = ts;

    String webcomPayload;
    doc.prettyPrintTo(webcomPayload);
    DEBUG_OUT.println(webcomPayload);

    char dateStr[12];
    tm info;
    uint32_t ms=10000;
    getLocalTime(&info, ms);
    sprintf(dateStr, "%4d/%02d/%02d", 1900+info.tm_year, 1+info.tm_mon, info.tm_mday);
//    time_t moment = now();
//    sprintf(dateStr, "%4d/%02d/%02d", year(moment), month(moment), day(moment));
    String path(AppConfiguration::get().webcomLocation);
    path.concat("/");
    path.concat(dateStr);
    webcom.post(path, webcomPayload);
#endif

#ifdef COUCHDB


    // time_t moment = now();
    // sprintf(dateStr, "%4d-%02d-%02d_%02d-%02d-%02d",
    //   year(moment), month(moment), day(moment),
    //   hour(moment), minute(moment), second(moment)
    //   );
    //doc["ts"] = "ts";
 
    char dateStr[20];
    tm info;
    uint32_t ms=10000;
    getLocalTime(&info, ms);
    sprintf(dateStr, "%4d-%02d-%02dT%02d:%02d:%02d", 
      1900+info.tm_year, 1+info.tm_mon, info.tm_mday,
      info.tm_hour, 1+info.tm_min, info.tm_sec
      );


    doc["ts"] = dateStr;
 
    String couchdbPayload;
    doc.prettyPrintTo(couchdbPayload);
    DEBUG_OUT.println(couchdbPayload);

    String path(dateStr);
    couchdb.post(path, couchdbPayload);
#endif

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

  Log.begin(LOG_LEVEL_NOTICE, &DEBUG_OUT);

  AppConfiguration::get().setupAppConfig();

  pmsWifi.setupWifi(PIN_CONFIG);
  pmsNtp.setupNtp("europe.pool.ntp.org", 1, true);

#ifdef WEBCOM
  webcom.setupWebcom(
      AppConfiguration::get().webcomDatabase,
      AppConfiguration::get().webcomPassword);
#endif

#ifdef COUCHDB
  couchdb.setupCouchdb(
      AppConfiguration::get().couchdbUrl,
      AppConfiguration::get().couchdbFingerprint,
      AppConfiguration::get().couchdbDatabase,
      AppConfiguration::get().couchdbUser,
      AppConfiguration::get().couchdbPassword
  );
#endif

  DEBUG_OUT.println("starting pms sensor");
  pmSensorSerial.begin(PMS::BAUD_RATE);

#ifndef DEEP_SLEEP
  DEBUG_OUT.println("Switch to passive mode.");
  pms.passiveMode();
#endif

  // Default state after sensor power, but undefined after ESP restart e.g. by OTA flash, so we have to manually wake up the sensor for sure.
  // Some logs from bootloader is sent via Serial port to the sensor after power up. This can cause invalid first read or wake up so be patient and wait for next read cycle.
  DEBUG_OUT.println("wake up");
  pms.wakeUp();

#ifdef DEEP_SLEEP
  DEBUG_OUT.println("waiting for the sensor to be ready");
  delay(PMS_READ_FIRST_DELAY);
  tm info;
  uint32_t ms=10000;
  while (!getLocalTime(&info, ms))
  {
    DEBUG_OUT.println("waiting toget the ttime synced");
    delay(15 * 1000);
    if (millis() > 5 * 60 * 1000)
    {
      DEBUG_OUT.println("giving up, will try another time");
      pms.sleep();
      ESP.deepSleep(PMS_READ_INTERVAL * 1000);
    }
  }
  readData();
  DEBUG_OUT.println("shuting down");
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
  pmsWifi.loopWifi();
  pmsNtp.loopNtp();

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
