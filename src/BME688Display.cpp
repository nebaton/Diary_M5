#include "BME688Display.h"
#include <Wire.h>

// 静的メンバーの初期化
BME688Display* BME688Display::instance = nullptr;

BME688Display::BME688Display() :
    initialized(false),
    currentPage(0),
    lastPageChange(0),
    touchPressed(false) {
    instance = this;
}

bool BME688Display::initialize() {
    Serial.println("BME688ディスプレイを初期化中...");
    
    // M5Stackディスプレイの設定
    M5.Display.setRotation(1);
    M5.Display.setTextSize(2);
    clearScreen();
    M5.Display.setFont(&fonts::lgfxJapanGothic_12);
    
    // I2C初期化
    Wire.begin();
    delay(1000);
    
    // BME688センサーの初期化
    if (!envSensor.begin(BME68X_I2C_ADDR_HIGH, Wire)) {
        Serial.println("高アドレスで失敗、低アドレスを試行中...");
        if (!envSensor.begin(BME68X_I2C_ADDR_LOW, Wire)) {
            Serial.println("BME688の初期化に失敗しました");
            lcdPrint(50, "BME688初期化失敗", RED);
            return false;
        }
    }
    
    // BSEC出力の設定
    bsecSensor sensorList[] = {
        BSEC_OUTPUT_IAQ,
        BSEC_OUTPUT_CO2_EQUIVALENT,
        BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
        BSEC_OUTPUT_RAW_TEMPERATURE,
        BSEC_OUTPUT_RAW_PRESSURE,
        BSEC_OUTPUT_RAW_HUMIDITY,
        BSEC_OUTPUT_RAW_GAS,
        BSEC_OUTPUT_STABILIZATION_STATUS,
        BSEC_OUTPUT_RUN_IN_STATUS
    };
    
    // センサー出力の購読
    if (!envSensor.updateSubscription(sensorList, ARRAY_LEN(sensorList), BSEC_SAMPLE_RATE_ULP)) {
        Serial.println("BSEC購読に失敗しました");
        lcdPrint(50, "BSEC購読失敗", RED);
        return false;
    }
    
    // コールバック設定
    envSensor.attachCallback(bsecCallback);
    
    initialized = true;
    lcdPrint(50, "BME688初期化完了!", GREEN);
    delay(2000);
    
    Serial.println("BME688ディスプレイの初期化が完了しました");
    return true;
}

void BME688Display::bsecCallback(const bme68xData data, const bsecOutputs outputs, Bsec2 bsec) {
    if (instance) {
        instance->processBsecData(outputs);
    }
}

void BME688Display::processBsecData(const bsecOutputs& outputs) {
    if (!outputs.nOutputs) return;
    
    // センサーデータを更新
    for (uint8_t i = 0; i < outputs.nOutputs; i++) {
        const bsecData output = outputs.output[i];
        
        switch (output.sensor_id) {
            case BSEC_OUTPUT_RAW_TEMPERATURE:
                sensorData.temperature = output.signal;
                break;
            case BSEC_OUTPUT_RAW_HUMIDITY:
                sensorData.humidity = output.signal;
                break;
            case BSEC_OUTPUT_RAW_PRESSURE:
                sensorData.pressure = output.signal / 100.0f; // hPaに変換
                break;
            case BSEC_OUTPUT_CO2_EQUIVALENT:
                sensorData.co2 = output.signal;
                break;
            case BSEC_OUTPUT_IAQ:
                sensorData.iaq = output.signal;
                break;
            case BSEC_OUTPUT_BREATH_VOC_EQUIVALENT:
                sensorData.voc = output.signal;
                break;
            case BSEC_OUTPUT_STABILIZATION_STATUS:
                sensorData.stabilized = (output.signal == 1);
                break;
            case BSEC_OUTPUT_RUN_IN_STATUS:
                sensorData.runin = output.signal;
                break;
        }
    }
    
    sensorData.dataValid = true;
    
    // シリアル出力
    Serial.println("=== センサーデータ更新 ===");
    Serial.println("温度: " + String(sensorData.temperature, 1) + "℃");
    Serial.println("湿度: " + String(sensorData.humidity, 1) + "%");
    Serial.println("気圧: " + String(sensorData.pressure, 0) + "hPa");
    Serial.println("CO2: " + String(sensorData.co2, 0) + "ppm");
    Serial.println("IAQ: " + String(sensorData.iaq, 0));
    Serial.println("安定状態: " + String(sensorData.stabilized ? "安定" : "調整中"));
    
    // 画面更新
    displayCurrentPage();
}

void BME688Display::clearScreen() {
    M5.Display.fillScreen(BLACK);
    M5.Display.setFont(&fonts::lgfxJapanGothic_12);
}

void BME688Display::lcdPrint(int y, const String& msg, uint32_t color) {
    M5.Display.setCursor(5, y);
    M5.Display.setTextColor(color, BLACK);
    M5.Display.print(msg);
}

void BME688Display::lcdPrint(int x, int y, const String& msg, uint32_t color) {
    M5.Display.setCursor(x, y);
    M5.Display.setTextColor(color, BLACK);
    M5.Display.print(msg);
}

void BME688Display::displayCurrentPage() {
    if (!sensorData.dataValid) return;
    
    clearScreen();
    
    int y = 15;
    int lineHeight = 24;
    
    if (currentPage == 0) {
        // ページ1: 基本環境データ
        lcdPrint(y, "環境データ (1/2)", CYAN);
        y += lineHeight + 4;
        
        lcdPrint(y, "温度: " + String(sensorData.temperature, 1) + "℃", WHITE);
        y += lineHeight;
        lcdPrint(y, "湿度: " + String(sensorData.humidity, 1) + "%", WHITE);
        y += lineHeight;
        lcdPrint(y, "気圧: " + String(sensorData.pressure, 0) + "hPa", WHITE);
        y += lineHeight;
        lcdPrint(y, "CO2: " + String(sensorData.co2, 0) + "ppm", WHITE);
    } else {
        // ページ2: 空気質データ
        lcdPrint(y, "空気質データ (2/2)", CYAN);
        y += lineHeight + 4;
        
        lcdPrint(y, "空気質: " + String(sensorData.iaq, 0), WHITE);
        y += lineHeight;
        lcdPrint(y, "VOC: " + String(sensorData.voc, 1) + "ppm", WHITE);
        y += lineHeight;
        lcdPrint(y, "状態: " + String(sensorData.stabilized ? "安定" : "調整中"), WHITE);
        y += lineHeight;
        lcdPrint(y, "慣らし: " + String(sensorData.runin, 0) + "%", WHITE);
        
        if (sensorData.runin < 50) {
            y += lineHeight;
            lcdPrint(y, "※長期慣らし運転中", YELLOW);
        }
    }
    
    // フッター
    y = 240 - 50;
    String buttonText = (currentPage == 0) ? "次へ >" : "< 前へ";
    M5.Display.setCursor(240, y);
    M5.Display.setTextColor(WHITE, DARKGREY);
    M5.Display.print(buttonText);
    
    lcdPrint(10, y, "ページ " + String(currentPage + 1) + "/2", GREEN);
    y += 20;
    lcdPrint(10, y, "更新: " + String(millis()/1000) + "秒", YELLOW);
}

void BME688Display::handleTouch() {
    M5.update();
    
    if (M5.Touch.getCount() > 0) {
        if (!touchPressed) {
            auto touch = M5.Touch.getDetail();
            
            // 画面右半分のタッチでページ切り替え
            if (touch.x > 160 && millis() - lastPageChange > 1000) {
                currentPage = (currentPage == 0) ? 1 : 0;
                lastPageChange = millis();
                touchPressed = true;
                
                displayCurrentPage();
                Serial.println("ページを切り替えました: " + String(currentPage + 1));
            }
        }
    } else {
        touchPressed = false;
    }
}

void BME688Display::update() {
    if (!initialized) return;
    
    // BSECライブラリの更新
    if (!envSensor.run()) {
        // エラーハンドリング
        if (envSensor.status < BSEC_OK) {
            Serial.println("BSECエラー: " + String(envSensor.status));
        }
        if (envSensor.sensor.status < BME68X_OK) {
            Serial.println("BME688エラー: " + String(envSensor.sensor.status));
        }
    }
    
    // タッチ処理
    handleTouch();
}