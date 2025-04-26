#include "web_handlers.h"
#include "parser.h"
#include <SPIFFS.h>
#include <FS.h>
#include <WiFi.h>

// External objects from main
extern SensorParser sensorParser;
extern unsigned long lastSerialRead;
extern unsigned long serialReadCount;

// Serve HTML file from SPIFFS
void serveFile(const String &path, const String &contentType) {
  if (SPIFFS.exists(path)) {
    fs::File file = SPIFFS.open(path, "r");
    if (file) {
      server.streamFile(file, contentType);
      file.close();
      return;
    }
  }
  
  server.send(404, "text/plain", "File not found: " + path);
}

// Captive portal handler
void handleCaptivePortal() {
#ifdef DEBUG_MODE
  Serial.print("Captive portal redirect: ");
  Serial.println(server.uri());
#endif

  if (server.hostHeader() != apIP.toString()) {
    server.sendHeader("Location", String("http://") + apIP.toString(), true);
    server.send(302, "text/plain", "");
    return;
  }

  handleRoot();
}

// HTML page handler
void handleRoot() {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  
  serveFile("/index.html", "text/html");
}

// Status handler
void handleStatus() {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");

  String json = "{";
  json.reserve(200);
  
  json += "\"hasValidData\":" + String(sensorParser.hasValidData() ? "true" : "false") + ",";
  json += "\"lastReadMs\":" + String(millis() - lastSerialRead) + ",";
  json += "\"serialReads\":" + String(serialReadCount) + ",";
  json += "\"parseAttempts\":" + String(sensorParser.getParseCount()) + ",";
  json += "\"parseErrors\":" + String(sensorParser.getErrorCount()) + ",";
  json += "\"uptime\":" + String(millis());

  String error = sensorParser.getError();
  if (error.length() > 0) {
    json += ",\"error\":\"" + error + "\"";
  }

  json += ",\"ip\":\"" + WiFi.softAPIP().toString() + "\"";

  json += "}";

  server.send(200, "application/json", json);
}

// JSON data handler
void handleData() {
  const SensorData& data = sensorParser.getData();

  String json = "{";
  json.reserve(256);
  
  json += "\"cpu_temp\":" + formatValue(data.cpu_temp) + ",";
  json += "\"cpu_usage\":" + formatValue(data.cpu_usage) + ",";
  json += "\"gpu_temp\":" + formatValue(data.gpu_temp) + ",";
  json += "\"gpu_usage\":" + formatValue(data.gpu_usage) + ",";
  json += "\"ram_usage\":" + formatValue(data.ram_usage) + ",";
  json += "\"ram_unit\":\"GB\",";
  json += "\"network_up\":" + formatValue(data.network_up) + ",";
  json += "\"network_down\":" + formatValue(data.network_down) + ",";
  json += "\"time\":" + String(millis());
  json += "}";

  server.send(200, "application/json", json);
}

// Raw data handler
void handleRawData() {
  const SensorData& data = sensorParser.getData();

  String json = "{";
  json.reserve(512);

  json += "\"status\":{";
  json += "\"serialReads\":" + String(serialReadCount) + ",";
  json += "\"parseAttempts\":" + String(sensorParser.getParseCount()) + ",";
  json += "\"parseErrors\":" + String(sensorParser.getErrorCount()) + ",";
  json += "\"lastReadMs\":" + String(millis() - lastSerialRead) + ",";
  json += "\"hasValidData\":" + String(sensorParser.hasValidData() ? "true" : "false");

  String error = sensorParser.getError();
  if (error.length() > 0) {
    json += ",\"error\":\"" + error + "\"";
  }
  json += "},";

  json += "\"rawJson\":\"" + sensorParser.getRawJson().substring(0, 200) + "\",";

  json += "\"sensors\":{";
  json += "\"cpuTemp\":\"" + formatValue(data.cpu_temp) + "\",";
  json += "\"cpuUsage\":\"" + formatValue(data.cpu_usage) + "\",";
  json += "\"gpuTemp\":\"" + formatValue(data.gpu_temp) + "\",";
  json += "\"gpuUsage\":\"" + formatValue(data.gpu_usage) + "\",";
  json += "\"ramUsage\":\"" + formatValue(data.ram_usage) + "\",";
  json += "\"networkUp\":\"" + formatValue(data.network_up) + "\",";
  json += "\"networkDown\":\"" + formatValue(data.network_down) + "\"";
  json += "}";

  json += "}";

  serveFile("/raw.html", "text/html");
}

// Helper: Format a float to 1 decimal place
String formatValue(float value) {
  char buffer[10];
  dtostrf(value, 1, 1, buffer);
  return String(buffer);
}
