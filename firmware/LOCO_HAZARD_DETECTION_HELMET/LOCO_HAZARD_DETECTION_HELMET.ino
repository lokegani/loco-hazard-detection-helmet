#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Wi-Fi Credentials
const char* ssid = "KLOKKY";
const char* password = "lokesh007*";

// Telegram Bot Credentials
const char* botToken = "7590404389:AAFsrwDEpmqzC00Tlj26Dz2J7Wz9HD6OKzc";
const char* chatID = "6457655080";

// GPS Setup
static const int RXPin = 16, TXPin = 17; // Change as per your ESP32 board
static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;
HardwareSerial gpsSerial(1);

// DHT Sensor
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Buzzer and RGB LED Pins
#define BUZZER_PIN 19
#define RED_PIN 23
#define GREEN_PIN 22
#define BLUE_PIN 21

// LCD Setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Thresholds
const float highTempThreshold = 30.0;
const float highHumidityThreshold = 70.0;
bool alertTriggered = false;
int smsCount = 0;
const int maxSmsCount = 2;

void setup() {
    Serial.begin(115200);
    gpsSerial.begin(GPSBaud, SERIAL_8N1, RXPin, TXPin);
    dht.begin();

    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(RED_PIN, OUTPUT);
    pinMode(GREEN_PIN, OUTPUT);
    pinMode(BLUE_PIN, OUTPUT);

    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.print("System Ready");
    delay(2000);

    Serial.println("\nConnecting to Wi-Fi...");
    WiFi.begin(ssid, password);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected to Wi-Fi!");
    } else {
        Serial.println("\nFailed to connect. Restarting...");
        ESP.restart();
    }
}

void sendLocationToTelegram(float latitude, float longitude) {
    if (WiFi.status() == WL_CONNECTED && smsCount < maxSmsCount) {
        HTTPClient http;
        String url = "https://api.telegram.org/bot" + String(botToken) +
                     "/sendLocation?chat_id=" + String(chatID) +
                     "&latitude=" + String(latitude, 6) +
                     "&longitude=" + String(longitude, 6);
        Serial.println("Sending location...");
        http.begin(url);
        int httpResponseCode = http.GET();
        if (httpResponseCode > 0) {
            Serial.println("Location sent!");
            smsCount++;
        } else {
            Serial.print("Error: ");
            Serial.println(httpResponseCode);
        }
        http.end();
    }
}

void loop() {
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    if (!isnan(humidity) && !isnan(temperature)) {
        Serial.print("Temp: "); Serial.print(temperature);
        Serial.print(" C | Humi: "); Serial.println(humidity);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Temp: ");
        lcd.print(temperature);
        lcd.print(" C");
        lcd.setCursor(0, 1);
        lcd.print("Humi: ");
        lcd.print(humidity);
        lcd.print(" %");

        if ((temperature > highTempThreshold || humidity > highHumidityThreshold) && !alertTriggered) {
            alertTriggered = true;
            digitalWrite(BUZZER_PIN, HIGH);
            setRGBColor(255, 0, 0);
            delay(15000); // Buzzer rings for 15 seconds
            digitalWrite(BUZZER_PIN, LOW);
            setRGBColor(0, 255, 0);
        } else if (temperature <= highTempThreshold && humidity <= highHumidityThreshold) {
            alertTriggered = false;
        }
    } else {
        Serial.println("DHT Sensor Error");
        lcd.clear();
        lcd.print("Sensor Error");
    }

    while (gpsSerial.available() > 0) {
        gps.encode(gpsSerial.read());
        if (gps.location.isUpdated()) {
            float latitude = gps.location.lat();
            float longitude = gps.location.lng();
            Serial.print("Lat: "); Serial.print(latitude, 6);
            Serial.print(" | Lon: "); Serial.println(longitude, 6);
            sendLocationToTelegram(latitude, longitude);
        }
    }
    delay(2000);
}

void setRGBColor(int red, int green, int blue) {
    ledcWrite(0, red);
    ledcWrite(1, green);
    ledcWrite(2, blue);
}
