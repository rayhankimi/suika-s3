/*
  SensorParser.h - Library for parsing sensor data
  
  This header provides functions for parsing JSON sensor data
  from PC monitoring software.
  
  Version 1.0
*/

#ifndef PARSER_H
#define PARSER_H

#include <Arduino.h>
#include <ArduinoJson.h>

// Temperature scale adjustment (some sensors report Kelvin instead of Celsius)
#define ADJUST_TEMP(t) ((t > 150) ? (t - 273.15) : t) // Convert Kelvin to Celsius if needed

/**
 * Data structure to store sensor values
 * All values are stored as floats for consistent handling
 */
struct SensorData {
  float cpu_usage = 0;
  float gpu_usage = 0;
  float cpu_temp = 0;
  float gpu_temp = 0;
  float ram_usage = 0;
  float network_up = 0;
  float network_down = 0;
  
  // Last update time
  unsigned long last_update = 0;
};

/**
 * SensorParser class - handles JSON parsing for hardware sensor data
 * 
 * This class encapsulates all the logic for parsing the JSON data
 * sent from the PC monitoring software, with error handling and
 * fallback mechanisms for inconsistent data formats.
 */
class SensorParser {
  private:
    SensorData _sensorData;
    String _rawJson = "{}";
    String _parsingError = "";
    bool _hasValidData = false;
    
    // Tracking counters for statistics
    uint16_t _parseCount = 0;
    uint16_t _errorCount = 0;

    // Helper enum for sensor types
    enum SensorType {
      CPU_USAGE = 0,
      GPU_USAGE,
      CPU_TEMP,
      GPU_TEMP,
      RAM_USAGE,
      NETWORK_UP,
      NETWORK_DOWN,
      UNKNOWN
    };

    // Identify sensor type from role or key
    SensorType _identifySensor(const char* role, const char* key, const char* name) {
      // Use role for primary identification
      if (role && *role) {
        if (strcmp(role, "cpu_usage") == 0) return CPU_USAGE;
        if (strcmp(role, "gpu_usage") == 0) return GPU_USAGE;
        if (strcmp(role, "cpu_temp") == 0) return CPU_TEMP;
        if (strcmp(role, "gpu_temp") == 0) return GPU_TEMP;
        if (strcmp(role, "ram_usage") == 0) return RAM_USAGE;
        if (strcmp(role, "network_up") == 0) return NETWORK_UP;
        if (strcmp(role, "network_down") == 0) return NETWORK_DOWN;
      }
      
      // Fallback to key prefix and name
      if (key && *key) {
        char firstChar = key[0];
        
        switch (firstChar) {
          case 'c': return CPU_USAGE;
          case 'g': return GPU_USAGE;
          case 'm': return RAM_USAGE;
          case 't': 
            if (name && strstr(name, "CPU")) return CPU_TEMP;
            if (name && strstr(name, "GPU")) return GPU_TEMP;
            break;
          case 'n': return NETWORK_UP; // Just assume NETWORK_UP, will handle proper assignment later
        }
      }
      
      return UNKNOWN;
    }

    // Set sensor value based on type
    void _setSensorValue(SensorType type, float value, bool& foundCpuTemp, bool& foundGpuTemp, 
                        bool& foundCpuUsage, bool& foundGpuUsage, bool& foundRamUsage, 
                        bool& foundNetUp, bool& foundNetDown) {
                          
      switch (type) {
        case CPU_TEMP:
          if (!foundCpuTemp) {
            _sensorData.cpu_temp = ADJUST_TEMP(value);
            foundCpuTemp = true;
          }
          break;
          
        case GPU_TEMP:
          if (!foundGpuTemp) {
            _sensorData.gpu_temp = ADJUST_TEMP(value);
            foundGpuTemp = true;
          }
          break;
          
        case CPU_USAGE:
          if (!foundCpuUsage) {
            _sensorData.cpu_usage = value;
            foundCpuUsage = true;
          }
          break;
          
        case GPU_USAGE:
          if (!foundGpuUsage) {
            _sensorData.gpu_usage = value;
            foundGpuUsage = true;
          }
          break;
          
        case RAM_USAGE:
          if (!foundRamUsage) {
            _sensorData.ram_usage = value;
            foundRamUsage = true;
          }
          break;
          
        case NETWORK_UP:
          if (!foundNetUp) {
            _sensorData.network_up = value;
            foundNetUp = true;
          }
          break;
          
        case NETWORK_DOWN:
          if (!foundNetDown) {
            _sensorData.network_down = value;
            foundNetDown = true;
          }
          break;
          
        default:
          // Unknown sensor, ignore
          break;
      }
    }

  public:
    /**
     * Constructor - initializes the parser
     */
    SensorParser() {}
    
    /**
     * Reset all sensor data and status flags to initial values
     */
    void reset() {
      _sensorData = SensorData(); // Reset all values to defaults
      _rawJson = "{}";
      _parsingError = "";
      _hasValidData = false;
    }
    
    /**
     * Get the raw JSON string from the last parse operation
     * @return The raw JSON string
     */
    String getRawJson() const {
      return _rawJson;
    }
    
    /**
     * Get the last parsing error message if any
     * @return The error message string or empty if no error
     */
    String getError() const {
      return _parsingError;
    }
    
    /**
     * Get the current sensor data structure
     * @return SensorData structure with all sensor values
     */
    const SensorData& getData() const {
      return _sensorData;
    }
    
    /**
     * Check if the parser has valid sensor data
     * @return true if valid data was parsed successfully
     */
    bool hasValidData() const {
      return _hasValidData;
    }
    
    /**
     * Get the total number of parse attempts
     * @return The parse count
     */
    uint16_t getParseCount() const {
      return _parseCount;
    }
    
    /**
     * Get the total number of parse errors
     * @return The error count
     */
    uint16_t getErrorCount() const {
      return _errorCount;
    }
    
    /**
     * Parse JSON data from string
     * @param jsonData The JSON string to parse
     * @param debugMode Enable/disable debug output (not used anymore)
     * @return true if parsing was successful and at least one sensor was found
     */
    bool parse(const String& jsonData, bool debugMode = false) {
      (void)debugMode; // Suppress unused parameter warning
      
      // Increment parse counter
      _parseCount++;
      
      // Store raw data for debugging
      _rawJson = jsonData;
      
      // Allocate JSON document with small buffer
      StaticJsonDocument<1024> doc;
      
      // Parse JSON
      DeserializationError error = deserializeJson(doc, jsonData);
      
      // Check for parsing errors
      if (error) {
        _parsingError = String(F("JSON error: ")) + error.c_str();
        _errorCount++;
        return false;
      }
      
      // Reset found flags
      bool foundAny = false;
      bool foundCpuUsage = false;
      bool foundGpuUsage = false;
      bool foundCpuTemp = false;
      bool foundGpuTemp = false;
      bool foundRamUsage = false;
      bool foundNetUp = false;
      bool foundNetDown = false;
    
      // Check if empty
      JsonObject root = doc.as<JsonObject>();
      if (root.size() == 0) {
        _parsingError = F("Empty JSON");
        _errorCount++;
        return false;
      }
      
      // First pass: Process all sensors
      for (JsonPair p : root) {
        const char* key = p.key().c_str();
        
        // Skip if not JSON object
        if (!p.value().is<JsonObject>()) continue;
        
        JsonObject sensor = p.value().as<JsonObject>();
        if (!sensor.containsKey("v")) continue; // Value is required
        
        foundAny = true;
        
        const char* name = sensor.containsKey("n") ? sensor["n"].as<const char*>() : "";
        const char* role = sensor.containsKey("r") ? sensor["r"].as<const char*>() : "";
        float val = sensor["v"].as<float>();
        
        // Identify sensor and set value
        SensorType type = _identifySensor(role, key, name);
        _setSensorValue(type, val, foundCpuTemp, foundGpuTemp, foundCpuUsage, 
                       foundGpuUsage, foundRamUsage, foundNetUp, foundNetDown);
      }
      
      // Update timestamp on success
      if (foundAny) {
        _sensorData.last_update = millis();
        _hasValidData = true;
      } else {
        _parsingError = F("No valid sensor data found");
        _errorCount++;
      }
      
      return foundAny;
    }
};

#endif // SENSOR_PARSER_H 