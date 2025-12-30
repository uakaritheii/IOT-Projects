#include <ESP32Servo.h>

// Pins for the color sensor
#define S0_PIN 16
#define S1_PIN 17
#define S2_PIN 18
#define S3_PIN 19
#define OUT_PIN 23

// Servo and LED pins
#define SERVO_PIN 25
#define RGB_RED_PIN 26
#define RGB_GREEN_PIN 27
#define RGB_BLUE_PIN 14

// Button and Ultrasonic pins
#define BUTTON_PIN 32
#define TRIG_PIN 33
#define ECHO_PIN 35

// Logic thresholds and timing
#define DISTANCE_THRESHOLD 8
#define DELAY_PROX_TO_SERVO 3000
#define SERVO_ROTATION_TIME 500
#define SERVO_HOLD_TIME 2000

// Servo movement constants
#define SERVO_STOP 90
#define SERVO_FORWARD 180
#define SERVO_REVERSE 0

Servo myServo;
int colorMode = 0; // 0=Red, 1=Green, 2=Blue

// Debounce and button tracking
bool lastButtonReading = HIGH;
bool buttonPressed = false;
unsigned long lastDebounceTime = 0;

// Servo state logic
enum ServoState { IDLE, WAITING, ROTATING_FWD, HOLDING, ROTATING_BACK };
ServoState servoState = IDLE;
unsigned long servoStateStartTime = 0;
bool servoTriggered = false;

void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("Full System Test - Fixed Button");
  
  // Prep color sensor
  pinMode(S0_PIN, OUTPUT);
  pinMode(S1_PIN, OUTPUT);
  pinMode(S2_PIN, OUTPUT);
  pinMode(S3_PIN, OUTPUT);
  pinMode(OUT_PIN, INPUT);
  digitalWrite(S0_PIN, HIGH);
  digitalWrite(S1_PIN, HIGH);
  
  // Prep servo
  myServo.attach(SERVO_PIN);
  myServo.write(SERVO_STOP);
  
  // Prep LEDs
  pinMode(RGB_RED_PIN, OUTPUT);
  pinMode(RGB_GREEN_PIN, OUTPUT);
  pinMode(RGB_BLUE_PIN, OUTPUT);
  
  // Prep button and ultrasonic
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  Serial.println("Mode: RED");
  Serial.println("Press button to change mode");
  
  // Quick hardware check
  digitalWrite(RGB_RED_PIN, HIGH);
  delay(500);
  digitalWrite(RGB_RED_PIN, LOW);
  
  myServo.write(SERVO_FORWARD);
  delay(500);
  myServo.write(SERVO_STOP);
}

// Get raw frequency from color sensor
int readColorFrequency(int s2, int s3) {
  digitalWrite(S2_PIN, s2);
  digitalWrite(S3_PIN, s3);
  return pulseIn(OUT_PIN, LOW, 100000);
}

// Get distance in cm
float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  return (duration * 0.0343) / 2;
}

// Handle button presses with debouncing
void checkButton() {
  bool reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonReading) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > 50) {
    if (reading == LOW && !buttonPressed) {
      buttonPressed = true;
      colorMode = (colorMode + 1) % 3;
      
      Serial.print("Mode changed to: ");
      if (colorMode == 0) Serial.println("RED");
      else if (colorMode == 1) Serial.println("GREEN");
      else Serial.println("BLUE");
    }
    if (reading == HIGH) {
      buttonPressed = false;
    }
  }
  lastButtonReading = reading;
}

void setRGBColor(bool r, bool g, bool b) {
  digitalWrite(RGB_RED_PIN, r ? HIGH : LOW);
  digitalWrite(RGB_GREEN_PIN, g ? HIGH : LOW);
  digitalWrite(RGB_BLUE_PIN, b ? HIGH : LOW);
}

// Manages the servo movement sequence
void handleServo() {
  unsigned long elapsed = millis() - servoStateStartTime;
  switch (servoState) {
    case IDLE:
      myServo.write(SERVO_STOP);
      break;
    case WAITING:
      if (elapsed >= DELAY_PROX_TO_SERVO) {
        Serial.println("Rotating Forward");
        servoState = ROTATING_FWD;
        servoStateStartTime = millis();
      }
      break;
    case ROTATING_FWD:
      myServo.write(SERVO_FORWARD);
      if (elapsed >= SERVO_ROTATION_TIME) {
        Serial.println("Holding");
        myServo.write(SERVO_STOP);
        servoState = HOLDING;
        servoStateStartTime = millis();
      }
      break;
    case HOLDING:
      if (elapsed >= SERVO_HOLD_TIME) {
        Serial.println("Rotating Back");
        servoState = ROTATING_BACK;
        servoStateStartTime = millis();
      }
      break;
    case ROTATING_BACK:
      myServo.write(SERVO_REVERSE);
      if (elapsed >= SERVO_ROTATION_TIME) {
        Serial.println("Back to Idle");
        myServo.write(SERVO_STOP);
        servoState = IDLE;
        servoTriggered = false;
      }
      break;
  }
}

void triggerServo() {
  if (!servoTriggered && servoState == IDLE) {
    servoTriggered = true;
    servoState = WAITING;
    servoStateStartTime = millis();
    Serial.println("Servo Triggered - Waiting 3s");
  }
}

void loop() {
  checkButton();
  handleServo();
  
  float distance = getDistance();
  int redFreq = readColorFrequency(LOW, LOW);
  int greenFreq = readColorFrequency(HIGH, HIGH);
  int blueFreq = readColorFrequency(LOW, HIGH);

  bool objectInRange = (distance > 0 && distance <= DISTANCE_THRESHOLD);
  bool redDetected = (redFreq < greenFreq && redFreq < blueFreq && redFreq > 0);
  bool greenDetected = (greenFreq < redFreq && greenFreq < blueFreq && greenFreq > 0);
  bool blueDetected = (blueFreq < redFreq && blueFreq < greenFreq && blueFreq > 0);

  // Match onboard LED to what the sensor sees
  if (redDetected) setRGBColor(true, false, false);
  else if (greenDetected) setRGBColor(false, true, false);
  else if (blueDetected) setRGBColor(false, false, true);
  else setRGBColor(false, false, false);
  
  // Kick off servo if color matches current mode
  if (objectInRange) {
    if (colorMode == 0 && redDetected) {
      Serial.println("RED in range");
      triggerServo();
    } else if (colorMode == 1 && greenDetected) {
      Serial.println("GREEN in range");
      triggerServo();
    } else if (colorMode == 2 && blueDetected) {
      Serial.println("BLUE in range");
      triggerServo();
    }
  }
  
  // Serial debugging
  Serial.print("D:"); Serial.print(distance, 1);
  Serial.print(" R:"); Serial.print(redFreq);
  Serial.print(" G:"); Serial.print(greenFreq);
  Serial.print(" B:"); Serial.print(blueFreq);
  Serial.print(" Mode:"); Serial.print(colorMode == 0 ? "R" : (colorMode == 1 ? "G" : "B"));
  Serial.print(" Servo:");
  
  switch(servoState) {
    case IDLE: Serial.println("IDLE"); break;
    case WAITING: Serial.println("WAIT"); break;
    case ROTATING_FWD: Serial.println("FWD"); break;
    case HOLDING: Serial.println("HOLD"); break;
    case ROTATING_BACK: Serial.println("BACK"); break;
  }
  
  delay(100);
}