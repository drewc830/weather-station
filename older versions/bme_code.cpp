#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SDA_PIN 4
#define SCL_PIN 5

#define BME_ADDR 0x77
#define OLED_ADDR 0x3D

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_BME280 bme;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
    Serial.begin(115200);
    delay(1000);

    Wire.begin(SDA_PIN, SCL_PIN);

    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        Serial.println("OLED not found.");
        while (1) delay(10);
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("OLED ready");
    display.display();

    if (!bme.begin(BME_ADDR, &Wire)) {
        Serial.println("BME280 not found.");
        while (1) delay(10);
    }

    Serial.println("BME280 ready.");
    delay(1000);
}

void loop() {
    float temp = (9.0 / 5.0) * bme.readTemperature() + 32.0;
    float humidity = bme.readHumidity();
    float pressure = bme.readPressure() / 100.0F;

    Serial.printf(
        "Temp: %.1f F | Humidity: %.1f %% | Pressure: %.1f hPa\n",
        temp,
        humidity,
        pressure
    );

    display.clearDisplay();

    display.setCursor(0, 0);
    display.printf("Temp: %.1f F", temp);

    display.setCursor(0, 16);
    display.printf("Hum:  %.1f %%", humidity);

    display.setCursor(0, 32);
    display.printf("Pres: %.0f hPa", pressure);

    display.display();

    delay(2000);
}
