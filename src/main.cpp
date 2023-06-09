#include <Arduino.h>
#include <map>

// Temp/Humidity & AWS imports:
#include <HttpClient.h>
#include <WiFi.h>
#include <inttypes.h>
#include <stdio.h>
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <DHT20.h>


// LED pins
#define RED_PIN 27
#define YELLOW_PIN 26
#define GREEN_PIN 25

// Light Sensor
#define LIGHT_PIN 36

// Categories
// Temperature
#define LOW_TEMP 22     // <= 22C
#define ROOM_TEMP 24    // 23 - 24
#define WARM_TEMP 29    // 25 - 29
#define HIGH_TEMP 30    // >= 30
// Humidity
#define LOW_HUMIDITY 28     // <= 28
#define NORMAL_HUMIDITY 55  // 28 - 54
#define HIGH_HUMIDITY 56    // >= 56
// Light
#define NO_LIGHT 0
#define DIM_LIGHT 1300
#define LOW_LIGHT 2500
#define BRIGHT_LIGHT 2501

// Patterns
#define GREEN_STATE 1 
#define YELLOW_STATE 2
#define RED_STATE 3
int current_state; 

int green[2][3] = {{ROOM_TEMP, NORMAL_HUMIDITY, BRIGHT_LIGHT}, {LOW_TEMP, LOW_HUMIDITY, LOW_LIGHT}};
int yellow[3][3] = {{ROOM_TEMP, NORMAL_HUMIDITY, NO_LIGHT}, {HIGH_TEMP, LOW_HUMIDITY, LOW_LIGHT}, {WARM_TEMP, HIGH_HUMIDITY, NO_LIGHT}};
int red[1][3] = {{WARM_TEMP, HIGH_HUMIDITY, DIM_LIGHT}};

// Wifi Connection & DHT20 Setup:
char ssid[50]; // your network SSID (name)
char pass[50]; // your network password (use for WPA, or use
// as key for WEP)
// Name of the server we want to connect to
const char kHostname[] = "worldtimeapi.org";
// Path to download (this is the bit after the hostname in the URL
// that you want to download
const char kPath[] = "/api/timezone/Europe/London.txt";
// Number of milliseconds to wait without receiving any data before we give up
const int kNetworkTimeout = 30 * 1000;
// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 1000;

// Global Variables
DHT20 DHT(&Wire);       // Temperature & Humidity Sensor

float currentLight;
void nvs_access() {
  // Initialize NVS
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
  err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    // NVS partition was truncated and needs to be erased
    // Retry nvs_flash_init
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);
  
  // Open
  Serial.printf("\n");
  Serial.printf("Opening Non-Volatile Storage (NVS) handle... ");
  nvs_handle_t my_handle;
  err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if (err != ESP_OK) {
    Serial.printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
  } else {
    Serial.printf("Done\n");
    Serial.printf("Retrieving SSID/PASSWD\n");
    size_t ssid_len;
    size_t pass_len;
    err = nvs_get_str(my_handle, "ssid", ssid, &ssid_len);
    err |= nvs_get_str(my_handle, "pass", pass, &pass_len);
    switch (err) {
      case ESP_OK:
        Serial.printf("Done\n");
        //Serial.printf("SSID = %s\n", ssid);
        //Serial.printf("PASSWD = %s\n", pass);
        break;
      case ESP_ERR_NVS_NOT_FOUND:
        Serial.printf("The value is not initialized yet!\n");
        break;
      default:
        Serial.printf("Error (%s) reading!\n", esp_err_to_name(err));
    }
  }
  // Close
  nvs_close(my_handle);
}

void wifiSetup() {
  // Retrieve SSID/PASSWD from flash before anything else
  nvs_access();
  // We start by connecting to a WiFi network
  delay(1000);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("MAC address: ");
  Serial.println(WiFi.macAddress());

  
  #if defined(ESP8266) || defined(ESP32)
  DHT.begin(21, 22);  //  select your pin numbers here
  #else
  DHT.begin();
  #endif

  Serial.println(__FILE__);
  Serial.print("DHT20 LIBRARY VERSION: ");
  Serial.println(DHT20_LIB_VERSION);
  Serial.println();
  delay(2000);
}

void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println();
  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
  wifiSetup();
  // set LED pins # as output
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  
  // Default value
  current_state = GREEN_STATE;
}

void loop() {
  // put your main code here, to run repeatedly:

  // LED Lights
  switch(current_state) {
    case YELLOW_STATE:
      digitalWrite(YELLOW_PIN, HIGH);
      digitalWrite(RED_PIN, LOW);
      digitalWrite(GREEN_PIN, LOW);
      break;
    case RED_STATE:
      digitalWrite(RED_PIN, HIGH);
      digitalWrite(YELLOW_PIN, LOW);
      digitalWrite(GREEN_PIN, LOW);
      break;
    default:
      digitalWrite(GREEN_PIN, HIGH);
      digitalWrite(RED_PIN, LOW);
      digitalWrite(YELLOW_PIN, LOW);
      break;
  }
  

  currentLight = analogRead(LIGHT_PIN);
  Serial.println("\nLight Sensor Read Value: " + (String) currentLight);

  // READ DATA
  Serial.println();
  Serial.print("DHT20, \t");
  int status = DHT.read();
  switch (status)
  {
  case DHT20_OK:
    Serial.print("OK,\t");
    break;
  case DHT20_ERROR_CHECKSUM:
    Serial.print("Checksum error,\t");
    break;
  case DHT20_ERROR_CONNECT:
    Serial.print("Connect error,\t");
    break;
  case DHT20_MISSING_BYTES:
    Serial.print("Missing bytes,\t");
    break;
  case DHT20_ERROR_BYTES_ALL_ZERO:
    Serial.print("All bytes read zero");
    break;
  case DHT20_ERROR_READ_TIMEOUT:
    Serial.print("Read time out");
    break;
  case DHT20_ERROR_LASTREAD:
    Serial.print("Error read too fast");
    break;
  default:
    Serial.print("Unknown error,\t");
    break;
  }

  //  DISPLAY DATA, sensor has only one decimal.
  Serial.print(DHT.getHumidity(), 1);
  Serial.print(",\t");
  Serial.println(DHT.getTemperature(), 1);

  int err = 0;
  WiFiClient c;
  HttpClient http(c);
  // HTTPClient http;
  //  err = http.get(kHostname, kPath);
  char query[50];
  sprintf(query, "/?var=Temperature=%f&Humidity=%f", DHT.getTemperature(), DHT.getHumidity());
  // Serial.println(query);
  err = http.get("3.145.196.4", 5000, query);
  int httpResponseCode = http.post("3.145.196.4", 5000, query);
  
  

  if (err == 0) {
  Serial.println("startedRequest ok");
  err = http.responseStatusCode();
  if (err >= 0) {
    Serial.print("Got status code: ");
    Serial.println(err);
    // Usually you'd check that the response code is 200 or a
    // similar "success" code (200-299) before carrying on,
    // but we'll print out whatever response we get
    err = http.skipResponseHeaders();
    if (err >= 0) {
      int bodyLen = http.contentLength();
      Serial.print("Content length is: ");
      Serial.println(bodyLen);
      Serial.println();
      Serial.println("Body returned follows:");
      // Now we've got to the body, so we can print it out
      unsigned long timeoutStart = millis();
      char c;
      // Whilst we haven't timed out & haven't reached the end of the body
      while ((http.connected() || http.available()) && ((millis() - timeoutStart) < kNetworkTimeout)) {
        if (http.available()) {
          c = http.read();
          // Print out this character
          Serial.print(c);
          bodyLen--;
          // We read something, reset the timeout counter
          timeoutStart = millis();
        } else {
          // We haven't got any data, so let's pause to allow some to
          // arrive
          
          delay(kNetworkDelay);
        }
      }
      } else {
        Serial.print("Failed to skip response headers: ");
        Serial.println(err);
      }
    } else {
      Serial.print("Getting response failed: ");
      Serial.println(err);
    }
  } else {
    Serial.print("Connect failed: ");
    Serial.println(err);
  }
  http.stop();

  // Category Checking
  delay(2000);

}
