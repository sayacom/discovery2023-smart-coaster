#include <Arduino.h>
#include <FreeRTOS.h>
#include <freertos/queue.h>
#include <WiFi.h>
#include "task.hpp"
#include "../utils/led_control.hpp"

void sendDataTask(void *pvParameters)
{
  WiFiClient client;
  int queuedValue;

  while (1)
  {
    // Serial.println("Waiting value...");
    xQueueReceive(sensorValueQueue, &queuedValue, portMAX_DELAY);
    Serial.println();
    Serial.print("Queued value: ");
    Serial.println(queuedValue);

    if (!client.connect("uni.soracom.io", 80))
    {
      Serial.println("Failed to connect...");
      delay(5000);
      continue;
    }

    Serial.printf("Sending force sensor value %lu \r\n", queuedValue);
    String json;
    json += "{\"sensorValue\": ";
    json.concat(static_cast<int>(queuedValue));
    json += "}";
    Serial.printf("payload: %s\r\n", json.c_str());

    client.write("POST / HTTP/1.1\r\n");
    client.write("Host: harvest.soracom.io\r\n");
    client.write("Connection: Keep-Alive\r\n");
    client.write("Keep-Alive: timeout=5, max=2\r\n");
    client.write("Content-Type: application/json\r\n");
    client.write("Content-Length: ");
    client.write(String(json.length(), 10).c_str());
    client.write("\r\n\r\n");
    client.write(json.c_str());

    while (client.connected())
    {
      auto line = client.readStringUntil('\n');
      Serial.write(line.c_str());
      Serial.write("\n");
      if (line == "\r")
        break;
    }
    if (client.connected())
    {
      uint8_t buffer[256];
      size_t bytesToRead = 0;
      while ((bytesToRead = client.available()) > 0)
      {
        bytesToRead = bytesToRead > sizeof(buffer) ? sizeof(buffer) : bytesToRead;
        auto bytesRead = client.readBytes(buffer, bytesToRead);
        Serial.write(buffer, bytesRead);
      }

      led_animation_t led = blinkAll(36000, 255, 192, 250, 2);
      xQueueSend(ledControlQueue, &led, portMAX_DELAY);
    }
  }
}
