#ifndef BME688_DISPLAY_H
#define BME688_DISPLAY_H

#include <M5Unified.h>
#include <bsec2.h>

// シンプルなBME688表示クラス
class BME688Display {
private:
    Bsec2 envSensor;
    bool initialized;
    int currentPage;
    unsigned long lastPageChange;
    bool touchPressed;
    
    // センサーデータ構造体
    struct SensorData {
        float temperature = 0;
        float humidity = 0;
        float pressure = 0;
        float co2 = 0;
        float iaq = 0;
        float voc = 0;
        bool stabilized = false;
        float runin = 0;
        bool dataValid = false;
    } sensorData;
    
    // 静的コールバック用
    static BME688Display* instance;
    static void bsecCallback(const bme68xData data, const bsecOutputs outputs, Bsec2 bsec);
    
    // 表示関数
    void clearScreen();
    void lcdPrint(int y, const String& msg, uint32_t color = GREEN);
    void lcdPrint(int x, int y, const String& msg, uint32_t color = GREEN);
    void displayCurrentPage();
    void processBsecData(const bsecOutputs& outputs);

public:
    BME688Display();
    bool initialize();
    void update();
    void handleTouch();
    bool isInitialized() const { return initialized; }
};

#endif // BME688_DISPLAY_H