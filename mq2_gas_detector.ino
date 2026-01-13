#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ========== CONFIGURABLE PARAMETERS ==========
const unsigned long INITIALIZATION_TIME = 60000;    // 60 seconds for sensor calibration
const int GAS_THRESHOLD = 375;                      // Gas threshold to activate buzzer and relay
const unsigned long PURGE_TIME = 60000;             // 1 minute fan purge time
const int BUZZER_PIN = 8;                           // Buzzer pin
const int RELAY_PIN = 4;                            // Relay module pin (fan control)
const int MQ2_PIN = A0;                             // MQ-2 sensor analog pin
const int NUM_READINGS = 30;                        // Number of readings for moving average
const int BUZZER_INTERVAL = 1000;                   // Interval in ms for intermittent buzzer

// ========== INITIALIZATION ==========
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Variables for moving average (smoothing)
int readings[NUM_READINGS];
int readIndex = 0;
int total = 0;
int average = 0;

// Time variables
unsigned long startTime = 0;
bool sensorReady = false;

// Variables for intermittent buzzer
unsigned long lastBuzzerToggle = 0;
bool buzzerOn = false;

// ========== VARIABLES FOR FAN PURGE ==========
unsigned long lastGasDetectedTime = 0;
bool gasWasDetected = false;

// ========== SETUP ==========
void setup() {
  Serial.begin(9600);
  
  // Configure buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  
  // Configure relay (fan)
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  
  // Initialize readings array
  for (int i = 0; i < NUM_READINGS; i++) {
    readings[i] = 0;
  }
  
  // Record start time
  startTime = millis();
  
  // Initial messages
  Serial.println("=== MQ-2 Gas Detector with LCD I2C, Buzzer and Fan ===");
  Serial.print("Initialization time: ");
  Serial.print(INITIALIZATION_TIME / 1000);
  Serial.println(" seconds");
  Serial.print("Gas threshold: ");
  Serial.println(GAS_THRESHOLD);
  Serial.print("Purge time: ");
  Serial.print(PURGE_TIME / 1000);
  Serial.println(" seconds");
  
  lcd.print("MQ-2 + Fan");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  
  delay(2000);
  lcd.clear();
}

// ========== FUNCTION: Calculate moving average (smooths signal) ==========
int calculateSmoothedValue(int newReading) {
  total = total - readings[readIndex];
  readings[readIndex] = newReading;
  total = total + readings[readIndex];
  readIndex = (readIndex + 1) % NUM_READINGS;
  return total / NUM_READINGS;
}

// ========== FUNCTION: Control intermittent buzzer ==========
void handleBuzzer(bool shouldBuzz) {
  unsigned long currentTime = millis();
  
  if (shouldBuzz) {
    if (currentTime - lastBuzzerToggle >= BUZZER_INTERVAL) {
      buzzerOn = !buzzerOn;
      digitalWrite(BUZZER_PIN, buzzerOn ? HIGH : LOW);
      lastBuzzerToggle = currentTime;
    }
  } else {
    if (buzzerOn) {
      digitalWrite(BUZZER_PIN, LOW);
      buzzerOn = false;
    }
  }
}

// ========== FUNCTION: Control relay (fan) with purge ==========
void handleRelayWithPurge(bool gasDetected) {
  unsigned long currentTime = millis();
  
  // If gas is detected NOW
  if (gasDetected) {
    digitalWrite(RELAY_PIN, LOW);  // Activate fan immediately
    lastGasDetectedTime = currentTime;  // Record the time
    gasWasDetected = true;
  }
  // If NO gas is detected NOW
  else {
    // If we are in the purge period
    if (gasWasDetected && (currentTime - lastGasDetectedTime < PURGE_TIME)) {
      digitalWrite(RELAY_PIN, LOW);  // Keep fan running
    }
    // If purge time has elapsed
    else {
      digitalWrite(RELAY_PIN, HIGH);   // Turn off fan
      gasWasDetected = false;
    }
  }
}

// ========== FUNCTION: Check initialization status ==========
bool isSensorReady() {
  unsigned long elapsedTime = millis() - startTime;
  return (elapsedTime >= INITIALIZATION_TIME);
}

// ========== FUNCTION: Get remaining initialization time ==========
unsigned long getRemainingInitTime() {
  unsigned long elapsedTime = millis() - startTime;
  if (elapsedTime >= INITIALIZATION_TIME) {
    return 0;
  }
  return (INITIALIZATION_TIME - elapsedTime) / 1000;
}

// ========== FUNCTION: Get remaining purge time ==========
unsigned long getRemainingPurgeTime() {
  unsigned long currentTime = millis();
  if (!gasWasDetected || (currentTime - lastGasDetectedTime >= PURGE_TIME)) {
    return 0;
  }
  return (PURGE_TIME - (currentTime - lastGasDetectedTime)) / 1000;
}

// ========== MAIN LOOP ==========
void loop() {
  // Read raw analog value
  int rawGasValue = analogRead(MQ2_PIN);
  
  // Calculate smoothed value (moving average)
  average = calculateSmoothedValue(rawGasValue);
  
  // Calculate voltage
  float voltage = average * 5.0 / 1023.0;
  
  // Check if sensor is ready
  sensorReady = isSensorReady();
  
  // ========== PRINT TO SERIAL ==========
  Serial.print("Raw value: ");
  Serial.print(rawGasValue);
  Serial.print(" | Smoothed: ");
  Serial.print(average);
  Serial.print(" | Voltage: ");
  Serial.print(voltage, 2);
  Serial.print("V | Status: ");
  
  if (!sensorReady) {
    Serial.print("WARM-UP (");
    Serial.print(getRemainingInitTime());
    Serial.println("s)");
  } else if (average > GAS_THRESHOLD) {
    Serial.println("ALERT! - Buzzer ACTIVE and Fan RUNNING");
  } else if (gasWasDetected && getRemainingPurgeTime() > 0) {
    Serial.print("PURGE - Fan running (");
    Serial.print(getRemainingPurgeTime());
    Serial.println("s)");
  } else {
    Serial.println("READY - Normal");
  }
  
  // ========== DISPLAY ON LCD ==========
  lcd.setCursor(0, 0);
  
  if (!sensorReady) {
    // Initialization mode - Show remaining time
    lcd.print("INIT: ");
    lcd.print(getRemainingInitTime());
    lcd.print("s ");
  } else {
    // Normal and purge mode - Show gas value
    lcd.print("Gas: ");
    lcd.print(average);
    lcd.print("    ");
  }
  
  lcd.setCursor(0, 1);
  
  if (!sensorReady) {
    // During initialization, show smoothed gas value
    lcd.print("Gas: ");
    lcd.print(average);
    lcd.print("   ");
  } else if (average > GAS_THRESHOLD) {
    // During alert, show voltage and gas value
    lcd.print("V:");
    lcd.print(voltage, 2);
    lcd.print(" G:");
    lcd.print(average);
  } else {
    // During normal operation
    lcd.print("V: ");
    lcd.print(voltage, 2);
    lcd.print("V  ");
  }
  
  // ========== BUZZER AND FAN LOGIC ==========
  bool shouldActivateAlarm = sensorReady && (average > GAS_THRESHOLD);
  
  handleBuzzer(shouldActivateAlarm);
  handleRelayWithPurge(shouldActivateAlarm);
  
  delay(100);
}

