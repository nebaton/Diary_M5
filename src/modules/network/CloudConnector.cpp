#include "CloudConnector.h"
#include "ErrorHandler.h"
#include "TimeUtils.h"

CloudConnector::CloudConnector() :
    connectionStatus(ConnectionStatus::DISCONNECTED),
    recoveryMode(RecoveryMode::MEMORY_QUEUE),
    lastConnectionCheck(0),
    lastUploadAttempt(0),
    retryCount(0) {
}

CloudConnector::~CloudConnector() {
}

bool CloudConnector::initializeWiFi() {
    Serial.println("WiFiを初期化中...");
    
    // WiFiモードを設定
    WiFi.mode(WIFI_STA);
    
    connectionStatus = ConnectionStatus::DISCONNECTED;
    
    // 基本実装：WiFiの初期化のみ行い、実際の接続は後で行う
    Serial.println("WiFi初期化完了（接続は設定後に行います）");
    return true;
}

bool CloudConnector::uploadToGoogleSheets(const SensorReading& data) {
    if (connectionStatus != ConnectionStatus::CONNECTED) {
        // 接続されていない場合はキューに追加
        addToQueue(data);
        return false;
    }
    
    // 基本実装：実際のアップロード機能は後で実装
    Serial.println("Google Sheetsへのアップロード機能は未実装です");
    
    // デバッグ用：データをシリアルに出力
    Serial.println("アップロードデータ:");
    Serial.println("  温度: " + String(data.temperature) + "℃");
    Serial.println("  湿度: " + String(data.humidity) + "%");
    Serial.println("  気圧: " + String(data.pressure) + "hPa");
    Serial.println("  CO2: " + String(data.co2_equivalent) + "ppm");
    Serial.println("  IAQ: " + String(data.iaq));
    
    return true; // 基本実装では常に成功とする
}

bool CloudConnector::uploadToCloudDatabase(const SensorReading& data) {
    // 基本実装（後で拡張予定）
    Serial.println("クラウドデータベースへのアップロード機能は未実装です");
    return false;
}

bool CloudConnector::syncOfflineData(const std::vector<String>& files) {
    if (connectionStatus != ConnectionStatus::CONNECTED) {
        return false;
    }
    
    Serial.println("オフラインデータの同期を開始します（" + String(files.size()) + "ファイル）");
    
    // 基本実装（後で拡張予定）
    for (const String& file : files) {
        Serial.println("同期予定ファイル: " + file);
    }
    
    return true;
}

String CloudConnector::generateOmenReport(const std::vector<SensorReading>& data) {
    // 基本実装：簡単なレポートを生成
    String report = "=== 予感AIちゃん レポート ===\n";
    report += "データ数: " + String(data.size()) + "\n";
    
    if (!data.empty()) {
        const auto& latest = data.back();
        report += "最新データ:\n";
        report += "  温度: " + String(latest.temperature, 1) + "℃\n";
        report += "  湿度: " + String(latest.humidity, 1) + "%\n";
        report += "  空気質: " + String(latest.iaq, 0) + "\n";
        
        // 簡単な分析
        if (latest.iaq < 50) {
            report += "空気質: 良好です！\n";
        } else if (latest.iaq < 100) {
            report += "空気質: 普通です\n";
        } else {
            report += "空気質: 注意が必要です\n";
        }
    }
    
    report += "========================\n";
    return report;
}

void CloudConnector::addToUploadQueue(const SensorReading& data) {
    addToQueue(data);
}

void CloudConnector::addToQueue(const SensorReading& data) {
    if (uploadQueue.size() >= MAX_QUEUE_SIZE) {
        // キューが満杯の場合は古いデータを削除
        uploadQueue.pop();
        ErrorHandler::logWarning(ErrorComponent::NETWORK, "QUEUE_FULL", 
                                "アップロードキューが満杯です。古いデータを削除しました");
    }
    
    uploadQueue.push(data);
}

bool CloudConnector::processUploadQueue() {
    if (connectionStatus != ConnectionStatus::CONNECTED || uploadQueue.empty()) {
        return false;
    }
    
    Serial.println("アップロードキューを処理中（" + String(uploadQueue.size()) + "件）");
    
    int processedCount = 0;
    int maxProcessPerCycle = 10; // 一度に処理する最大数
    
    while (!uploadQueue.empty() && processedCount < maxProcessPerCycle) {
        SensorReading data = uploadQueue.front();
        uploadQueue.pop();
        
        if (uploadToGoogleSheets(data)) {
            processedCount++;
        } else {
            // アップロードに失敗した場合は再度キューに追加
            uploadQueue.push(data);
            break;
        }
    }
    
    Serial.println("キュー処理完了: " + String(processedCount) + "件処理");
    return processedCount > 0;
}

void CloudConnector::update() {
    // 定期的な接続状態チェック
    if (millis() - lastConnectionCheck > CONNECTION_CHECK_INTERVAL) {
        checkConnectionStatus();
        lastConnectionCheck = millis();
    }
    
    // キューの処理
    if (connectionStatus == ConnectionStatus::CONNECTED && !uploadQueue.empty()) {
        processUploadQueue();
    }
}

void CloudConnector::checkConnectionStatus() {
    ConnectionStatus oldStatus = connectionStatus;
    
    if (WiFi.status() == WL_CONNECTED) {
        if (connectionStatus != ConnectionStatus::CONNECTED) {
            connectionStatus = ConnectionStatus::CONNECTED;
            Serial.println("WiFi接続が確立されました");
            
            // 時刻同期を試行
            TimeUtils::syncTimeWithNTP();
        }
    } else {
        if (connectionStatus == ConnectionStatus::CONNECTED) {
            connectionStatus = ConnectionStatus::DISCONNECTED;
            Serial.println("WiFi接続が切断されました");
        }
    }
    
    // 状態変化をログに記録
    if (oldStatus != connectionStatus) {
        String statusStr = "";
        switch (connectionStatus) {
            case ConnectionStatus::CONNECTED: statusStr = "接続"; break;
            case ConnectionStatus::CONNECTING: statusStr = "接続中"; break;
            case ConnectionStatus::DISCONNECTED: statusStr = "切断"; break;
            case ConnectionStatus::ERROR: statusStr = "エラー"; break;
        }
        ErrorHandler::logInfo(ErrorComponent::NETWORK, "接続状態変化: " + statusStr);
    }
}

bool CloudConnector::connectToWiFi() {
    // 基本実装：実際の接続処理は後で実装
    Serial.println("WiFi接続機能は未実装です（WiFiManagerを使用予定）");
    return false;
}

int CloudConnector::getWiFiStrength() {
    if (WiFi.status() == WL_CONNECTED) {
        return WiFi.RSSI();
    }
    return -100; // 接続されていない場合
}