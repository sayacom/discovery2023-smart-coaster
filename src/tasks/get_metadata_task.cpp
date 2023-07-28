#include <Arduino.h>
#include <FreeRTOS.h>
#include <freertos/queue.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "task.hpp"
#include "../utils/led_control.hpp"

void getMetadataTask(void *pvParameters)
{
  WiFiClient client;
  HTTPClient httpClient;
  StaticJsonDocument<2048> jsonBuffer;

  while (1)
  {
    // ----------- GET USERDATA ----------
    if (!httpClient.begin(client, "http://metadata.soracom.io/v1/userdata"))
    {
      Serial.println("[ERROR] Failed to get metadata.");
      vTaskDelay(60 * 1000);
      continue;
    }

    int statusCode = httpClient.GET();
    String responseBody = httpClient.getString();
    if (statusCode != HTTP_CODE_OK)
    {
      Serial.print("[WARN] Metadata(userdata) service returned ");
      Serial.println(statusCode);
      Serial.println("\tResponse: " + responseBody);

      vTaskDelay(60 * 1000);
      continue;
    }
    else
    {
      Serial.println("[OK] Get metadata(userdata) successful.");
      Serial.println("\tResponse: " + responseBody);
    }

    if (responseBody.length() > 0)
    {
      DeserializationError err = deserializeJson(jsonBuffer, responseBody);
      if (err)
      {
        Serial.println("[ERROR] Failed to parse response as JSON.");
        Serial.println("\tRaw: " + responseBody);
      }
      else
      {
        Serial.println("[OK] Parse response as JSON.");

        int _defaultLedHue = jsonBuffer["defaultLedHue"].as<int>();
        int _sensorCutoffMinValue = jsonBuffer["sensorCutoffMinValue"].as<int>();
        int _sensorCutoffMaxValue = jsonBuffer["sensorCutoffMaxValue"].as<int>();

        // Set to global variables
        DEFAULT_LED_HUE = _defaultLedHue;
        SENSOR_CUTOFF_MIN_VALUE = _sensorCutoffMinValue;
        SENSOR_CUTOFF_MAX_VALUE = _sensorCutoffMaxValue;
      }
    }

    // ---------- GET ALERT TAG ----------
    if (!httpClient.begin(client, "http://metadata.soracom.io/v1/subscriber.tags.alerting"))
    {
      Serial.println("[ERROR] Failed to get metadata(sim tag).");
      vTaskDelay(60 * 1000);
      continue;
    }

    statusCode = httpClient.GET();
    responseBody = httpClient.getString();
    if (statusCode != HTTP_CODE_OK)
    {
      Serial.print("[WARN] Metadata(tag) service returned ");
      Serial.println(statusCode);
      Serial.println("\tResponse: " + responseBody);

      vTaskDelay(60 * 1000);
      continue;
    }
    else
    {
      Serial.println("[OK] Get metadata(tag) successful.");
      responseBody.trim();
      Serial.println("\tResponse: " + responseBody);
    }

    if (responseBody.length() > 0)
    {
      if (responseBody.equals("ALERTING"))
      {
        Serial.println("Blink LED RED");
        led_animation_t led = blinkAll(0, 255, 192, 250, 20);
        xQueueSend(ledControlQueue, &led, portMAX_DELAY);
      }
      
      Serial.println("Updating tag...");
      httpClient.begin(client, "http://metadata.soracom.io/v1/subscriber/tags");
      httpClient.addHeader("Content-Type", "application/json");
      statusCode = httpClient.PUT("[{\"tagName\": \"alerting\", \"tagValue\":\"\"}]");
      if (statusCode != HTTP_CODE_OK)
      {
        Serial.print("[WARN] Put metadata(tag) service returned ");
        Serial.println(statusCode);
        Serial.println("\tResponse: " + responseBody);

        vTaskDelay(60 * 1000);
        continue;
      }
      else
      {
        Serial.println("[OK] Put metadata(tag) successful.");
        Serial.println("\tResponse: " + responseBody);
      }
    }

    httpClient.end();
    vTaskDelay(60 * 1000);
  }
}
