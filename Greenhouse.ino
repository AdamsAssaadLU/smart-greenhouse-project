#include <LiquidCrystal.h>
#include <SPI.h>
#include <MFRC522.h>
#include "Constants.h"
#include "Sensors.h"

LiquidCrystal lcd(22, 23, 24, 25, 26, 27);
MFRC522 rfid(SS_PIN, RST_PIN);

SensorData myData = {0, 0, 0, 0, 0, 0, false, 0, 0.0, 0.0};

bool systemEnabled = false;
byte authorizedUID[4] = {0x2A, 0xFB, 0x69, 0x32};

int lcdPage = 0;
bool lastButtonState = HIGH;

unsigned long lastLCDUpdate = 0;
unsigned long lastSerialStream = 0;
unsigned long buzzerStartTime = 0;
unsigned long soundBuzzerStart = 0;
bool buzzerActive = false;
bool soundBuzzerActive = false;
bool lastObjState = false;

void checkRFID();
void controlOutputs();
void handleButton();
void updateLCD();

void setup() {
  Serial.begin(9600);
  
  // PRINT CSV HEADER LINE FOR ANALYSIS TOOLS
  Serial.println("Timestamp_ms,Temp_C,Hum_Pct,WaterLevel_Raw,SoilMoisture_Raw,LDR_Raw,Distance_cm,Sound_Raw,FlowRate_Lm,TotalLiters");

  lcd.begin(16, 2);
  lcd.print("Greenhouse v2.0");
  lcd.setCursor(0, 1);
  lcd.print("Modular Boot...");
  delay(2000);
  lcd.clear();

  initSensors();
  SPI.begin();
  rfid.PCD_Init();

  pinMode(LED_WATER1, OUTPUT);
  pinMode(LED_WATER2, OUTPUT);
  pinMode(LED_WATER3, OUTPUT);
  pinMode(LED_RFID, OUTPUT);
  pinMode(LED_RFID_RED, OUTPUT);
  pinMode(LED_LDR1, OUTPUT);
  pinMode(LED_LDR2, OUTPUT);
  pinMode(SOIL_RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(FAN_PIN, OUTPUT);

  digitalWrite(SOIL_RELAY_PIN, HIGH);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_RFID, LOW);
  digitalWrite(LED_RFID_RED, LOW);
  digitalWrite(FAN_PIN, LOW);

  lcd.print("Scan RFID card");
  lcd.setCursor(0, 1);
  lcd.print("to start system");
}

void loop() {
  checkRFID();
  if (!systemEnabled) return;

  readAllSensors(myData);
  controlOutputs();
  handleButton();
  updateLCD();

  // CSV Data Logger Output (Satisfies assignment requirements)
  unsigned long now = millis();
  if (now - lastSerialStream >= SERIAL_STREAM_RATE_MS) {
    lastSerialStream = now;
    Serial.print(now); Serial.print(",");
    Serial.print(myData.temperature, 1); Serial.print(",");
    Serial.print(myData.humidity, 1); Serial.print(",");
    Serial.print(myData.waterLevel); Serial.print(",");
    Serial.print(myData.soilValue); Serial.print(",");
    Serial.print(myData.ldrValue); Serial.print(",");
    Serial.print(myData.distance); Serial.print(",");
    Serial.print(myData.soundLevel); Serial.print(",");
    Serial.print(myData.flowRate, 2); Serial.print(",");
    Serial.println(myData.totalLiters, 2);
  }
}

void checkRFID() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  if (rfid.uid.uidByte[0] == authorizedUID[0] && rfid.uid.uidByte[1] == authorizedUID[1] &&
      rfid.uid.uidByte[2] == authorizedUID[2] && rfid.uid.uidByte[3] == authorizedUID[3]) {
    systemEnabled = true;
    digitalWrite(LED_RFID_RED, LOW);
    digitalWrite(LED_RFID, HIGH);
    delay(1000);
    digitalWrite(LED_RFID, LOW);
    lcd.clear();
  } else {
    digitalWrite(LED_RFID, LOW);
    digitalWrite(LED_RFID_RED, HIGH);
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("Access denied!");
    lcd.setCursor(0, 1); lcd.print("Wrong card!     ");
    delay(2000);
    digitalWrite(LED_RFID_RED, LOW);
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("Scan RFID card");
    lcd.setCursor(0, 1); lcd.print("to start system");
  }
  rfid.PICC_HaltA();
}

void controlOutputs() {
  digitalWrite(FAN_PIN, myData.temperature > TEMP_THRESHOLD ? HIGH : LOW);

  if (myData.waterLevel < WATER_LOW) {
    digitalWrite(LED_WATER1, LOW); digitalWrite(LED_WATER2, LOW); digitalWrite(LED_WATER3, LOW);
  } else if (myData.waterLevel < WATER_MED) {
    digitalWrite(LED_WATER1, HIGH); digitalWrite(LED_WATER2, LOW); digitalWrite(LED_WATER3, LOW);
  } else if (myData.waterLevel < WATER_HIGH) {
    digitalWrite(LED_WATER1, HIGH); digitalWrite(LED_WATER2, HIGH); digitalWrite(LED_WATER3, LOW);
  } else {
    digitalWrite(LED_WATER1, HIGH); digitalWrite(LED_WATER2, HIGH); digitalWrite(LED_WATER3, HIGH);
  }

  digitalWrite(SOIL_RELAY_PIN, myData.soilValue > SOIL_DRY_THRESH ? LOW : HIGH);

  if (myData.ldrValue > DARK_THRESHOLD) {
    digitalWrite(LED_LDR1, HIGH); digitalWrite(LED_LDR2, HIGH);
  } else {
    digitalWrite(LED_LDR1, LOW); digitalWrite(LED_LDR2, LOW);
  }

  if (myData.objectNear && !lastObjState) {
    buzzerActive = true;
    buzzerStartTime = millis();
    digitalWrite(BUZZER_PIN, HIGH);
  }
  if (buzzerActive && millis() - buzzerStartTime >= 5000) {
    buzzerActive = false;
    digitalWrite(BUZZER_PIN, LOW);
  }
  lastObjState = myData.objectNear;

  if (myData.soundLevel > SOUND_THRESHOLD && !soundBuzzerActive) {
    soundBuzzerActive = true;
    soundBuzzerStart = millis();
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LED_RFID_RED, HIGH);
  }
  if (soundBuzzerActive && millis() - soundBuzzerStart >= 3000) {
    soundBuzzerActive = false;
    if (!buzzerActive) digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_RFID_RED, LOW);
  }
}

void handleButton() {
  bool currentState = digitalRead(BUTTON_PIN);
  if (currentState == LOW && lastButtonState == HIGH) {
    lcdPage = (lcdPage + 1) % 6;
    lcd.clear();
    delay(50);
  }
  lastButtonState = currentState;
}

void updateLCD() {
  unsigned long now = millis();
  if (now - lastLCDUpdate < LCD_UPDATE_RATE_MS) return;
  lastLCDUpdate = now;

  switch (lcdPage) {
    case 0:
      lcd.setCursor(0, 0); lcd.print("T:"); lcd.print(myData.temperature, 1);
      lcd.print((char)223); lcd.print("C Fan:"); lcd.print(myData.temperature > TEMP_THRESHOLD ? "ON " : "OFF");
      lcd.setCursor(0, 1); lcd.print("Hum: "); lcd.print(myData.humidity, 1); lcd.print("%        ");
      break;
    case 1:
      lcd.setCursor(0, 0); lcd.print("Flow:"); lcd.print(myData.flowRate, 2); lcd.print("L/m  ");
      lcd.setCursor(0, 1); lcd.print("Total:"); lcd.print(myData.totalLiters, 2); lcd.print("L    ");
      break;
    case 2:
      lcd.setCursor(0, 0); lcd.print("Water level:    ");
      lcd.setCursor(0, 1);
      if (myData.waterLevel < WATER_LOW) lcd.print("EMPTY  (0 LEDs) ");
      else if (myData.waterLevel < WATER_MED) lcd.print("LOW    (1 LED)  ");
      else if (myData.waterLevel < WATER_HIGH) lcd.print("MEDIUM (2 LEDs) ");
      else lcd.print("FULL   (3 LEDs) ");
      break;
    case 3:
      lcd.setCursor(0, 0); lcd.print("Soil moisture:  ");
      lcd.setCursor(0, 1);
      if (myData.soilValue > SOIL_DRY_THRESH) lcd.print("DRY  Pump: ON   ");
      else lcd.print("WET  Pump: OFF  ");
      break;
    case 4:
      lcd.setCursor(0, 0); lcd.print(myData.objectNear ? "!! OBJECT !!    " : "No object       ");
      lcd.setCursor(0, 1); lcd.print("Dist: "); lcd.print(myData.distance); lcd.print(" cm        ");
      break;
    case 5:
      lcd.setCursor(0, 0); lcd.print("Sound level:    ");
      lcd.setCursor(0, 1); lcd.print("Val:"); lcd.print(myData.soundLevel); lcd.print("  ");
      if (myData.soundLevel > SOUND_THRESHOLD) lcd.print("LOUD!   ");
      else lcd.print("quiet   ");
      break;
  }
}
