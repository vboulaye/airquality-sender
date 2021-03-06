
#ifndef __couchdb_h
#define __couchdb_h

#include <Arduino.h>

#ifdef ESP32
#include <HTTPClient.h>
#else
#include <ESP8266HTTPClient.h>
#endif


class Couchdb
{
public:
    void setupCouchdb(const char *databaseUrl,const char *databaseFingerprint, 
        const char *databaseName,
        const char *user, const char *pasword);
    void loopCouchdb();

    void post(String &path, String &data);

private:
    HTTPClient client;
    String *databaseUrl;
    String *databaseName;
    String *databaseFingerprint;
    String *user;
    String *password;
};

#endif
