#define ST128x160

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <FS.h>
#include "main.h"
#include "parser.h"
#include "web_handlers.h"
#include "esp_system.h"

// Web server
WebServer server(WEB_SERVER_PORT);

// DNS Server for captive portal
DNSServer dnsServer;
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1); // Hardcoded IP address for clarity

// Create sensor parser instance
SensorParser sensorParser;

// Globals for tracking connection status
unsigned long lastSerialRead = 0;
unsigned long serialReadCount = 0;
unsigned long lastDataRefresh = 0;

// TFT instance
TFT_eSPI tft = TFT_eSPI();

TaskHandle_t TaskSerialHandle;
TaskHandle_t TaskWebServerHandle;

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);

  if (!SPIFFS.begin(true)) {
    #ifdef DEBUG_MODE
    Serial.println("An Error has occurred while mounting SPIFFS");
    #endif
  }

  // Setup WiFi AP
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(AP_SSID, AP_PASSWORD, AP_CHANNEL, AP_HIDDEN, AP_MAX_CONNECTIONS);

  dnsServer.start(DNS_PORT, "*", apIP);

  // Setup WebServer Routes
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/raw", handleRawData);
  server.on("/status", handleStatus);
  server.on("/generate_204", handleCaptivePortal);
  server.on("/gen_204", handleCaptivePortal);
  server.on("/connecttest.txt", handleCaptivePortal);
  server.on("/redirect", handleCaptivePortal);
  server.on("/hotspot-detect.html", handleCaptivePortal);
  server.on("/canonical.html", handleCaptivePortal);
  server.on("/success.txt", handleCaptivePortal);
  server.on("/ncsi.txt", handleCaptivePortal);
  server.onNotFound(handleCaptivePortal);

  server.begin();

  #ifdef DEBUG_MODE
  Serial.print("Access Point IP: ");
  Serial.println(WiFi.softAPIP());
  Serial.print("SSID: ");
  Serial.println(AP_SSID);
  #endif

  // Create tasks
  xTaskCreatePinnedToCore(
    TaskSerialReader,   // Function
    "Serial Reader",    // Name
    4096,               // Stack Size
    NULL,               // Params
    1,                  // Priority
    &TaskSerialHandle,  // Handle
    0                   // Core 0
  );

  xTaskCreatePinnedToCore(
    TaskWebServerHandler,
    "Web Server",
    4096,
    NULL,
    1,
    &TaskWebServerHandle,
    1                   // Core 1
  );
}

void loop() {
  // Kosong! (jangan ada kerjaan berat di loop())
  delay(1000); 
}

// ==================== TASKS ====================

// Task untuk baca serial dan parsing sensor
void TaskSerialReader(void *pvParameters) {
  unsigned long currentMillis = millis();

  for (;;) {
    currentMillis = millis();

    if (Serial.available() > 0) {
      if (currentMillis - lastDataRefresh >= MIN_REFRESH_INTERVAL_MS) {
        lastDataRefresh = currentMillis;
        
        String data = Serial.readStringUntil('\n');
        if (data.length() > 0) {
          serialReadCount++;
          lastSerialRead = currentMillis;
          
          sensorParser.parse(data, false);
        }
      } else {
        Serial.read(); // clear buffer
      }
    }
    vTaskDelay(1); // supaya task lain sempat jalan (1 tick delay)
  }
}

// Task untuk DNS server dan WebServer
void TaskWebServerHandler(void *pvParameters) {
  for (;;) {
    dnsServer.processNextRequest();
    server.handleClient();
    vTaskDelay(1); // kasih waktu ke FreeRTOS scheduler
  }
}