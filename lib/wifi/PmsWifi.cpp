#include "PmsWifi.h"

#include <ArduinoLog.h>
#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WebServer.h> //Local WebServer used to serve the configuration portal
#endif
#include <WiFiManager.h>      //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

#include "AppConfig.h"

#include <functional>

#define DEBUG_PMS_WIFI

#ifdef DEBUG_PMS_WIFI
#define PMS_WIFI_F(string_literal) F("[wifi] " string_literal CR)
#define PMS_WIFI_LOG(level, format, ...) Log.level(PMS_WIFI_F(format), ##__VA_ARGS__)
#else
#define PMS_WIFI_LOG(...) //now defines a blank line
#endif

bool waitForConnection()
{
  long startWait = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    if ((millis() - startWait > CONNECTION_TIMEOUT))
    {
      PMS_WIFI_LOG(error, "unable to connect to STA %s", WiFi.SSID().c_str());
      return false;
    }
    PMS_WIFI_LOG(warning, "waiting for connection to STA %s", WiFi.SSID().c_str());
    delay(500);
  }
  PMS_WIFI_LOG(notice, "connected to STA %s", WiFi.SSID().c_str());
  return true;
}

void exportValueIntoParam(const char *value, char *buffer)
{
  strcpy(buffer, value);
}

void exportValueIntoParam(bool value, char *buffer)
{
  strcpy(buffer, value ? "true" : "false");
}

void exportValueIntoParam(unsigned int value, char *buffer)
{
  itoa(value, buffer, 10);
}

void importParamIntoValue(WiFiManagerParameter &parameter, const char *&value)
{
  // reset previous value
  if (value != NULL)
  {
    // delete[] value; cannot delete values when it is the one from the code
    value = NULL;
  }
  if (parameter.getValue() == NULL)
  {
    return;
  }

  int _length = strlen(parameter.getValue());
  char *_value = new char[_length + 1];
  strncpy(_value, parameter.getValue(), _length);
  _value[_length] = 0;
  value = _value;
  PMS_WIFI_LOG(notice, "importParamIntoValue(): %s = %s", parameter.getID(), value);
}

void importParamIntoValue(WiFiManagerParameter &parameter, bool &value)
{
  value = String(parameter.getValue()).equalsIgnoreCase("true");
  PMS_WIFI_LOG(notice, "importParamIntoValue(): %s = %T", parameter.getID(), value);
}

void importParamIntoValue(WiFiManagerParameter &parameter, unsigned int &value)
{
  value = atoi(parameter.getValue());
  PMS_WIFI_LOG(notice, "importParamIntoValue(): %s = %d", parameter.getID(), value);
}

void saveConfigCallback()
{
  doSave = true;
}

void PmsWifi::setupWifi(byte configPin)
{
  PMS_WIFI_LOG(notice, "PmsWifi::setup()");

  //pinMode(configPin, INPUT);
  this->configPin = configPin;

  WiFiManager wifiManager;

// build WiFiManagerParameter for each of the RabbitConfig fields
#define WM_PARAM_EXPORT(field)                                                   \
  char field##Value[100];                                                        \
  exportValueIntoParam(AppConfiguration::get().field, field##Value);             \
  PMS_WIFI_LOG(notice, "exportValueIntoParam(): %s = %s", #field, field##Value); \
  WiFiManagerParameter field##Param(#field, #field, field##Value, 100);          \
  wifiManager.addParameter(&field##Param);

  MAP_ALL_CONFIG_FIELDS(WM_PARAM_EXPORT)

  // always save config even if wifi not modified
  wifiManager.setBreakAfterConfig(true);

  wifiManager.setSaveConfigCallback(saveConfigCallback);

  // wifiManager.setSaveConfigCallback([&doSave]() {
  //   doSave = true;
  // });

  wifiManager.setDebugOutput(true);

  if (digitalRead(configPin) == LOW || AppConfiguration::get().startConfigOnBoot)
  {
    AppConfiguration::get().startConfigOnBoot = false;
    AppConfiguration::get().saveConfig();
    PMS_WIFI_LOG(notice, "start config portal");
    wifiManager.startConfigPortal();
  }

  wifiManager.autoConnect();

  if (doSave)
  {
#define WM_PARAM_IMPORT(field) \
  importParamIntoValue(field##Param, AppConfiguration::get().field);

    MAP_ALL_CONFIG_FIELDS(WM_PARAM_IMPORT)

    AppConfiguration::get().saveConfig();
    PMS_WIFI_LOG(notice, "new config %s", AppConfiguration::get().toJson().c_str());
  }

  // mqtt_server = custom_mqtt_server.getValue();

  // WiFi.onEvent(handleWifiEvent);

  // if (!WiFi.enableSTA(true))
  // {
  //   PMS_WIFI_LOG(error, "unable to switch STA to %T", staEnabled);
  // }

  // WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);

  // char hostname[80];
  // spri  ntf(hostname, "airquality-sensor-%x", ESP.getChipId());
  // WiFi.hostname(hostname);

  // if (!WiFi.begin())
  // {
  //   PMS_WIFI_LOG(error, "unable to connect to STA");
  // }

  waitForConnection();
}

void PmsWifi::loopWifi()
{

  if (digitalRead(configPin) == LOW || AppConfiguration::get().startConfigOnBoot)
  {
    AppConfiguration::get().startConfigOnBoot = true;
    AppConfiguration::get().saveConfig();
    PMS_WIFI_LOG(notice, "start config portal on reboot");
    delay(1000);
    ESP.restart();
  }
}

/** IP to String? */
String toStringIp(IPAddress ip)
{
  String res = "";
  for (int i = 0; i < 3; i++)
  {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}
