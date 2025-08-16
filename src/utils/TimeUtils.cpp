#include "TimeUtils.h"
#include <WiFi.h>

// 静的メンバーの初期化
bool TimeUtils::timeSynced = false;
const char* TimeUtils::NTP_SERVER = "pool.ntp.org";
const long TimeUtils::GMT_OFFSET_SEC = 9 * 3600; // JST (UTC+9)
const int TimeUtils::DAYLIGHT_OFFSET_SEC = 0;

String TimeUtils::getCurrentTimestamp() {
    if (!timeSynced) {
        // NTPが同期されていない場合はmillis()を使用
        return String(millis());
    }
    
    time_t now;
    time(&now);
    return String(now);
}

String TimeUtils::formatTimestamp(uint32_t timestamp) {
    if (!timeSynced) {
        // NTPが同期されていない場合は経過時間として表示
        uint32_t seconds = timestamp / 1000;
        uint32_t minutes = seconds / 60;
        uint32_t hours = minutes / 60;
        
        return String(hours) + ":" + String(minutes % 60) + ":" + String(seconds % 60);
    }
    
    time_t rawtime = timestamp;
    struct tm* timeinfo = localtime(&rawtime);
    
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    return String(buffer);
}

String TimeUtils::getCurrentDateString() {
    if (!timeSynced) {
        // NTPが同期されていない場合は起動からの日数
        uint32_t days = millis() / (24 * 60 * 60 * 1000);
        return "Day" + String(days);
    }
    
    time_t now;
    time(&now);
    struct tm* timeinfo = localtime(&now);
    
    char buffer[12];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", timeinfo);
    return String(buffer);
}

String TimeUtils::getCurrentTimeString() {
    if (!timeSynced) {
        // NTPが同期されていない場合は経過時間
        uint32_t totalSeconds = millis() / 1000;
        uint32_t hours = totalSeconds / 3600;
        uint32_t minutes = (totalSeconds % 3600) / 60;
        uint32_t seconds = totalSeconds % 60;
        
        return String(hours) + ":" + String(minutes) + ":" + String(seconds);
    }
    
    time_t now;
    time(&now);
    struct tm* timeinfo = localtime(&now);
    
    char buffer[10];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", timeinfo);
    return String(buffer);
}

String TimeUtils::generateDailyFileName(const String& prefix, const String& extension) {
    String dateStr = getCurrentDateString();
    return prefix + "_" + dateStr + "." + extension;
}

String TimeUtils::generateTimestampedFileName(const String& prefix, const String& extension) {
    String timestamp = getCurrentTimestamp();
    return prefix + "_" + timestamp + "." + extension;
}

uint32_t TimeUtils::getCurrentUnixTime() {
    if (!timeSynced) {
        // NTPが同期されていない場合はmillis()を返す
        return millis();
    }
    
    time_t now;
    time(&now);
    return (uint32_t)now;
}

bool TimeUtils::isNewDay(uint32_t lastTimestamp) {
    if (!timeSynced) {
        // NTPが同期されていない場合は24時間経過で新しい日とする
        return (millis() - lastTimestamp) > (24 * 60 * 60 * 1000);
    }
    
    time_t now;
    time(&now);
    struct tm* nowInfo = localtime(&now);
    
    time_t lastTime = lastTimestamp;
    struct tm* lastInfo = localtime(&lastTime);
    
    return (nowInfo->tm_yday != lastInfo->tm_yday) || (nowInfo->tm_year != lastInfo->tm_year);
}

uint32_t TimeUtils::getSecondsSinceMidnight() {
    if (!timeSynced) {
        // NTPが同期されていない場合は起動からの秒数を24時間で割った余り
        return (millis() / 1000) % (24 * 60 * 60);
    }
    
    time_t now;
    time(&now);
    struct tm* timeinfo = localtime(&now);
    
    return timeinfo->tm_hour * 3600 + timeinfo->tm_min * 60 + timeinfo->tm_sec;
}

bool TimeUtils::syncTimeWithNTP() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFiが接続されていないため、NTP同期をスキップします");
        return false;
    }
    
    Serial.println("NTPサーバーと時刻を同期中...");
    
    // NTPクライアントの設定
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
    
    // 同期を待つ（最大10秒）
    int attempts = 0;
    while (attempts < 10) {
        time_t now = time(nullptr);
        if (now > 1000000000) { // 有効な時刻が取得できた場合
            timeSynced = true;
            Serial.println("NTP同期が完了しました");
            
            struct tm* timeinfo = localtime(&now);
            char buffer[30];
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
            Serial.println("現在時刻: " + String(buffer));
            
            return true;
        }
        delay(1000);
        attempts++;
    }
    
    Serial.println("NTP同期に失敗しました");
    return false;
}

bool TimeUtils::isTimeSynced() {
    return timeSynced;
}