#ifndef CLOUD_CONNECTOR_H
#define CLOUD_CONNECTOR_H

#include "SystemTypes.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <vector>
#include <queue>

class CloudConnector {
private:
    ConnectionStatus connectionStatus;
    RecoveryMode recoveryMode;
    std::queue<SensorReading> uploadQueue;
    unsigned long lastConnectionCheck;
    unsigned long lastUploadAttempt;
    int retryCount;
    
    // WiFi管理
    bool connectToWiFi();
    void checkConnectionStatus();
    
    // アップロードメソッド
    bool uploadSingleReading(const SensorReading& data);
    bool processRetryLogic();
    
    // キュー管理
    void addToQueue(const SensorReading& data);
    bool processQueue();
    
    static const uint32_t MAX_QUEUE_SIZE = 1000;
    static const uint32_t CONNECTION_CHECK_INTERVAL = 30000; // 30秒
    static const uint32_t MAX_RETRY_ATTEMPTS = 3;

public:
    CloudConnector();
    ~CloudConnector();
    
    // コアインターフェースメソッド
    bool initializeWiFi();
    ConnectionStatus getConnectionStatus() const { return connectionStatus; }
    bool uploadToGoogleSheets(const SensorReading& data);
    bool uploadToCloudDatabase(const SensorReading& data);
    bool syncOfflineData(const std::vector<String>& files);
    String generateOmenReport(const std::vector<SensorReading>& data);
    
    // ネットワーク復旧メソッド
    void addToUploadQueue(const SensorReading& data);
    
    // Queue management (moved to public)
    bool processUploadQueue();
    
    void setRecoveryMode(RecoveryMode mode) { recoveryMode = mode; }
    uint32_t getQueueSize() const { return uploadQueue.size(); }
    bool isQueueFull() const { return uploadQueue.size() >= MAX_QUEUE_SIZE; }
    
    // メインループで呼び出す更新メソッド
    void update();
    
    // ステータスメソッド
    bool isConnected() const { return connectionStatus == ConnectionStatus::CONNECTED; }
    int getWiFiStrength();
};

#endif // CLOUD_CONNECTOR_H