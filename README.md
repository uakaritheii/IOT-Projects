# IoT & Embedded Systems Projects

A collection of embedded systems projects I help build. All projects were team-based, and I served as the **code lead** — meaning I wrote and integrated most of the software.

---

## Projects

| Project | Hardware | What it does |
|---------|----------|--------------|
| [Smart Piggy Bank](#smart-piggy-bank) | ESP32, IR sensors, TFT display | Detects coin denominations, plays animations, gamified savings |
| [Auto Umbrella](#auto-umbrella) | Arduino Mega, DHT22, rain sensor | Automatically deploys umbrella based on weather |
| [RGB Color Sorter](#rgb-color-sorter) | ESP32, TCS34725, servo | Sorts objects by color on a conveyor belt |

---

## Smart Piggy Bank

A piggy bank that knows what coins you're inserting and rewards you with animations and sounds as you save.

### How it works

**Coin detection** uses a 4-channel IR sensor array. Different coins block different numbers of sensors:
- 1 Baht (smallest) → triggers 1 sensor
- 5 Baht (medium) → triggers 2-4 sensors  
- 10 Baht (largest) → triggers all sensors including the outermost one

**Feedback system:**
- TFT display shows animated GIFs (idle, coin inserted, milestone reached)
- DFPlayer Mini plays sound effects for each coin type
- Different animations for 1/5/10 Baht coins

**Gamification:**
- Hit 30 Baht → unlock bronze skin + celebration animation
- Hit 50 Baht → unlock silver skin
- Hit 70 Baht → unlock gold skin
- Each milestone has unique audio and animation

### Files

```
piggy-bank-code/
├── gif_test1.ino          ← Main code
├── black_idle2.h          ← Idle animation (default)
├── black_base.h           ← 1 Baht coin animation
├── black_stars.h          ← 10 Baht coin animation
├── fuzzystate1.h          ← 5 Baht coin animation
├── bstone.h / bidle.h     ← Bronze milestone
├── sstone.h / sidle.h     ← Silver milestone
└── gstone.h / gidle.h     ← Gold milestone
```

The `.h` files contain GIF frames as byte arrays (converted using LCD Image Converter).

### Hardware

- ESP32 DevKit
- 4x IR obstacle sensors
- 1.8" TFT display (ST7735 driver)
- DFPlayer Mini + speaker
- I2C LCD (for tally display)

---

## Auto Umbrella

An automated umbrella that deploys itself when it detects rain or high UV.

### How it works

**Sensors:**
- DHT22 → temperature and humidity
- Rain sensor (analog) → detects water droplets
- Photoresistor → measures light intensity (proxy for UV)

**Control logic:**
```
if rain detected (analog < 700):
    deploy umbrella
else if comfort value > 1:
    deploy umbrella (too sunny/hot)
else if umbrella is out AND conditions are fine:
    retract umbrella
```

The "comfort value" combines light intensity and temperature into a single metric.

**Actuation:**
- 12V DC motor pulls a string attached to the umbrella spine
- BTS7960 H-bridge controls motor direction (deploy/retract)
- 9-second delay for full extension

### Files

```
auto-umbrella-code/
└── umbrella_code.ino
```

### Hardware

- Arduino Mega
- DHT22 temperature/humidity sensor
- Rain sensor module
- Photoresistor
- BTS7960 H-bridge motor driver
- 12V DC motor
- I2C LCD (16x2)

---

## RGB Color Sorter

A conveyor belt system that detects the color of objects and sorts them accordingly.

### How it works

**Detection:**
- TCS34725 color sensor reads RGB values
- Ultrasonic sensor detects when an object is in range
- Compares red/green/blue frequencies to determine dominant color

**Sorting:**
- Button cycles through modes: RED → GREEN → BLUE
- When an object matches the selected color, the servo activates
- Servo pushes the object off the belt into the correct bin

**State machine for servo:**
```
IDLE → WAITING (3s delay) → ROTATING_FWD → HOLDING → ROTATING_BACK → IDLE
```

This prevents the servo from triggering multiple times for the same object.

### Files

```
belt-code/
└── full_system_fixed.ino
```

### Hardware

- ESP32 DevKit
- TCS34725 color sensor
- HC-SR04 ultrasonic sensor
- Continuous rotation servo
- RGB LED (feedback indicator)
- Push button (mode select)
- 4x DC motors + BTS7960 drivers (for the conveyor belts)

---

## Demos

GIFs and videos of each project in action:

```
demos/
├── piggy-bank-demo.gif
├── umbrella-demo.gif
└── sorter-demo.gif
```

---

## How to run

All projects use the Arduino IDE.

1. Install required libraries:
   - **Piggy Bank:** `TFT_eSPI`, `AnimatedGIF`, `DFPlayerMini_Fast`
   - **Umbrella:** `DHT`, `LiquidCrystal_I2C`
   - **Sorter:** `ESP32Servo`

2. Select the correct board:
   - ESP32 projects: `ESP32 Dev Module`
   - Umbrella: `Arduino Mega 2560`

3. Upload and open Serial Monitor to debug.

---

## What I learned

- **Sensor fusion** — combining multiple inputs (IR array, temp + humidity + light) to make decisions
- **State machines** — essential for servo control and animation sequencing
- **Hardware debugging** — wiring issues, voltage drops, and sensor calibration take longer than writing code
- **Team coordination** — syncing hardware and software development across 3-5 people

---

All projects were team-based (3-5 people). I was responsible for software architecture and implementation.
