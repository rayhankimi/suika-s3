#ifndef MAIN_H
#define MAIN_H

//---------- DEBUG SETTINGS ----------
#define DEBUG_MODE true               // Enable debug output via Serial
//---------- SERIAL SETTINGS ----------
#define SERIAL_BAUD_RATE 115200       // Serial port baud rate
//---------- WEB SERVER SETTINGS ----------
#define WEB_SERVER_PORT 80            // Web server port
//---------- WIFI ACCESS POINT SETTINGS ----------
#define AP_IP_ADDR 192,168,4,1        // Access Point IP address (192.168.4.1)
#define AP_SSID "SuikaHW-Monitor"     // Access Point SSID
#define AP_PASSWORD "suikahw123"      // Access Point password (min 8 characters)
#define AP_CHANNEL 6                  // WiFi channel (1-13)
#define AP_HIDDEN 0                   // Hide SSID (0 = visible, 1 = hidden)
#define AP_MAX_CONNECTIONS 4          // Maximum concurrent connections

//---------- TIMEOUT SETTINGS ----------
#define DATA_TIMEOUT_MS 15000         // Data timeout in milliseconds (how long to wait before considering the connection lost)
#define MIN_REFRESH_INTERVAL_MS 1000  // Minimum time between data refreshes in milliseconds (to prevent buffer overflow) 

void TaskWebServerHandler(void *pvParameters);
void TaskSerialReader(void *pvParameters);

void handleRoot();
void handleData();
void handleRawData();
void handleStatus();
void handleCaptivePortal();

#endif // CONFIG_H 