#ifndef DISPLAY_CONTROLLER_H
#define DISPLAY_CONTROLLER_H

#include "SystemTypes.h"
#include <M5Unified.h>

class DisplayController {
private:
    DisplayPage currentPage;
    unsigned long lastPageChange;
    bool touchPressed;
    SensorReading lastSensorData;
    SystemStatus lastSystemStatus;
    
    // 表示ヘルパーメソッド
    void clearScreen();
    void lcdPrint(int y, const String& msg, uint32_t color = GREEN);
    void lcdPrint(int x, int y, const String& msg, uint32_t color = GREEN);
    void drawPageHeader(const String& title);
    void drawPageFooter();
    void drawStatusIndicators();
    
    // ページレンダリングメソッド
    void renderSensorDataPage1();
    void renderSensorDataPage2();
    void renderStatusPage();
    void renderConfigPage();
    
    // タッチハンドリング
    bool handleTouchInput();
    bool isInButtonArea(int x, int y);

public:
    DisplayController();
    ~DisplayController();
    
    // コアインターフェースメソッド
    void initialize();
    void showSensorData(const SensorReading& data, DisplayPage page);
    void showStatus(const SystemStatus& status);
    void showConfigMenu();
    void handleTouch();
    DisplayPage getCurrentPage() const { return currentPage; }
    void setCurrentPage(DisplayPage page);
    
    // メインループで呼び出す更新メソッド
    void update();
    
    // 表示制御
    void setBrightness(uint8_t brightness);
    void showMessage(const String& message, uint32_t color = WHITE, uint32_t duration = 2000);
    void showError(const String& error);
    void showWarning(const String& warning);
    
    // 定数
    static const int SCREEN_WIDTH = 320;
    static const int SCREEN_HEIGHT = 240;
    static const int LINE_HEIGHT = 24;
    static const uint32_t TOUCH_DEBOUNCE_MS = 1000;
};

#endif // DISPLAY_CONTROLLER_H