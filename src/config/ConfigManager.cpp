#include "ConfigManager.h"
#include "ErrorHandler.h"

// 静的メンバーの初期化
const char* ConfigManager::CONFIG_FILE_PATH = "/config.json";

ConfigManager::ConfigManager() :
    configLoaded(false),
    configFilePath(CONFIG_FILE_PATH) {
}

ConfigManager::~ConfigManager() {
}

bool ConfigManager::loadConfig() {
    Serial.println("設定ファイルを読み込み中...");
    
    // SPIFFSを初期化
    if (!SPIFFS.begin(true)) {
        ErrorHandler::logError(ErrorComponent::SYSTEM, ErrorHandler::ERROR_CONFIG_LOAD_FAILED, 
                              "SPIFFSの初期化に失敗しました");
        setDefaultConfig();
        return false;
    }
    
    // 設定ファイルの存在確認
    if (!SPIFFS.exists(configFilePath)) {
        Serial.println("設定ファイルが存在しません。デフォルト設定を使用します");
        setDefaultConfig();
        configLoaded = true;
        return true;
    }
    
    // ファイルから設定を読み込み
    String content;
    if (!readConfigFile(content)) {
        ErrorHandler::logError(ErrorComponent::SYSTEM, ErrorHandler::ERROR_CONFIG_LOAD_FAILED, 
                              "設定ファイルの読み込みに失敗しました");
        setDefaultConfig();
        return false;
    }
    
    // JSONをパース
    if (!deserializeConfig(content, currentConfig)) {
        ErrorHandler::logError(ErrorComponent::SYSTEM, ErrorHandler::ERROR_CONFIG_LOAD_FAILED, 
                              "設定ファイルのパースに失敗しました");
        setDefaultConfig();
        return false;
    }
    
    // 設定の妥当性をチェック
    if (!validateConfig(currentConfig)) {
        ErrorHandler::logWarning(ErrorComponent::SYSTEM, "CONFIG_VALIDATION_FAILED", 
                                "設定に無効な値があります。デフォルト値で補完します");
        setDefaultConfig();
    }
    
    configLoaded = true;
    Serial.println("設定ファイルの読み込みが完了しました");
    return true;
}

bool ConfigManager::saveConfig(const SystemConfig& config) {
    Serial.println("設定ファイルを保存中...");
    
    // 設定の妥当性をチェック
    if (!validateConfig(config)) {
        ErrorHandler::logError(ErrorComponent::SYSTEM, ErrorHandler::ERROR_CONFIG_SAVE_FAILED, 
                              "無効な設定のため保存できません");
        return false;
    }
    
    // JSONにシリアライズ
    String jsonString;
    if (!serializeConfig(config, jsonString)) {
        ErrorHandler::logError(ErrorComponent::SYSTEM, ErrorHandler::ERROR_CONFIG_SAVE_FAILED, 
                              "設定のシリアライズに失敗しました");
        return false;
    }
    
    // ファイルに書き込み
    if (!writeConfigFile(jsonString)) {
        ErrorHandler::logError(ErrorComponent::SYSTEM, ErrorHandler::ERROR_CONFIG_SAVE_FAILED, 
                              "設定ファイルの書き込みに失敗しました");
        return false;
    }
    
    currentConfig = config;
    Serial.println("設定ファイルの保存が完了しました");
    return true;
}

void ConfigManager::setDefaultConfig() {
    currentConfig.wifi_ssid = "";
    currentConfig.wifi_password = "";
    currentConfig.google_sheets_id = "";
    currentConfig.api_key = "";
    currentConfig.sampling_interval = 3000; // 3秒間隔
    currentConfig.auto_upload_enabled = true;
    currentConfig.storage_mode = StorageMode::HYBRID;
    
    Serial.println("デフォルト設定を適用しました");
}

void ConfigManager::resetToDefaults() {
    setDefaultConfig();
    configLoaded = true;
}

bool ConfigManager::serializeConfig(const SystemConfig& config, String& jsonString) {
    JsonDocument doc;
    
    doc["wifi_ssid"] = config.wifi_ssid;
    doc["wifi_password"] = config.wifi_password;
    doc["google_sheets_id"] = config.google_sheets_id;
    doc["api_key"] = config.api_key;
    doc["sampling_interval"] = config.sampling_interval;
    doc["auto_upload_enabled"] = config.auto_upload_enabled;
    doc["storage_mode"] = (int)config.storage_mode;
    
    if (serializeJson(doc, jsonString) == 0) {
        return false;
    }
    
    return true;
}

bool ConfigManager::deserializeConfig(const String& jsonString, SystemConfig& config) {
    JsonDocument doc;
    
    DeserializationError error = deserializeJson(doc, jsonString);
    if (error) {
        Serial.println("JSON解析エラー: " + String(error.c_str()));
        return false;
    }
    
    config.wifi_ssid = doc["wifi_ssid"] | "";
    config.wifi_password = doc["wifi_password"] | "";
    config.google_sheets_id = doc["google_sheets_id"] | "";
    config.api_key = doc["api_key"] | "";
    config.sampling_interval = doc["sampling_interval"] | 3000;
    config.auto_upload_enabled = doc["auto_upload_enabled"] | true;
    config.storage_mode = (StorageMode)(doc["storage_mode"] | (int)StorageMode::HYBRID);
    
    return true;
}

bool ConfigManager::readConfigFile(String& content) {
    File file = SPIFFS.open(configFilePath, "r");
    if (!file) {
        return false;
    }
    
    content = file.readString();
    file.close();
    
    return true;
}

bool ConfigManager::writeConfigFile(const String& content) {
    File file = SPIFFS.open(configFilePath, "w");
    if (!file) {
        return false;
    }
    
    size_t bytesWritten = file.print(content);
    file.close();
    
    return bytesWritten > 0;
}

bool ConfigManager::validateConfig(const SystemConfig& config) {
    // サンプリング間隔の妥当性チェック
    if (config.sampling_interval < MIN_SAMPLING_INTERVAL || 
        config.sampling_interval > MAX_SAMPLING_INTERVAL) {
        return false;
    }
    
    // その他の妥当性チェックは必要に応じて追加
    return true;
}

// 個別設定メソッド
bool ConfigManager::setWiFiCredentials(const String& ssid, const String& password) {
    currentConfig.wifi_ssid = ssid;
    currentConfig.wifi_password = password;
    return saveConfig(currentConfig);
}

bool ConfigManager::setGoogleSheetsId(const String& sheetsId) {
    currentConfig.google_sheets_id = sheetsId;
    return saveConfig(currentConfig);
}

bool ConfigManager::setApiKey(const String& apiKey) {
    currentConfig.api_key = apiKey;
    return saveConfig(currentConfig);
}

bool ConfigManager::setSamplingInterval(uint32_t interval) {
    if (interval < MIN_SAMPLING_INTERVAL || interval > MAX_SAMPLING_INTERVAL) {
        return false;
    }
    currentConfig.sampling_interval = interval;
    return saveConfig(currentConfig);
}

bool ConfigManager::setAutoUpload(bool enabled) {
    currentConfig.auto_upload_enabled = enabled;
    return saveConfig(currentConfig);
}

bool ConfigManager::setStorageMode(StorageMode mode) {
    currentConfig.storage_mode = mode;
    return saveConfig(currentConfig);
}