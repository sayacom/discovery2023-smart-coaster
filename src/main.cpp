#include <Arduino.h>
#include <FreeRTOS.h>
#include <freertos/queue.h>
#include <Wire.h>
#include <WiFi.h>
#include <WireGuard-ESP32.h>
#include <Adafruit_NeoPixel.h>

#include "./tasks/task.hpp"
#include "./utils/led_control.hpp"

// TODO: Update your own SSID
char ssid[] = "REPLACE_SSID_HERE";
char password[] = "REPLACE_WIFI_PASSWORD_HERE";

// TODO: Update your own WireGuard credentials
char privateKey[] = "REPLACE_WG_privateKey_HERE";
IPAddress localIp(0, 0, 0, 0);
char publicKey[] = "REPLACE_WG_publicKey_HERE";
char endpointAddress[] = "REPLACE_WG_ENDPOINT_HERE";
int endpointPort = 11010;

Adafruit_NeoPixel coasterLeds = Adafruit_NeoPixel(NUMPIXELS, COASTER_LED, NEO_GRB + NEO_KHZ800);

WireGuard wg;
QueueHandle_t sensorValueQueue;
QueueHandle_t ledControlQueue;

constexpr const uint32_t UPDATE_INTERVAL_MS = 15 * 1000;
int SENSOR_CUTOFF_MIN_VALUE = 100;
int SENSOR_CUTOFF_MAX_VALUE = 3200;
int DEFAULT_LED_HUE = 32000;

void setup()
{
  Serial.begin(115200);
  pinMode(FORCE_SENSOR, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  analogSetAttenuation(ADC_11db);
  coasterLeds.clear();
  coasterLeds.show();

  sensorValueQueue = xQueueCreate(1, sizeof(int));
  if (sensorValueQueue == NULL)
  {
    Serial.println("[!! Failed to create sensor value queue]");
  }

  ledControlQueue = xQueueCreate(1, sizeof(led_animation_t));
  if (ledControlQueue == NULL)
  {
    Serial.println("[!! Failed to create sensor value queue]");
  }

  delay(3 * 1000);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.print("Connecting to Wi-Fi...");
  bool done = true;
  WiFi.begin(ssid, password);
  while (done)
  {
    Serial.print("WiFi connecting");
    auto last = millis();
    while (WiFi.status() != WL_CONNECTED && last + 5000 > millis())
    {
      delay(500);
      Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED)
    {
      done = false;
    }
    else
    {
      Serial.println("retry");
      WiFi.disconnect();
      WiFi.reconnect();
    }
  }
  Serial.println("\nWiFi connected.");

  Serial.println();
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("DNS: ");
  Serial.println(WiFi.dnsIP());

  Serial.println("Adjusting system time...");
  configTime(9 * 60 * 60, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp", "time.google.com");

  unsigned long m;
  struct tm timeInfo;

  m = millis();
  getLocalTime(&timeInfo);
  Serial.printf("getLocalTime() %luMS\n", millis() - m);
  Serial.printf("%04d/%02d/%02d %02d:%02d:%02d\n", timeInfo.tm_year + 1900, timeInfo.tm_mon + 1, timeInfo.tm_mday, timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);

  Serial.println("Connected. Initializing WireGuard...");
  wg.begin(
      localIp,
      privateKey,
      endpointAddress,
      publicKey,
      endpointPort);

  xTaskCreatePinnedToCore(forceSensorTask, "forceSensor", 2048, NULL, 5, NULL, 0);
  xTaskCreatePinnedToCore(sendDataTask, "sendData", 16384, NULL, 4, NULL, 0);
  xTaskCreatePinnedToCore(ledControlTask, "ledControl", 2048, NULL, 3, NULL, 0);
  xTaskCreatePinnedToCore(getMetadataTask, "getMetadata", 16384, NULL, 4, NULL, 0);
}

void loop()
{
  delay(1);
}