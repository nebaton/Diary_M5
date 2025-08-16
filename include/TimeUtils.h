#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <Arduino.h>
#include <time.h>

class TimeUtils {
public:
    // 時刻フォーマット
    static String getCurrentTimestamp();
    static String formatTimestamp(uint32_t timestamp);
    static String getCurrentDateString();
    static String getCurrentTimeString();
    
    // ファイル名ヘルパー
    static String generateDailyFileName(const String& prefix, const String& extension);
    static String generateTimestampedFileName(const String& prefix, const String& extension);
    
    // 時刻計算
    static uint32_t getCurrentUnixTime();
    static bool isNewDay(uint32_t lastTimestamp);
    static uint32_t getSecondsSinceMidnight();
    
    // NTP同期
    static bool syncTimeWithNTP();
    static bool isTimeSynced();
    
private:
    static bool timeSynced;
    static const char* NTP_SERVER;
    static const long GMT_OFFSET_SEC;
    static const int DAYLIGHT_OFFSET_SEC;
};

#endif // TIME_UTILS_H