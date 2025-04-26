#pragma once

#include <Arduino.h>
#include <WebServer.h>

// Externs so we can access from other files
extern WebServer server;
extern IPAddress apIP;

// Declare all handler functions
void serveFile(const String &path, const String &contentType);
void handleCaptivePortal();
void handleRoot();
void handleStatus();
void handleData();
void handleRawData();

// Additional helper
String formatValue(float value);
