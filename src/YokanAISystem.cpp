#include "YokanAISystem.h"

YokanAISystem::YokanAISystem() :
    systemInitialized(false),
    lastStatusUpdate(0),
    systemStartTime(0) {
}

YokanAISystem::~YokanAISystem() {
    shutdown();
}

bool YokanAISystem::initialize() {
    Serial.println("Initializing Yokan AI System...");
    systemStartTime = millis();
    
    // Initialize error handler
    ErrorHandler::clearLog();
    
    // Initialize configuration manager
    if (!configManager.loadConfig()) {
        ErrorHandler::logWarning(ErrorComponent::SYSTEM, "CONFIG_LOAD", 
                                "Using default configuration");
        configManager.resetToDefaults();
    }
    
    // Initialize display controller first (for user feedback)
    displayController.initialize();
    displayController.showMessage("システム初期化中...", WHITE, 1000);
    
    // Initialize sensor data collector
    if (!sensorCollector.initialize()) {
        ErrorHandler::logError(ErrorComponent::SENSOR, "INIT_FAILED", 
                              "Failed to initialize BME688 sensor");
        displayController.showError("センサー初期化失敗");
        return false;
    }
    
    // センサーの安定化を待つ
    displayController.showMessage("センサー安定化中...", YELLOW, 3000);
    delay(3000); // センサーの安定化時間
    
    // Set up sensor callback
    sensorCollector.setCallback([this](const SensorReading& data) {
        this->onSensorDataReceived(data);
    });
    
    // Initialize storage manager
    if (!storageManager.initializeSDCard()) {
        ErrorHandler::logWarning(ErrorComponent::STORAGE, "SD_INIT_FAILED", 
                                "SD card not available, using memory only");
        displayController.showWarning("SDカード未検出");
    }
    
    // Initialize network connector
    if (!cloudConnector.initializeWiFi()) {
        ErrorHandler::logWarning(ErrorComponent::NETWORK, "WIFI_INIT_FAILED", 
                                "WiFi initialization failed, using offline mode");
        displayController.showWarning("WiFi接続失敗");
    }
    
    // Update initial system status
    updateSystemStatus();
    
    systemInitialized = true;
    displayController.showMessage("初期化完了!", GREEN, 2000);
    
    Serial.println("System initialization completed");
    return true;
}

void YokanAISystem::update() {
    if (!systemInitialized) return;
    
    // Update all modules
    sensorCollector.update();
    cloudConnector.update();
    displayController.update();
    
    // Update system status periodically
    if (millis() - lastStatusUpdate > STATUS_UPDATE_INTERVAL) {
        updateSystemStatus();
        lastStatusUpdate = millis();
    }
    
    // Handle any system errors
    handleSystemErrors();
    
    // Perform periodic maintenance
    static unsigned long lastMaintenance = 0;
    if (millis() - lastMaintenance > MAINTENANCE_INTERVAL) {
        performPeriodicMaintenance();
        lastMaintenance = millis();
    }
}

void YokanAISystem::shutdown() {
    if (!systemInitialized) return;
    
    Serial.println("Shutting down Yokan AI System...");
    
    // Save current configuration
    configManager.saveConfig(configManager.getCurrentConfig());
    
    // Save error log
    ErrorHandler::saveLogToFile();
    
    systemInitialized = false;
    Serial.println("System shutdown completed");
}

void YokanAISystem::onSensorDataReceived(const SensorReading& data) {
    // Update display with new sensor data
    displayController.showSensorData(data, displayController.getCurrentPage());
    
    // Try to upload to cloud if connected
    if (cloudConnector.isConnected()) {
        if (!cloudConnector.uploadToGoogleSheets(data)) {
            // If upload fails, add to queue for retry
            cloudConnector.addToUploadQueue(data);
        }
    } else {
        // Store offline if not connected
        if (storageManager.isSDCardReady()) {
            storageManager.saveToSDCard(data);
        } else {
            // Add to memory queue as last resort
            cloudConnector.addToUploadQueue(data);
        }
    }
}

void YokanAISystem::onSystemStatusChanged(const SystemStatus& status) {
    systemStatus = status;
    
    // Update display if on status page
    if (displayController.getCurrentPage() == DisplayPage::STATUS) {
        displayController.showStatus(status);
    }
}

void YokanAISystem::updateSystemStatus() {
    systemStatus.sensor_healthy = sensorCollector.isInitialized();
    systemStatus.connection_status = cloudConnector.getConnectionStatus();
    systemStatus.storage_usage_percent = storageManager.getStorageUsagePercent();
    systemStatus.battery_level = M5.Power.getBatteryLevel();
    systemStatus.uptime_seconds = (millis() - systemStartTime) / 1000;
    systemStatus.free_memory = ESP.getFreeHeap();
    systemStatus.wifi_strength = cloudConnector.getWiFiStrength();
    
    onSystemStatusChanged(systemStatus);
}

void YokanAISystem::handleSystemErrors() {
    auto recentErrors = ErrorHandler::getRecentErrors(1);
    if (!recentErrors.empty()) {
        const auto& lastError = recentErrors[0];
        
        // Handle critical errors
        if (lastError.level == ErrorLevel::CRITICAL) {
            displayController.showError("重大エラー: " + lastError.message);
            Serial.println("CRITICAL ERROR: " + lastError.message);
        }
    }
}

void YokanAISystem::performPeriodicMaintenance() {
    // Check storage usage and cleanup if needed
    if (storageManager.getStorageUsagePercent() > StorageManager::WARNING_THRESHOLD_PERCENT) {
        storageManager.archiveOldFiles();
    }
    
    // Process upload queue if connected
    if (cloudConnector.isConnected() && cloudConnector.getQueueSize() > 0) {
        cloudConnector.processUploadQueue();
    }
    
    // Sync time if connected
    if (cloudConnector.isConnected() && !TimeUtils::isTimeSynced()) {
        TimeUtils::syncTimeWithNTP();
    }
}