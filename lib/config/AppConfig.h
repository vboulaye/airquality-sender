#ifndef __config_h
#define __config_h

#include <Arduino.h>
#include <ArduinoJson.h>

#define APP_CONFIG_FILE_PATH "/config.json"

class AppConfiguration
{
public:

        const char *webcomDatabase = "airquality";
        const char *webcomPassword = "XXXXX";
        const char *webcomLocation = "juvisy";

        //const char *couchdbUrl = "https://air-quality.ngrok.io";
        //const char *couchdbFingerprint = "9B125401611B8AEF73417FBD43746B306CD7661D";
        const char *couchdbUrl = "XXX";
        //echo "${$(openssl s_client -connect air-quality.ngrok.io:443 2>/dev/null </dev/null | sed -ne '/-BEGIN CERTIFICATE-/,/-END CERTIFICATE-/p' | openssl x509 -noout -fingerprint -sha1)//:/ }"  | sed -e 's/.*=//' -e 's/ //g'
        const char *couchdbFingerprint = "5BC71E380C92E9F135B017B2FD958FD35DC8D8EC";

        const char *couchdbDatabase = "air-quality-villiers";
        const char *couchdbUser = "admin";
        const char *couchdbPassword = "admin";

        bool startConfigOnBoot = false;

        static AppConfiguration &get()
        {
                static AppConfiguration singleton;
                return singleton;
        }

        // load/init the config on startup
        void setupAppConfig();

        // serialize the configuration to json
        String toJson();

        // save the configuration to the file system
        bool saveConfig();
        // load the configuration from the file system
        bool loadConfig();

        // reset the configuration to the default one
        void resetConfig();

        bool parse(const String &json);

        // this macro can be used to apply another macro to all the configuration fields
#define MAP_ALL_CONFIG_FIELDS(MACRO) \
        MACRO(webcomDatabase) \
        MACRO(webcomPassword) \
        MACRO(webcomLocation) \
        MACRO(couchdbUrl) \
        MACRO(couchdbFingerprint) \
        MACRO(couchdbDatabase) \
        MACRO(couchdbUser) \
        MACRO(couchdbPassword) \
        MACRO(startConfigOnBoot)

private:
        AppConfiguration(){}; // nobody can create Singletons directly

        JsonObject &reportState(JsonBuffer &jsonBuffer);
        bool parseJsonObject(const JsonObject &doc);
};

#endif
