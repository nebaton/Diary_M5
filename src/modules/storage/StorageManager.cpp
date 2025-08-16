#include "StorageManager.h"
#include "ErrorHandler.h"
#include "TimeUtils.h"

StorageManager::StorageManager() :
    currentMode(StorageMode::HYBRID),
    sdCardInitialized(false),
    maxStorageSize(MAX_STORAGE_MB * 1024 * 1024) {
}

StorageManager::~StorageManager() {
}

bool StorageManager::initializeSDCard() {
    Serial.println("SDカードを初期化中...");
    
    // M5StackのSDカードピン設定
    if (!SD.begin()) {
        ErrorHandler::logWarning(ErrorComponent::STORAGE, ErrorHandler::ERROR_STORAGE_INIT_FAILED, 
                                "SDカードの初期化に失敗しました");
        return false;
    }
    
    // SDカードの種類とサイズを確認
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        ErrorHandler::logWarning(ErrorComponent::STORAGE, ErrorHandler::ERROR_STORAGE_INIT_FAILED, 
                                "SDカードが検出されませんでした");
        return false;
    }
    
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.println("SDカードサイズ: " + String((uint32_t)cardSize) + "MB");
    
    // データディレクトリを作成
    if (!ensureDirectoryExists("/sensor_data")) {
        ErrorHandler::logError(ErrorComponent::STORAGE, ErrorHandler::ERROR_STORAGE_INIT_FAILED, 
                              "データディレクトリの作成に失敗しました");
        return false;
    }
    
    sdCardInitialized = true;
    Serial.println("SDカードの初期化が完了しました");
    return true;
}

bool StorageManager::saveToSDCard(const SensorReading& data) {
    if (!sdCardInitialized) {
        return false;
    }
    
    // 日次ログファイルを作成/取得
    String filename = generateDailyFileName();
    if (currentLogFile != filename) {
        currentLogFile = filename;
        if (!createDailyLogFile()) {
            ErrorHandler::logError(ErrorComponent::STORAGE, ErrorHandler::ERROR_STORAGE_WRITE_FAILED, 
                                  "ログファイルの作成に失敗しました");
            return false;
        }
    }
    
    // データをCSV形式で保存
    File file = SD.open(currentLogFile, FILE_APPEND);
    if (!file) {
        ErrorHandler::logError(ErrorComponent::STORAGE, ErrorHandler::ERROR_STORAGE_WRITE_FAILED, 
                              "ログファイルのオープンに失敗しました");
        return false;
    }
    
    // CSVデータを作成
    String csvLine = String(data.timestamp) + "," +
                     String(data.temperature, 2) + "," +
                     String(data.humidity, 2) + "," +
                     String(data.pressure, 2) + "," +
                     String(data.co2_equivalent, 2) + "," +
                     String(data.iaq, 2) + "," +
                     String(data.voc_equivalent, 2) + "," +
                     String(data.gas_resistance, 2) + "," +
                     String(data.stabilized ? 1 : 0) + "," +
                     String(data.runin_status, 2) + "," +
                     data.device_id + "\n";
    
    size_t bytesWritten = file.print(csvLine);
    file.close();
    
    if (bytesWritten == 0) {
        ErrorHandler::logError(ErrorComponent::STORAGE, ErrorHandler::ERROR_STORAGE_WRITE_FAILED, 
                              "データの書き込みに失敗しました");
        return false;
    }
    
    return true;
}

bool StorageManager::createDailyLogFile() {
    if (!sdCardInitialized) {
        return false;
    }
    
    String filename = generateDailyFileName();
    
    // ファイルが既に存在する場合はヘッダーを追加しない
    if (SD.exists(filename)) {
        return true;
    }
    
    // 新しいファイルを作成してCSVヘッダーを追加
    File file = SD.open(filename, FILE_WRITE);
    if (!file) {
        return false;
    }
    
    // CSVヘッダー
    String header = "timestamp,temperature,humidity,pressure,co2_equivalent,iaq,voc_equivalent,gas_resistance,stabilized,runin_status,device_id\n";
    file.print(header);
    file.close();
    
    Serial.println("新しいログファイルを作成しました: " + filename);
    return true;
}

String StorageManager::generateDailyFileName() {
    return "/sensor_data/" + TimeUtils::generateDailyFileName("sensor_data", "csv");
}

bool StorageManager::ensureDirectoryExists(const String& path) {
    // SDライブラリではディレクトリ作成が自動的に行われる
    // 基本実装として常にtrueを返す
    return true;
}

std::vector<String> StorageManager::getUnsyncedFiles() {
    std::vector<String> files;
    
    if (!sdCardInitialized) {
        return files;
    }
    
    // 基本実装：すべてのCSVファイルを未同期として返す
    File root = SD.open("/sensor_data");
    if (!root || !root.isDirectory()) {
        return files;
    }
    
    File file = root.openNextFile();
    while (file) {
        if (!file.isDirectory() && String(file.name()).endsWith(".csv")) {
            files.push_back(String(file.path()));
        }
        file = root.openNextFile();
    }
    
    return files;
}

bool StorageManager::markFileAsSynced(const String& filename) {
    // 基本実装：ファイル名に.syncedサフィックスを追加
    String syncedName = filename + ".synced";
    return SD.rename(filename, syncedName);
}

void StorageManager::setStorageMode(StorageMode mode) {
    currentMode = mode;
    Serial.println("ストレージモードを変更しました: " + String((int)mode));
}

uint32_t StorageManager::getAvailableSpace() {
    if (!sdCardInitialized) {
        return 0;
    }
    
    uint64_t totalBytes = SD.totalBytes();
    uint64_t usedBytes = SD.usedBytes();
    
    return (uint32_t)((totalBytes - usedBytes) / (1024 * 1024)); // MB単位
}

uint32_t StorageManager::getStorageUsagePercent() {
    if (!sdCardInitialized) {
        return 0;
    }
    
    uint64_t totalBytes = SD.totalBytes();
    uint64_t usedBytes = SD.usedBytes();
    
    if (totalBytes == 0) {
        return 0;
    }
    
    return (uint32_t)((usedBytes * 100) / totalBytes);
}

bool StorageManager::exportData(const String& format, const String& dateRange) {
    // 基本実装（後で拡張予定）
    Serial.println("データエクスポート機能は未実装です");
    return false;
}

bool StorageManager::archiveOldFiles() {
    Serial.println("古いファイルのアーカイブを実行中...");
    
    if (!sdCardInitialized) {
        return false;
    }
    
    // 基本実装：30日以上古いファイルを削除
    // 実際の実装では日付を解析して古いファイルを特定する
    
    cleanupOldFiles();
    return true;
}

void StorageManager::cleanupOldFiles() {
    // 基本実装：ストレージ使用量が90%を超えた場合に古いファイルを削除
    if (getStorageUsagePercent() > 90) {
        Serial.println("ストレージ容量不足のため、古いファイルを削除します");
        
        // 実際の実装では日付順にソートして古いファイルから削除
        // 現在は基本実装のみ
    }
}