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
        MACRO(startConfigOnBoot) 

private:
        AppConfiguration(){}; // nobody can create Singletons directly

        JsonObject &reportState(JsonBuffer &jsonBuffer);
        bool parseJsonObject(const JsonObject &doc);
};

#endif
