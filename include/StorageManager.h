#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include "SystemTypes.h"
#include <vector>
#include <SD.h>

class StorageManager {
private:
    StorageMode currentMode;
    bool sdCardInitialized;
    String currentLogFile;
    uint32_t maxStorageSize;
    
    String generateDailyFileName();
    bool ensureDirectoryExists(const String& path);
    void cleanupOldFiles();

public:
    StorageManager();
    ~StorageManager();
    
    // コアインターフェースメソッド
    bool initializeSDCard();
    bool saveToSDCard(const SensorReading& data);
    bool createDailyLogFile();
    std::vector<String> getUnsyncedFiles();
    bool markFileAsSynced(const String& filename);
    StorageMode getCurrentMode() const { return currentMode; }
    void setStorageMode(StorageMode mode);
    uint32_t getAvailableSpace();
    
    // ステータスメソッド
    bool isSDCardReady() const { return sdCardInitialized; }
    uint32_t getStorageUsagePercent();
    
    // ファイル管理
    bool exportData(const String& format, const String& dateRange);
    bool archiveOldFiles();
    
    // 定数
    static const uint32_t MAX_STORAGE_MB = 8000; // センサーデータ用8GB
    static const uint32_t WARNING_THRESHOLD_PERCENT = 85;
};

#endif // STORAGE_MANAGER_H