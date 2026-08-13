#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <esp_sleep.h>

#define SDA_PIN 4
#define SCL_PIN 5
#define BME_ADDR 0x77
#define OLED_ADDR 0x3D
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SLEEP_TIME_SEC 60

const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

const char* serverUrl = "http://192.168.0.67:8000/weather";

Adafruit_BME280 bme;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void connectWiFi() {
    if (WiFi.status() == WL_CONNECTED) {
        return;
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    Serial.print("Connecting to WiFi");

    unsigned long startTime = millis();

    while (
        WiFi.status() != WL_CONNECTED &&
        millis() - startTime < 10000
    ) {
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

void postWeather(
    float temp,
    float humidity,
    float pressure,
    int signal
) {
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

    display.ssd1306_command(SSD1306_DISPLAYOFF);

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

    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        Serial.println("OLED not found.");
        delay(1000);
        enterDeepSleep();
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    if (!bme.begin(BME_ADDR, &Wire)) {
        Serial.println("BME280 not found.");

        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("BME280 not found");
        display.display();

        delay(1000);
        enterDeepSleep();
    }

    Serial.println("BME280 ready.");

    connectWiFi();
}

void loop() {
    float temp =
        (9.0 / 5.0) * bme.readTemperature() + 32.0;

    float humidity = bme.readHumidity();
    float pressure = bme.readPressure() / 100.0F;

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

    postWeather(temp, humidity, pressure, signal);

    display.clearDisplay();

    display.setCursor(0, 0);
    display.printf("Temp: %.1f F", temp);

    display.setCursor(0, 16);
    display.printf("Hum:  %.1f %%", humidity);

    display.setCursor(0, 32);
    display.printf("Pres: %.0f hPa", pressure);

    display.setCursor(0, 48);

    if (WiFi.status() == WL_CONNECTED) {
        display.printf("Sig:  %d dBm", signal);
    } else {
        display.print("WiFi: disconnected");
    }

    display.display();

    delay(500);

    enterDeepSleep();
}
