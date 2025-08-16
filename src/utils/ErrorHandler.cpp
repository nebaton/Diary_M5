#include "ErrorHandler.h"

// 静的メンバーの初期化
std::vector<ErrorEntry> ErrorHandler::errorLog;

// エラーコード定数の定義
const char* ErrorHandler::ERROR_SENSOR_INIT_FAILED = "SENSOR_INIT_FAILED";
const char* ErrorHandler::ERROR_SENSOR_READ_FAILED = "SENSOR_READ_FAILED";
const char* ErrorHandler::ERROR_STORAGE_INIT_FAILED = "STORAGE_INIT_FAILED";
const char* ErrorHandler::ERROR_STORAGE_WRITE_FAILED = "STORAGE_WRITE_FAILED";
const char* ErrorHandler::ERROR_NETWORK_CONNECT_FAILED = "NETWORK_CONNECT_FAILED";
const char* ErrorHandler::ERROR_NETWORK_UPLOAD_FAILED = "NETWORK_UPLOAD_FAILED";
const char* ErrorHandler::ERROR_DISPLAY_MODULE_INIT_FAILED = "DISPLAY_INIT_FAILED";
const char* ErrorHandler::ERROR_CONFIG_LOAD_FAILED = "CONFIG_LOAD_FAILED";
const char* ErrorHandler::ERROR_CONFIG_SAVE_FAILED = "CONFIG_SAVE_FAILED";

// ErrorEntryコンストラクタはヘッダーファイルでインライン定義済み

void ErrorHandler::logError(ErrorComponent component, const String& errorCode, 
                           const String& message, const String& context) {
    ErrorEntry entry(ErrorLevel::ERROR, component, errorCode, message, context);
    errorLog.push_back(entry);
    
    // ログサイズを制限
    if (errorLog.size() > MAX_LOG_ENTRIES) {
        errorLog.erase(errorLog.begin());
    }
    
    // シリアルにも出力
    Serial.println("[エラー] " + componentToString(component) + ": " + message);
}

void ErrorHandler::logWarning(ErrorComponent component, const String& errorCode, 
                             const String& message, const String& context) {
    ErrorEntry entry(ErrorLevel::WARNING, component, errorCode, message, context);
    errorLog.push_back(entry);
    
    // ログサイズを制限
    if (errorLog.size() > MAX_LOG_ENTRIES) {
        errorLog.erase(errorLog.begin());
    }
    
    // シリアルにも出力
    Serial.println("[警告] " + componentToString(component) + ": " + message);
}

void ErrorHandler::logInfo(ErrorComponent component, const String& message) {
    ErrorEntry entry(ErrorLevel::INFO, component, "INFO", message, "");
    errorLog.push_back(entry);
    
    // ログサイズを制限
    if (errorLog.size() > MAX_LOG_ENTRIES) {
        errorLog.erase(errorLog.begin());
    }
    
    // シリアルにも出力
    Serial.println("[情報] " + componentToString(component) + ": " + message);
}

std::vector<ErrorEntry> ErrorHandler::getRecentErrors(uint32_t count) {
    std::vector<ErrorEntry> recent;
    
    // 最新のエラーから指定数を取得
    uint32_t startIndex = (errorLog.size() > count) ? errorLog.size() - count : 0;
    
    for (uint32_t i = startIndex; i < errorLog.size(); i++) {
        recent.push_back(errorLog[i]);
    }
    
    return recent;
}

std::vector<ErrorEntry> ErrorHandler::getErrorsByComponent(ErrorComponent component) {
    std::vector<ErrorEntry> filtered;
    
    for (const auto& entry : errorLog) {
        if (entry.component == component) {
            filtered.push_back(entry);
        }
    }
    
    return filtered;
}

std::vector<ErrorEntry> ErrorHandler::getErrorsByLevel(ErrorLevel level) {
    std::vector<ErrorEntry> filtered;
    
    for (const auto& entry : errorLog) {
        if (entry.level == level) {
            filtered.push_back(entry);
        }
    }
    
    return filtered;
}

void ErrorHandler::clearLog() {
    errorLog.clear();
    Serial.println("エラーログをクリアしました");
}

bool ErrorHandler::saveLogToFile() {
    // 基本実装（後で拡張予定）
    Serial.println("エラーログの保存機能は未実装です");
    return false;
}

bool ErrorHandler::loadLogFromFile() {
    // 基本実装（後で拡張予定）
    Serial.println("エラーログの読み込み機能は未実装です");
    return false;
}

uint32_t ErrorHandler::getErrorCount() {
    return errorLog.size();
}

String ErrorHandler::levelToString(ErrorLevel level) {
    switch (level) {
        case ErrorLevel::INFO: return "情報";
        case ErrorLevel::WARNING: return "警告";
        case ErrorLevel::ERROR: return "エラー";
        case ErrorLevel::CRITICAL: return "重大";
        default: return "不明";
    }
}

String ErrorHandler::componentToString(ErrorComponent component) {
    switch (component) {
        case ErrorComponent::SENSOR: return "センサー";
        case ErrorComponent::STORAGE: return "ストレージ";
        case ErrorComponent::NETWORK: return "ネットワーク";
        case ErrorComponent::DISPLAY_MODULE: return "ディスプレイ";
        case ErrorComponent::SYSTEM: return "システム";
        default: return "不明";
    }
}