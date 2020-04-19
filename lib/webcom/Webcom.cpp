#include "Webcom.h"

#include <ArduinoLog.h>

#define DEBUG_WEBCOM

#ifdef DEBUG_WEBCOM
#define WEBCOM_F(string_literal) F("[webcom] " string_literal CR)
#define WEBCOM_LOG(level, format, ...) Log.level(WEBCOM_F(format), ##__VA_ARGS__)
#else
#define WEBCOM_LOG(...) //now defines a blank line
#endif

void Webcom::setupWebcom(const char *databaseName, const char *password)
{
  WEBCOM_LOG(notice, "Webcom::setup()");

  // You can specify the time server pool and the offset (in seconds, can be
  // changed later with setTimeOffset() ). Additionaly you can specify the
  // update interval (in milliseconds, can be changed using setUpdateInterval() ).
  this->databaseName = new String(databaseName);
  this->password = new String(password);
}

void Webcom::loopWebcom()
{
}

void Webcom::post(String &path, String &data)
{
  char buffer[200];
  sprintf(buffer, "%s/%s/data/%s",
          WEBCOM_HOST,
          databaseName->c_str(),
          path.c_str());

  WEBCOM_LOG(notice, "calling: %s", buffer);

  String bufferString(buffer);
  client.begin(bufferString, WEBCOM_FINGERPRINT);
  client.addHeader("Content-Type", "application/json");

  WEBCOM_LOG(notice, "password: %s", password->c_str());
  String authorization = "Bearer ";
  authorization.concat(*password);
  client.addHeader("Authorization", authorization);
  int httpCode = client.POST(data);
  WEBCOM_LOG(notice, "httpCode: %d", httpCode);
  String payload = client.getString();
  WEBCOM_LOG(notice, "payload: %s", payload.c_str());
  client.end();
}