#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <esp_sleep.h>
#include <secrets.h>

#define BME_ADDR 0x77
#define BATTERY_PIN 34
#define SLEEP_TIME_SEC 60
#define SDA_PIN 4
#define SCL_PIN 5

Adafruit_BME280 bme;

//Function to estimate the battery percent from voltage
//can be improved by geting more accurate curves
float batteryPercentFromVoltage(float voltage) {
    if (voltage >= 4.20f) {
        return 100.0f;
    }

    if (voltage >= 4.10f) {
        return 90.0f + ((voltage - 4.10f) / 0.10f) * 10.0f;
    }

    if (voltage >= 4.00f) {
        return 80.0f + ((voltage - 4.00f) / 0.10f) * 10.0f;
    }

    if (voltage >= 3.90f) {
        return 65.0f + ((voltage - 3.90f) / 0.10f) * 15.0f;
    }

    if (voltage >= 3.80f) {
        return 45.0f + ((voltage - 3.80f) / 0.10f) * 20.0f;
    }

    if (voltage >= 3.70f) {
        return 25.0f + ((voltage - 3.70f) / 0.10f) * 20.0f;
    }

    if (voltage >= 3.60f) {
        return 10.0f + ((voltage - 3.60f) / 0.10f) * 15.0f;
    }

    if (voltage >= 3.40f) {
        return 2.0f + ((voltage - 3.40f) / 0.20f) * 8.0f;
    }

    if (voltage >= 3.20f) {
        return ((voltage - 3.20f) / 0.20f) * 2.0f;
    }

    return 0.0f;
}

//function handles connecting to wifi
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

//function handles pushing data to API
void postWeather(
    float temperature,
    float humidity,
    float pressure,
    int rssi,
    float batteryVoltage,
    float batteryPercent
) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected. Skipping POST.");
        return;
    }

    HTTPClient http;

    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");
      
    String json = "{";
    json += "\"temperature\":" + String(temperature, 1) + ",";
    json += "\"humidity\":" + String(humidity, 1) + ",";
    json += "\"pressure\":" + String(pressure, 1) + ",";
    json += "\"rssi\":" + String(rssi) + ",";
    json += "\"battery_voltage\":" + String(batteryVoltage, 2) + ",";
    json += "\"battery_percent\":" + String(batteryPercent, 1);
    json += "}";

    Serial.print("POST JSON: ");
    Serial.println(json);

    int responseCode = http.POST(json);

    Serial.print("POST response: ");
    Serial.println(responseCode);

    if (responseCode > 0) {
        Serial.print("Response body: ");
        Serial.println(http.getString());
    } else {
        Serial.print("POST error: ");
        Serial.println(http.errorToString(responseCode));
    }

    http.end();
}

//sleep function to limit power consumption
void enterDeepSleep() {
    Serial.println("Entering deep sleep...");
    Serial.flush();

    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    esp_sleep_enable_timer_wakeup(
        static_cast<uint64_t>(SLEEP_TIME_SEC) * 1000000ULL
    );

    esp_deep_sleep_start();
}
//function to estimate battery voltage -- needs improvement
float readBatteryVoltage() {
    const int sampleCount = 20;
    uint32_t millivoltTotal = 0;

     //Discard the first reading after startup.
     //The ESP32 ADC can produce an unstable initial sample.
     
    analogReadMilliVolts(BATTERY_PIN);
    delay(10);
 
    //loop stores  samples to be averaged
    for (int i = 0; i < sampleCount; i++) {
        millivoltTotal += analogReadMilliVolts(BATTERY_PIN);
        delay(10);
    }
    //average the samples to improve accuracy
    float averageMillivolts =
        millivoltTotal / static_cast<float>(sampleCount);

     //convert to V
    float dividerVoltage = averageMillivolts / 1000.0f;

    //multiply by factor of 2 to get battery voltage
    //which accounts for equal resistance voltage divider
    float batteryVoltage = dividerVoltage * 2.0f;

    Serial.print("Average ADC millivolts: ");
    Serial.println(averageMillivolts, 1);

    Serial.print("Divider voltage: ");
    Serial.println(dividerVoltage, 3);

    return batteryVoltage;
}

void setup() {
    Serial.begin(115200);
    delay(200);
   
    pinMode(BATTERY_PIN, INPUT);
    analogReadResolution(12); //set ADC input to 12 bit resolution
    analogSetPinAttenuation(BATTERY_PIN, ADC_11db);  //set attenuation to 11dB to so ADC core does not saturate
	
    Wire.begin(SDA_PIN,SCL_PIN); //initalize I2C bus

    //make sure device is connected
    if (!bme.begin(BME_ADDR, &Wire)) { 
        Serial.println("BME280 not found.");
        delay(1000);
        enterDeepSleep();
    }

    Serial.println("BME280 ready.");

    connectWiFi();
}

void loop() {
    float temperatureC = bme.readTemperature();
    float temperatureF =
        (temperatureC * 9.0f / 5.0f) + 32.0f;
    float humidity = bme.readHumidity();
    float pressure = bme.readPressure() / 100.0f;
    float batteryVoltage = readBatteryVoltage();
    float batteryPercent =
        batteryPercentFromVoltage(batteryVoltage);

    batteryPercent = constrain(
        batteryPercent,
        0.0f,
        100.0f
    );
    
    //initalize dummy value indicating no signal
    int rssi = -127;

    if (WiFi.status() == WL_CONNECTED) {
        rssi = WiFi.RSSI();
    }

    Serial.print("WiFi Status: ");
    Serial.println(WiFi.status());

    Serial.print("RSSI: ");
    Serial.print(rssi);
    Serial.println(" dBm");

    Serial.printf(
        "Temp: %.1f F | Humidity: %.1f %% | Pressure: %.1f hPa\n",
        temperatureF,
        humidity,
        pressure
    );

    Serial.printf(
        "Battery: %.2f V | Charge: %.1f %%\n",
        batteryVoltage,
        batteryPercent
    );
    //push data to server
    postWeather(
        temperatureF,
        humidity,
        pressure,
        rssi,
        batteryVoltage,
        batteryPercent
    );

    delay(500);

    enterDeepSleep();
}
