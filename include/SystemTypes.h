#ifndef SYSTEM_TYPES_H
#define SYSTEM_TYPES_H

#include <Arduino.h>
#include <functional>

// 前方宣言
struct SensorReading;
struct SystemStatus;
struct SystemConfig;

// システム状態の列挙型
enum class StorageMode {
    ONLINE,
    OFFLINE, 
    HYBRID
};

enum class ConnectionStatus {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    ERROR
};

enum class RecoveryMode {
    MEMORY_QUEUE,
    TEMP_STORAGE,
    FULL_OFFLINE
};

enum class DisplayPage {
    SENSOR_DATA_1,
    SENSOR_DATA_2,
    STATUS,
    CONFIG
};

// コアセンサーデータ構造体
struct SensorReading {
    uint32_t timestamp;
    float temperature;
    float humidity;
    float pressure;
    float co2_equivalent;
    float iaq;
    float voc_equivalent;
    float gas_resistance;
    bool stabilized;
    float runin_status;
    String device_id;
    
    // データ品質フラグ
    bool has_co2_data;
    bool has_iaq_data;
    bool has_voc_data;
    bool is_calibrated;  // runin_status >= 75%
    
    // デフォルト値付きコンストラクタ
    SensorReading() : 
        timestamp(0), temperature(0), humidity(0), pressure(0),
        co2_equivalent(0), iaq(0), voc_equivalent(0), gas_resistance(0),
        stabilized(false), runin_status(0), device_id("M5Stack_001"),
        has_co2_data(false), has_iaq_data(false), has_voc_data(false), is_calibrated(false) {}
};

// システムステータス構造体
struct SystemStatus {
    bool sensor_healthy;
    ConnectionStatus connection_status;
    uint32_t storage_usage_percent;
    uint8_t battery_level;
    uint32_t uptime_seconds;
    uint32_t free_memory;
    int8_t wifi_strength;
    
    SystemStatus() :
        sensor_healthy(false), connection_status(ConnectionStatus::DISCONNECTED),
        storage_usage_percent(0), battery_level(0), uptime_seconds(0),
        free_memory(0), wifi_strength(-100) {}
};

// システム設定構造体
struct SystemConfig {
    String wifi_ssid;
    String wifi_password;
    String google_sheets_id;
    String api_key;
    uint32_t sampling_interval;
    bool auto_upload_enabled;
    StorageMode storage_mode;
    
    SystemConfig() :
        wifi_ssid(""), wifi_password(""), google_sheets_id(""),
        api_key(""), sampling_interval(3000), auto_upload_enabled(true),
        storage_mode(StorageMode::HYBRID) {}
};

// コールバック関数型
typedef std::function<void(const SensorReading&)> SensorCallback;
typedef std::function<void(const SystemStatus&)> StatusCallback;

#endif // SYSTEM_TYPES_H