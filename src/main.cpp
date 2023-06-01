#include <Arduino.h>

#include <map>

// LED pins
#define RED_PIN 27
#define YELLOW_PIN 26
#define GREEN_PIN 25

// Light Sensor
#define LIGHT_PIN 36

// Temp/Humidity Sensor
#define TEMP_HUMIDITY_PIN


void setup() {
  Serial.begin(9600);
  // set LED pins # as output
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(RED_PIN, HIGH);
  digitalWrite(GREEN_PIN, HIGH);
  digitalWrite(YELLOW_PIN, HIGH);
}
