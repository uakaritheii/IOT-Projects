#include <TFT_eSPI.h>
#include <AnimatedGIF.h>
#include "black_idle2.h"
#include "black_base.h"
#include "black_stars.h"
#include "fuzzystate1.h"
#include "idling.h"
#include "gstone.h"
#include "sstone.h"
#include "bstone.h"
#include "gidle.h"
#include "sidle.h"
#include "bidle.h"

#include <HardwareSerial.h>
#include <DFPlayerMini_Fast.h>

#define IDLE_TIMEOUT 2000 

TFT_eSPI tft = TFT_eSPI();
AnimatedGIF gif;

const int sensorPins[4] = {13, 12, 14, 27};
int coinTally = 0;
unsigned long lastCoinTime = 0;

HardwareSerial dfSerial(2);
DFPlayerMini_Fast dfPlayerFast;

// Track milestones
bool milestone30Reached = false;
bool milestone50Reached = false;
bool milestone70Reached = false;

// Current idle screen pointer
const uint8_t* currentIdleGIF = black_idle2;
size_t currentIdleGIFSize = sizeof(black_idle2);

bool coinInserted() {
  for (int i = 0; i < 4; i++) {
    if (digitalRead(sensorPins[i]) == HIGH) {
      return true;
    }
  }
  return false;
}

bool playGIFInterruptible(const uint8_t* gifData, size_t gifSize) {
  if (!gif.open((uint8_t*)gifData, gifSize, GIFDraw)) return false;

  tft.startWrite();
  bool interrupted = false;
  while (gif.playFrame(true, NULL)) {
    yield();
    if (coinInserted()) {
      interrupted = true;
      break;
    }
  }
  gif.close();
  tft.endWrite();
  return interrupted;
}

void playGIF(const uint8_t* gifData, size_t gifSize) {
  if (gif.open((uint8_t*)gifData, gifSize, GIFDraw)) {
    tft.startWrite();
    while (gif.playFrame(true, NULL)) {
      yield();
    }
    gif.close();
    tft.endWrite();
  }
}

void playMilestoneAudio(int milestone) {
  switch (milestone) {
    case 30: dfPlayerFast.play(5); break;
    case 50: dfPlayerFast.play(6); break;
    case 70: dfPlayerFast.play(4); break;
  }
  delay(2000); // Wait for audio to finish before GIF
}

void handleMilestones() {
  if (!milestone30Reached && coinTally >= 30) {
    milestone30Reached = true;
    currentIdleGIF = bidle;
    currentIdleGIFSize = sizeof(bidle);
    playMilestoneAudio(30);
    playGIF(bstone, sizeof(bstone));
  } else if (!milestone50Reached && coinTally >= 50) {
    milestone50Reached = true;
    currentIdleGIF = sidle;
    currentIdleGIFSize = sizeof(sidle);
    playMilestoneAudio(50);
    playGIF(sstone, sizeof(sstone));
  } else if (!milestone70Reached && coinTally >= 70) {
    milestone70Reached = true;
    currentIdleGIF = gidle;
    currentIdleGIFSize = sizeof(gidle);
    playMilestoneAudio(70);
    playGIF(gstone, sizeof(gstone));
  }
}

void playCoinAudio(int coinValue) {
  switch (coinValue) {
    case 1: dfPlayerFast.play(1); break;  
    case 5: dfPlayerFast.play(2); break; 
    case 10: dfPlayerFast.play(3); break; 
  }
}

void setup() {
  Serial.begin(57600); //Computer Serial Monitor
  Serial1.begin(115200, SERIAL_8N1, 19, 21);

  for (int i = 0; i < 4; i++) pinMode(sensorPins[i], INPUT);

  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  gif.begin(BIG_ENDIAN_PIXELS);

  dfSerial.begin(9600, SERIAL_8N1, 16, 17);
  dfPlayerFast.begin(dfSerial);
  dfPlayerFast.volume(30);
}

void loop() {
  if (millis() - lastCoinTime > IDLE_TIMEOUT) {
    bool wasInterrupted = playGIFInterruptible(currentIdleGIF, currentIdleGIFSize);
    if (wasInterrupted) {
      lastCoinTime = millis();
      return;
    }
    lastCoinTime = millis();
  }

  if (!coinInserted()) {
    delay(10);
    return;
  }

  lastCoinTime = millis();

  bool s1 = false, s2 = false, s4 = false, s5 = false;
  unsigned long lastTrigger = millis();

  while (millis() - lastTrigger < 150) {
    if (digitalRead(sensorPins[0]) == HIGH) { s1 = true; lastTrigger = millis(); }
    if (digitalRead(sensorPins[1]) == HIGH) { s2 = true; lastTrigger = millis(); }
    if (digitalRead(sensorPins[2]) == HIGH) { s4 = true; lastTrigger = millis(); }
    if (digitalRead(sensorPins[3]) == HIGH) { s5 = true; lastTrigger = millis(); }
    delay(2);
  }

  int sensorCount = s1 + s2 + s4 + s5;
  int coinValue = 0;

  if (s1 && sensorCount == 1) {
    coinValue = 1;
  } else if (s5) {
    coinValue = 10;
  } else {
    coinValue = 5;
  }

  int previousTally = coinTally;
  coinTally += coinValue;

  String message = "Total: " + String(coinTally) + " Baht";
  Serial1.print(message);
  Serial.println(message);

bool milestoneTriggered = false;

  // Milestone logic
  if ((previousTally < 30 && coinTally >= 30 && !milestone30Reached) ||
      (previousTally < 50 && coinTally >= 50 && !milestone50Reached) ||
      (previousTally < 70 && coinTally >= 70 && !milestone70Reached)) {
    handleMilestones();
    milestoneTriggered = true;
  }



// Only play coin audio + GIF if no milestone was triggered
  if (!milestoneTriggered) {
  playCoinAudio(coinValue);

  if (coinValue == 1) {
    playGIF(black_base, sizeof(black_base));
  } else if (coinValue == 10) {
    playGIF(black_stars, sizeof(black_stars));
  } else {
    playGIF(fuzzystate1, sizeof(fuzzystate1));
  }
}
//  Serial1.print("a");
  delay(1000);
}
