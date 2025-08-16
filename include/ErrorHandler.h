#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <Arduino.h>
#include <vector>

enum class ErrorLevel {
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

enum class ErrorComponent {
    SENSOR,
    STORAGE,
    NETWORK,
    DISPLAY_MODULE,  // DISPLAYはArduino.hで定義されているため変更
    SYSTEM
};

struct ErrorEntry {
    uint32_t timestamp;
    ErrorLevel level;
    ErrorComponent component;
    String errorCode;
    String message;
    String context;
    
    ErrorEntry(ErrorLevel lvl, ErrorComponent comp, const String& code, 
               const String& msg, const String& ctx = "") :
        timestamp(millis()), level(lvl), component(comp), 
        errorCode(code), message(msg), context(ctx) {}
};

class ErrorHandler {
private:
    static std::vector<ErrorEntry> errorLog;
    static const uint32_t MAX_LOG_ENTRIES = 100;
    
    static String levelToString(ErrorLevel level);
    static String componentToString(ErrorComponent component);
    
public:
    // ログメソッド
    static void logError(ErrorComponent component, const String& errorCode, 
                        const String& message, const String& context = "");
    static void logWarning(ErrorComponent component, const String& errorCode, 
                          const String& message, const String& context = "");
    static void logInfo(ErrorComponent component, const String& message);
    
    // 取得メソッド
    static std::vector<ErrorEntry> getRecentErrors(uint32_t count = 10);
    static std::vector<ErrorEntry> getErrorsByComponent(ErrorComponent component);
    static std::vector<ErrorEntry> getErrorsByLevel(ErrorLevel level);
    
    // 管理メソッド
    static void clearLog();
    static bool saveLogToFile();
    static bool loadLogFromFile();
    static uint32_t getErrorCount();
    
    // エラーコード
    static const char* ERROR_SENSOR_INIT_FAILED;
    static const char* ERROR_SENSOR_READ_FAILED;
    static const char* ERROR_STORAGE_INIT_FAILED;
    static const char* ERROR_STORAGE_WRITE_FAILED;
    static const char* ERROR_NETWORK_CONNECT_FAILED;
    static const char* ERROR_NETWORK_UPLOAD_FAILED;
    static const char* ERROR_DISPLAY_MODULE_INIT_FAILED;
    static const char* ERROR_CONFIG_LOAD_FAILED;
    static const char* ERROR_CONFIG_SAVE_FAILED;
};

#endif // ERROR_HANDLER_H