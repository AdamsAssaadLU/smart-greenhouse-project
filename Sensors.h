#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

// Structure to pass clean data across modules
struct SensorData {
    float temperature;
    float humidity;
    int waterLevel;
    int soilValue;
    int ldrValue;
    long distance;
    bool objectNear;
    int soundLevel;
    float flowRate;
    float totalLiters;
};

// Driver declarations
void initSensors();
void readAllSensors(SensorData &data);
long getDistance();

#endif