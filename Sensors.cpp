#include "Sensors.h"
#include "Constants.h"
#include <DHT.h>

DHT dht(DHTPIN, DHTTYPE);
volatile int pulseCount = 0;
unsigned long lastFlowTime = 0;
unsigned long lastDHTRead = 0;

void pulseCounter() {
    pulseCount++;
}

void initSensors() {
    dht.begin();
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(FLOW_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(FLOW_PIN), pulseCounter, FALLING);
}

void readAllSensors(SensorData &data) {
    unsigned long now = millis();

    // Modularized DHT reading with explicit timing constants
    if (now - lastDHTRead >= DHT_SAMPLE_RATE_MS) {
        float t = dht.readTemperature();
        float h = dht.readHumidity();
        if (!isnan(t)) data.temperature = t;
        if (!isnan(h)) data.humidity = h;
        lastDHTRead = now;
    }

    data.waterLevel = analogRead(WATER_SENSOR_PIN);
    data.soilValue  = analogRead(SOIL_PIN);
    data.ldrValue   = analogRead(LDR_PIN);
    data.distance   = getDistance();
    data.objectNear = (data.distance > 0 && data.distance <= DETECT_DIST_CM);
    data.soundLevel = analogRead(SOUND_AO);

    // Flow sensor calculations using timing windows
    if (now - lastFlowTime >= FLOW_SAMPLE_RATE_MS) {
        detachInterrupt(digitalPinToInterrupt(FLOW_PIN));
        data.flowRate   = pulseCount / 7.5;
        data.totalLiters += data.flowRate / 60.0;
        pulseCount     = 0;
        lastFlowTime   = now;
        attachInterrupt(digitalPinToInterrupt(FLOW_PIN), pulseCounter, FALLING);
    }
}

long getDistance() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    long duration = pulseIn(ECHO_PIN, HIGH, 30000);
    return duration / 58;
}