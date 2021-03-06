#include "Couchdb.h"

#include <ArduinoLog.h>

#define DEBUG_COuCHDB

#ifdef DEBUG_COuCHDB
#define COUCHDB_F(string_literal) F("[couchdb] " string_literal CR)
#define COUCHDB_LOG(level, format, ...) Log.level(COUCHDB_F(format), ##__VA_ARGS__)
#else
#define COUCHDB_LOG(...) //now defines a blank line
#endif

void Couchdb::setupCouchdb(const char *databaseUrl, const char *databaseFingerprint, 
  const char *databaseName,
  const char *user, const char *password)
{
  COUCHDB_LOG(notice, "Webcom::setup()");

  // You can specify the time server pool and the offset (in seconds, can be
  // changed later with setTimeOffset() ). Additionaly you can specify the
  // update interval (in milliseconds, can be changed using setUpdateInterval() ).
  this->databaseUrl = new String(databaseUrl);
  this->databaseFingerprint = new String(databaseFingerprint);
  this->databaseName = new String(databaseName);
  this->user = new String(user);
  this->password = new String(password); 
}

void Couchdb::loopCouchdb()
{
}

void Couchdb::post(String &path, String &data)
{
  char buffer[200];
  sprintf(buffer, "%s/%s/%s",
          databaseUrl->c_str(),
          databaseName->c_str(),
          path.c_str());

  COUCHDB_LOG(notice, "calling: %s", buffer);

  String bufferString(buffer);
  if (databaseFingerprint->length()>0) {
    COUCHDB_LOG(notice, "with fingerprint: %s", databaseFingerprint->c_str());
    client.begin(bufferString, databaseFingerprint->c_str());
  } else {
    client.begin(bufferString);
  }
  client.addHeader("Content-Type", "application/json");

  COUCHDB_LOG(notice, "user %s, password: %s", user->c_str(), password->c_str());
  client.setAuthorization(user->c_str(), password->c_str());
  COUCHDB_LOG(notice, "payload: %s", data.c_str());
  int httpCode = client.PUT(data);
  COUCHDB_LOG(notice, "httpCode: %d", httpCode);
  String payload = client.getString();
  COUCHDB_LOG(notice, "payload: %s", payload.c_str());
  client.end();
}