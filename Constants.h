#ifndef CONSTANTS_H
#define CONSTANTS_H

// ── PIN DEFINITIONS ────────────────────────────────────────
const int FAN_PIN        = 3;
#define DHTPIN            32
#define DHTTYPE           DHT11

#define SS_PIN            9
#define RST_PIN           6

const int LED_WATER1      = 48;
const int LED_WATER2      = 44;
const int LED_WATER3      = 46;
const int LED_RFID        = 36;
const int LED_RFID_RED    = 40;
const int LED_LDR1        = 38;
const int LED_LDR2        = 42;
const int WATER_SENSOR_PIN = A0;
const int SOIL_PIN         = A1;
const int LDR_PIN          = A2;
const int SOIL_RELAY_PIN   = 34;
const int TRIG_PIN         = 28;
const int ECHO_PIN         = 29;
const int BUZZER_PIN       = 53;
const int BUTTON_PIN       = 35;
const int SOUND_AO         = A15;
#define FLOW_PIN          2

// ── CALIBRATION THRESHOLDS & CONSTANTS ──────────────────────
const int WATER_LOW       = 300;
const int WATER_MED       = 600;
const int WATER_HIGH      = 700;
const int SOIL_DRY_THRESH = 500;
const int DARK_THRESHOLD  = 10;
const int DETECT_DIST_CM  = 20;
const float TEMP_THRESHOLD = 29.0;
const int SOUND_THRESHOLD = 70;

// ── TIMING & SAMPLING RATES (IN MILLISECONDS) ──────────────
// Justifications: 
// 1. DHT11 hardware limit requires at least 2000ms between reads.
// 2. Greenhouse water levels, soil moisture, and LDR drift slowly; 1000ms captures real-world changes efficiently.
// 3. Serial data streams out at 1000ms to avoid flooding data logs.
const unsigned long DHT_SAMPLE_RATE_MS    = 2000;
const unsigned long FLOW_SAMPLE_RATE_MS   = 1000;
const unsigned long SERIAL_STREAM_RATE_MS = 1000;
const unsigned long LCD_UPDATE_RATE_MS    = 500;

#endif