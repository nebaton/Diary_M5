#ifndef YOKAN_AI_SYSTEM_H
#define YOKAN_AI_SYSTEM_H

#include "SystemTypes.h"
#include "ConfigManager.h"
#include "SensorDataCollector.h"
#include "StorageManager.h"
#include "CloudConnector.h"
#include "DisplayController.h"
#include "ErrorHandler.h"
#include "TimeUtils.h"

class YokanAISystem {
private:
    // コアモジュール
    ConfigManager configManager;
    SensorDataCollector sensorCollector;
    StorageManager storageManager;
    CloudConnector cloudConnector;
    DisplayController displayController;
    
    // システム状態
    SystemStatus systemStatus;
    bool systemInitialized;
    unsigned long lastStatusUpdate;
    unsigned long systemStartTime;
    
    // コールバック
    void onSensorDataReceived(const SensorReading& data);
    void onSystemStatusChanged(const SystemStatus& status);
    
    // システム管理
    void updateSystemStatus();
    void handleSystemErrors();
    void performPeriodicMaintenance();
    
    // モード管理
    void switchToOnlineMode();
    void switchToOfflineMode();
    void handleNetworkRecovery();

public:
    YokanAISystem();
    ~YokanAISystem();
    
    // システムライフサイクル
    bool initialize();
    void update();
    void shutdown();
    
    // ステータスとコントロール
    bool isInitialized() const { return systemInitialized; }
    SystemStatus getSystemStatus() const { return systemStatus; }
    
    // モジュールアクセス（高度な制御用）
    SensorDataCollector& getSensorCollector() { return sensorCollector; }
    StorageManager& getStorageManager() { return storageManager; }
    CloudConnector& getCloudConnector() { return cloudConnector; }
    DisplayController& getDisplayController() { return displayController; }
    ConfigManager& getConfigManager() { return configManager; }
    
    // 定数
    static const uint32_t STATUS_UPDATE_INTERVAL = 5000; // 5秒
    static const uint32_t MAINTENANCE_INTERVAL = 300000; // 5分
};

#endif // YOKAN_AI_SYSTEM_H