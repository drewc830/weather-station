#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <esp_sleep.h>

#define SDA_PIN 4
#define SCL_PIN 5
#define BME_ADDR 0x77
#define SLEEP_TIME_SEC 60
#define BATTERY_PIN 34

const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

const char* serverUrl = "http://192.168.0.67:8000/weather";

Adafruit_BME280 bme;

void connectWiFi() {
    if (WiFi.status() == WL_CONNECTED) {
        return;
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    Serial.print("Connecting to WiFi");

    unsigned long startTime = millis();

    while (WiFi.status() != WL_CONNECTED &&
           millis() - startTime < 10000) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("WiFi connection timed out.");
    }
}

void postWeather(float temp, float humidity, float pressure, int signal) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected. Skipping POST.");
        return;
    }

    HTTPClient http;

    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    String json = "{";
    json += "\"temperature\":" + String(temp, 1) + ",";
    json += "\"humidity\":" + String(humidity, 1) + ",";
    json += "\"pressure\":" + String(pressure, 1) + ",";
    json += "\"rssi\":" + String(signal);
    json += "}";

    Serial.print("POST JSON: ");
    Serial.println(json);

    int responseCode = http.POST(json);

    Serial.print("POST response: ");
    Serial.println(responseCode);

    if (responseCode > 0) {
        Serial.print("Response body: ");
        Serial.println(http.getString());
    }

    http.end();
}

void enterDeepSleep() {
    Serial.println("Entering deep sleep...");
    Serial.flush();

    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    esp_sleep_enable_timer_wakeup(
        (uint64_t)SLEEP_TIME_SEC * 1000000ULL
    );

    esp_deep_sleep_start();
}

void setup() {
    Serial.begin(115200);
    delay(200);

    Wire.begin(SDA_PIN, SCL_PIN);
	pinMode(BATTERY_PIN, INPUT);
	
    if (!bme.begin(BME_ADDR, &Wire)) {
        Serial.println("BME280 not found.");
        delay(1000);
        enterDeepSleep();
    }

    Serial.println("BME280 ready.");

    connectWiFi();
}

void loop() {
    float temp = (9.0 / 5.0) * bme.readTemperature() + 32.0;
    float humidity = bme.readHumidity();
    float pressure = bme.readPressure() / 100.0F;
	
	int raw = analogRead(BATTERY_PIN);
	float adcVoltage = raw * (3.3 / 4095.0);
	float batteryVoltage = adcVoltage * 2.0;
	
    int signal = -127;

    if (WiFi.status() == WL_CONNECTED) {
        signal = WiFi.RSSI();
    }

    Serial.print("WiFi Status: ");
    Serial.println(WiFi.status());

    Serial.print("RSSI: ");
    Serial.print(signal);
    Serial.println(" dBm");

    Serial.printf(
        "Temp: %.1f F | Humidity: %.1f %% | Pressure: %.1f hPa\n",
        temp,
        humidity,
        pressure
    );
	
	int 
	
    postWeather(temp, humidity, pressure, signal);

    delay(500);

    enterDeepSleep();
}
