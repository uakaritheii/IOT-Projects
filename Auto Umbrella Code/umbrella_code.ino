#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);


const int lp = 51;
const int rs = A0;
const int ld = A1;
const int rpwm = 2;
const int lpwm = 3;
const int ren =  4;
const int len = 5;
bool ma = false;

DHT dht(50, DHT22);

void setup() {
  Wire.begin();
  Serial.begin(9600);
  dht.begin();
  pinMode(lp, OUTPUT); 
  pinMode(rs, INPUT);
  pinMode(ld, INPUT);
  pinMode(ren, OUTPUT);
  pinMode(len, OUTPUT);
  pinMode(rpwm, OUTPUT);
  pinMode(lpwm, OUTPUT);  

  digitalWrite(ren, HIGH);
  digitalWrite(len, HIGH);
  analogWrite(rpwm, 0);
  analogWrite(lpwm, 0);

  lcd.init();
  delay(1000);
  lcd.backlight();




}

void loop() {
  delay(2000);


  float rainValue = analogRead(rs);

  float h = dht.readHumidity(); //reads humidity
  float t = dht.readTemperature(); //reads temperature
  Serial.println(rainValue);
  Serial.println(lip());
  Serial.println(comfv());
  Serial.println(ma);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Humidity: ");
  lcd.print(h);
  lcd.setCursor(0,1);
  lcd.print("Temps: ");
  lcd.print(t);




  if (ma == false) {
      if (rainValue < 700) {
          digitalWrite(lp, HIGH);
          Serial.println("Rain Detected. Activating Umbrella");
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Rain Detected!");
          lcd.setCursor(0, 1);
          lcd.print("Deploy Umbrella");
          Serial.println(rainValue);
          analogWrite(rpwm, 0);
          analogWrite(lpwm, 255);
          delay(9000);
          analogWrite(lpwm, 0);
          ma = true;
      } else {
          Serial.print("Humidity (%): ");
          Serial.println(h);
          Serial.print("Temperature (C): ");
          Serial.println(t);

          if (comfv() > 1) {
              digitalWrite(lp, HIGH);
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("High UV Detected");
              lcd.setCursor(0, 1);
              lcd.print("Deploy Umbrella");
              analogWrite(rpwm, 0);
              analogWrite(lpwm, 255);
              delay(9000);
              analogWrite(lpwm, 0);
              ma = true;
          }
      }
  } else {
      if (ma == true) {
        if ((comfv() <= 1) && (rainValue > 700)){
          digitalWrite(lp, LOW);
          analogWrite(rpwm, 255);
          analogWrite(lpwm, 0);
          delay(9000);
          analogWrite(rpwm, 0);
          ma = false;
        }

      }

  }

}

float comfv(){
  float li = lip();
  float temp = dht.readTemperature();
  float tip = temp/23;
  float comfv = 0.5 * li + 0.5 * tip;
  return comfv;

}



float lip(){
  float lightValue = analogRead(ld);
  float intensity = (1023 - lightValue)/623;
  return intensity;
}

