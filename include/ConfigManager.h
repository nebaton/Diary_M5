#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include "SystemTypes.h"
#include <ArduinoJson.h>
#include <SPIFFS.h>

class ConfigManager {
private:
    SystemConfig currentConfig;
    bool configLoaded;
    String configFilePath;
    
    // JSONシリアライゼーションヘルパー
    bool serializeConfig(const SystemConfig& config, String& jsonString);
    bool deserializeConfig(const String& jsonString, SystemConfig& config);
    
    // ファイル操作
    bool readConfigFile(String& content);
    bool writeConfigFile(const String& content);
    
    // デフォルト設定
    void setDefaultConfig();

public:
    ConfigManager();
    ~ConfigManager();
    
    // コアインターフェースメソッド
    bool loadConfig();
    bool saveConfig(const SystemConfig& config);
    SystemConfig getCurrentConfig() const { return currentConfig; }
    void resetToDefaults();
    
    // 個別設定メソッド
    bool setWiFiCredentials(const String& ssid, const String& password);
    bool setGoogleSheetsId(const String& sheetsId);
    bool setApiKey(const String& apiKey);
    bool setSamplingInterval(uint32_t interval);
    bool setAutoUpload(bool enabled);
    bool setStorageMode(StorageMode mode);
    
    // バリデーションメソッド
    bool validateConfig(const SystemConfig& config);
    bool isConfigValid() const { return configLoaded; }
    
    // 定数
    static const char* CONFIG_FILE_PATH;
    static const uint32_t MIN_SAMPLING_INTERVAL = 1000; // 1秒
    static const uint32_t MAX_SAMPLING_INTERVAL = 300000; // 5分
};

#endif // CONFIG_MANAGER_H