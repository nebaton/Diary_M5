#ifndef SENSOR_DATA_COLLECTOR_H
#define SENSOR_DATA_COLLECTOR_H

#include "SystemTypes.h"
#include <bsec2.h>

class SensorDataCollector {
private:
    Bsec2 envSensor;
    SensorCallback dataCallback;
    SensorReading currentReading;
    bool initialized;
    unsigned long lastReadingTime;
    
    // BSEC2用静的コールバック
    static void bsecCallback(const bme68xData data, const bsecOutputs outputs, Bsec2 bsec);
    static SensorDataCollector* instance;
    
    void processBsecData(const bme68xData data, const bsecOutputs outputs);
    void updateCurrentReading(const bsecOutputs& outputs);

public:
    SensorDataCollector();
    ~SensorDataCollector();
    
    // コアインターフェースメソッド（要件1.1, 1.2対応）
    bool initialize();                          // センサー初期化
    SensorReading getCurrentReading();          // 現在のセンサーデータ取得
    bool isDataValid();                        // データ有効性チェック
    void setCallback(SensorCallback callback);  // データ更新コールバック設定
    
    // ステータスとコントロールメソッド
    bool isInitialized() const { return initialized; }
    bool isStabilized() const { return currentReading.stabilized; }
    float getRunInStatus() const { return currentReading.runin_status; }
    
    // センサー情報取得メソッド
    float getTemperature() const { return currentReading.temperature; }
    float getHumidity() const { return currentReading.humidity; }
    float getPressure() const { return currentReading.pressure; }
    float getCO2Equivalent() const { return currentReading.co2_equivalent; }
    float getIAQ() const { return currentReading.iaq; }
    float getVOCEquivalent() const { return currentReading.voc_equivalent; }
    uint32_t getLastReadingTime() const { return lastReadingTime; }
    
    // サンプリング間隔制御メソッド
    bool setSamplingMode(float sampleRate);  // サンプリング間隔変更
    bool upgradeToFullMode();                // フル機能モードに変更
    
    // メインループで呼び出す更新メソッド
    void update();
    
    // エラーチェック
    void checkBsecStatus();
};

#endif // SENSOR_DATA_COLLECTOR_H