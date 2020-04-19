
#include "AppConfig.h"

#include <ArduinoLog.h>
#include <FS.h>

#define DEBUG_APP_CONFIG

#ifdef DEBUG_APP_CONFIG
#define APP_CONFIG_F(string_literal) F("[config] " string_literal CR)
#define APP_CONFIG_LOG(level, format, ...) Log.level(APP_CONFIG_F(format), ##__VA_ARGS__)
#else
#define APP_CONFIG_LOG(...) //now defines a blank line
#endif

void AppConfiguration::resetConfig()
{
    APP_CONFIG_LOG(notice, "reset configuration to %s", toJson().c_str());
    SPIFFS.remove(APP_CONFIG_FILE_PATH);
}

void AppConfiguration::setupAppConfig()
{

    // start the file system, format it on failure
    if (!SPIFFS.begin())
    {
        if (!SPIFFS.format())
        {
            APP_CONFIG_LOG(error, "Failed to format file system");
            resetConfig();
            return;
        }
        if (!SPIFFS.begin())
        {
            APP_CONFIG_LOG(error, "Failed to mount file system");
            resetConfig();
            return;
        }
    }
    if (!loadConfig())
    {
        APP_CONFIG_LOG(error, "Failed to load configuration");
        resetConfig();
        return;
    }
}

JsonObject &AppConfiguration::reportState(JsonBuffer &jsonBuffer)
{

    JsonObject &info = jsonBuffer.createObject();
    info["sketchSize"] = ESP.getSketchSize();
    info["sketchMD5"] = ESP.getSketchMD5();

    JsonObject &doc = jsonBuffer.createObject();
    doc["info"] = info;

#define TO_JSON(field) doc[#field] = field;

    MAP_ALL_CONFIG_FIELDS(TO_JSON)

    return doc;
}

String AppConfiguration::toJson()
{
    const int capacity = JSON_OBJECT_SIZE(100);
    StaticJsonBuffer<capacity> jb;
    JsonObject &doc = reportState(jb);

    String output;
    doc.prettyPrintTo(output);
    APP_CONFIG_LOG(trace, "toJson(): %s", output.c_str());

    return output;
}

void importJsonIntoValue(const char *&input, const char *&value)
{
    int _length = strlen(input);
    char *_value = new char[_length + 1];
    strncpy(_value, input, _length);
    _value[_length] = 0;
    value = _value;
}

void importJsonIntoValue(const char *&input, bool &value)
{
    value = String(input).equalsIgnoreCase("true");
}

void importJsonIntoValue(const char *&input, unsigned int &value)
{
    value = atoi(input);
}

bool AppConfiguration::parse(const String &json)
{
    DynamicJsonBuffer newBuffer(1024);
    JsonObject &doc = newBuffer.parseObject(json);
    if (!doc.success())
    {
        APP_CONFIG_LOG(error, "unable to parse config json: %s", json.c_str());
        return false;
    }

    return parseJsonObject(doc);
}

bool AppConfiguration::parseJsonObject(const JsonObject &doc)
{

#define FROM_JSON(field)                                                 \
    if (doc.containsKey(#field))                                         \
    {                                                                    \
        const char *buffer = doc[#field].as<char *>();                   \
        APP_CONFIG_LOG(notice, "parsing field %s = %s", #field, buffer); \
        importJsonIntoValue(buffer, field);                              \
    }

    MAP_ALL_CONFIG_FIELDS(FROM_JSON)

    return true;
}
  

bool AppConfiguration::loadConfig()
{
    File configFile = SPIFFS.open(APP_CONFIG_FILE_PATH, "r");
    if (!configFile)
    {
        APP_CONFIG_LOG(error, "Failed to open config file");
        return false;
    }

    size_t size = configFile.size();
    if (size > 100024)
    {
        APP_CONFIG_LOG(error, "Config file size is too large");
        APP_CONFIG_LOG(error, "Config file size is too large");
        APP_CONFIG_LOG(error, "Config file size is too large");
        return false;
    }

    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);

    // We don't use String here because ArduinoJson library requires the input
    // buffer to be mutable. If you don't use ArduinoJson, you may as well
    // use configFile.readString instead.
    configFile.readBytes(buf.get(), size);
    configFile.close();

    // TODO for debug show the file contents

    bool result = parse(String(buf.get()));

    if (result)
    {
        APP_CONFIG_LOG(notice, "loaded the configuration from the file system: %s", toJson().c_str());
    }

    return result;
}

bool AppConfiguration::saveConfig()
{

    File configFile = SPIFFS.open(APP_CONFIG_FILE_PATH, "w");
    if (!configFile)
    {
        APP_CONFIG_LOG(error, "error opening config file for writing");
        return false;
    }

    String jsonConfiguration = toJson();
    configFile.println(jsonConfiguration);
    configFile.close();
    APP_CONFIG_LOG(notice, "saved configuration %s", jsonConfiguration.c_str());

    return true;
}
