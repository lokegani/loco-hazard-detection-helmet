#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 4      // Pin connected to the DHT11 data pin
#define DHTTYPE DHT11 // Define the sensor type (DHT11)
#define BUZZER_PIN 16 // Pin connected to the buzzer

DHT dht(DHTPIN, DHTTYPE);

// Set thresholds for high temperature and humidity
const float highTempThreshold = 30.0;      // High temperature threshold (in Celsius)
const float highHumidityThreshold = 70.0;  // High humidity threshold (in %)

// Duration for the buzzer to sound (in milliseconds)
const unsigned long buzzerDuration = 5000; // 5 seconds

// Variable to store the start time of the buzzer
unsigned long buzzerStartTime = 0;
bool buzzerActive = false;
bool buzzerTriggered = false;

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(BUZZER_PIN, OUTPUT); // Set the buzzer pin as output
}

void loop() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" *C");

  // Check if temperature or humidity exceeds the thresholds and the buzzer hasn't been triggered yet
  if ((temperature > highTempThreshold || humidity > highHumidityThreshold) && !buzzerTriggered) {
    digitalWrite(BUZZER_PIN, HIGH); // Turn on the buzzer
    buzzerStartTime = millis(); // Record the start time
    buzzerActive = true; // Mark the buzzer as active
    buzzerTriggered = true; // Mark the buzzer as triggered
  }

  // Check if the buzzer duration has elapsed
  if (buzzerActive && millis() - buzzerStartTime >= buzzerDuration) {
    digitalWrite(BUZZER_PIN, LOW); // Turn off the buzzer
    buzzerActive = false; // Reset the buzzer status
  }

  // Reset buzzer trigger when the conditions are back to normal
  if (temperature <= highTempThreshold && humidity <= highHumidityThreshold) {
    buzzerTriggered = false; // Allow the buzzer to be triggered again
  }

  delay(2000); // Wait a few seconds between measurements.
}
