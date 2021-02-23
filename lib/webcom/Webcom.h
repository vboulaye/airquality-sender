
#ifndef __webcom_h
#define __webcom_h

#include <Arduino.h>

#ifdef ESP32
#include <HTTPClient.h>
#else
#include <ESP8266HTTPClient.h>
#endif

#define WEBCOM_HOST "https://io.datasync.orange.com/datasync/v2"
//echo "${$(openssl s_client -connect io.datasync.orange.com:443 2>/dev/null </dev/null | sed -ne '/-BEGIN CERTIFICATE-/,/-END CERTIFICATE-/p' | openssl x509 -noout -fingerprint -sha1)//:/ }"  | sed -e 's/.*=//' -e 's/ //g' 
#define WEBCOM_FINGERPRINT "FF21B6F5B2953CDA673B607D63D9812B438414AF"

class Webcom
{
public:
    void setupWebcom(const char *databaseName, const char *pasword);
    void loopWebcom();

    void post(String &path, String &data);

private:
    HTTPClient client;
    String *databaseName;
    String *password;
};

#endif
