#ifndef __pm_sensor__
#define __pm_sensor__

#include <Arduino.h>

// livrary attempt from th eexamplesat adafruit https://learn.adafruit.com/pm25-air-quality-sensor/

struct pms5003data {
  uint16_t framelen;
  uint16_t pm10_standard, pm25_standard, pm100_standard;
  uint16_t pm10_env, pm25_env, pm100_env;
  uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
  uint16_t unused;
  uint16_t checksum;
};


class PmSensor {

    public:
        void setupPmSensor(Stream &stream);
        bool loopPmSensor();
        void printDebug();

        pms5003data& getData(){return data;};
    
    private:
        Stream *stream;
        struct pms5003data data;

};

#endif