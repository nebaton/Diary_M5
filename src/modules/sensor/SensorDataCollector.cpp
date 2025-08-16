#include "SensorDataCollector.h"
#include "ErrorHandler.h"
#include "TimeUtils.h"
#include <Wire.h>

// Static member initialization
SensorDataCollector* SensorDataCollector::instance = nullptr;

SensorDataCollector::SensorDataCollector() :
    initialized(false),
    lastReadingTime(0) {
    instance = this;
    
    // SensorReading構造体の初期化
    currentReading.timestamp = 0;
    currentReading.temperature = 0.0f;
    currentReading.humidity = 0.0f;
    currentReading.pressure = 0.0f;
    currentReading.co2_equivalent = 0.0f;
    currentReading.iaq = 0.0f;
    currentReading.voc_equivalent = 0.0f;
    currentReading.gas_resistance = 0.0f;
    currentReading.stabilized = false;
    currentReading.runin_status = 0.0f;
    currentReading.device_id = "M5Stack_001";
}

SensorDataCollector::~SensorDataCollector() {
    instance = nullptr;
}

bool SensorDataCollector::initialize() {
    Serial.println("BME688センサーを初期化中...");
    
    // I2Cの初期化
    Wire.begin();
    delay(1000); // センサーの安定化を待つ
    
    // BME688の初期化（高アドレスから試行）
    Serial.println("BME688 高アドレス(0x77)で接続を試行中...");
    if (!envSensor.begin(BME68X_I2C_ADDR_HIGH, Wire)) {
        Serial.println("BME688 低アドレス(0x76)で接続を試行中...");
        if (!envSensor.begin(BME68X_I2C_ADDR_LOW, Wire)) {
            ErrorHandler::logError(ErrorComponent::SENSOR, "BME688_INIT_FAILED", 
                                  "BME688センサーの初期化に失敗しました");
            return false;
        }
    }
    
    Serial.println("BME688センサーが検出されました");
    
    // BSECライブラリの状態確認
    checkBsecStatus();
    
    // センサーの安定化を待つ
    delay(2000);
    
    // BSEC出力の設定（要件1.1に対応：温度・湿度・気圧・CO2・IAQ・VOCデータ）
    // 最初からフル機能モードで開始
    bsecSensor fullSensorList[] = {
        BSEC_OUTPUT_IAQ,                    // 室内空気品質指数
        BSEC_OUTPUT_STATIC_IAQ,             // 静的IAQ
        BSEC_OUTPUT_CO2_EQUIVALENT,         // CO2等価値
        BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,  // VOC等価値
        BSEC_OUTPUT_RAW_TEMPERATURE,        // 温度
        BSEC_OUTPUT_RAW_PRESSURE,           // 気圧
        BSEC_OUTPUT_RAW_HUMIDITY,           // 湿度
        BSEC_OUTPUT_RAW_GAS,                // ガス抵抗値
        BSEC_OUTPUT_STABILIZATION_STATUS,   // 安定化状態
        BSEC_OUTPUT_RUN_IN_STATUS           // 慣らし運転状態
    };
    
    // フル機能モードで初期化を試行（LP mode: 3秒間隔）
    Serial.println("フル機能モード（LP: 3秒間隔）で初期化中...");
    if (!envSensor.updateSubscription(fullSensorList, ARRAY_LEN(fullSensorList), BSEC_SAMPLE_RATE_LP)) {
        Serial.println("LP mode失敗、ULP mode（5分間隔）で再試行...");
        
        if (!envSensor.updateSubscription(fullSensorList, ARRAY_LEN(fullSensorList), BSEC_SAMPLE_RATE_ULP)) {
            Serial.println("フル機能モード失敗、基本RAWデータで再試行...");
            
            // 最後の手段として基本RAWデータのみ
            bsecSensor basicSensorList[] = {
                BSEC_OUTPUT_RAW_TEMPERATURE,
                BSEC_OUTPUT_RAW_HUMIDITY,
                BSEC_OUTPUT_RAW_PRESSURE,
                BSEC_OUTPUT_RAW_GAS,
                BSEC_OUTPUT_STABILIZATION_STATUS,
                BSEC_OUTPUT_RUN_IN_STATUS
            };
            
            if (!envSensor.updateSubscription(basicSensorList, ARRAY_LEN(basicSensorList), BSEC_SAMPLE_RATE_ULP)) {
                ErrorHandler::logError(ErrorComponent::SENSOR, "BSEC_SUBSCRIPTION_FAILED", 
                                      "BSECセンサー出力の購読に失敗しました");
                checkBsecStatus(); // エラー詳細を出力
                return false;
            }
            Serial.println("基本RAWデータモードで初期化成功");
        } else {
            Serial.println("フル機能モード（ULP: 5分間隔）で初期化成功");
        }
    } else {
        Serial.println("フル機能モード（LP: 3秒間隔）で初期化成功");
    }
    
    // 購読設定後の状態確認
    checkBsecStatus();
    
    // コールバック関数の設定
    envSensor.attachCallback(bsecCallback);
    
    // デバイスIDの設定
    currentReading.device_id = "M5Stack_001";
    
    initialized = true;
    Serial.println("BME688センサーの初期化が完了しました（ULPモード: 5分間隔）");
    Serial.println("注意: 初期化後にupgradeToFullMode()を呼び出すことで、より頻繁な間隔に変更できます");
    ErrorHandler::logInfo(ErrorComponent::SENSOR, "BME688センサーが正常に初期化されました");
    
    return true;
}

void SensorDataCollector::bsecCallback(const bme68xData data, const bsecOutputs outputs, Bsec2 bsec) {
    if (instance) {
        instance->processBsecData(data, outputs);
    }
}

void SensorDataCollector::processBsecData(const bme68xData data, const bsecOutputs outputs) {
    if (!outputs.nOutputs) {
        Serial.println("BSECから出力データがありません");
        return;
    }
    
    // タイムスタンプの更新（要件1.2に対応：正確な日時タイムスタンプ）
    currentReading.timestamp = TimeUtils::getCurrentUnixTime();
    lastReadingTime = millis();
    
    // BSECデータの処理
    updateCurrentReading(outputs);
    
    // デバッグ情報の出力（取得できたデータのみ表示）
    Serial.println("センサーデータを更新しました (" + String(outputs.nOutputs) + "個の出力):");
    
    // 基本データ（常に表示）
    Serial.println("  温度: " + String(currentReading.temperature, 1) + "℃");
    Serial.println("  湿度: " + String(currentReading.humidity, 1) + "%");
    Serial.println("  気圧: " + String(currentReading.pressure, 1) + "hPa");
    Serial.println("  ガス抵抗: " + String(currentReading.gas_resistance, 0) + "Ω");
    
    // 拡張データ（取得できた場合のみ表示）
    if (currentReading.co2_equivalent > 0) {
        String co2Status = (currentReading.runin_status < 50) ? " (校正中)" : "";
        Serial.println("  ✓ CO2: " + String(currentReading.co2_equivalent, 0) + "ppm" + co2Status);
    } else {
        Serial.println("  ⚠ CO2: データなし");
    }
    
    if (currentReading.iaq > 0) {
        String iaqStatus = (currentReading.runin_status < 50) ? " (校正中)" : "";
        Serial.println("  ✓ IAQ: " + String(currentReading.iaq, 0) + iaqStatus);
    } else {
        Serial.println("  ⚠ IAQ: データなし");
    }
    
    if (currentReading.voc_equivalent > 0) {
        String vocStatus = (currentReading.runin_status < 50) ? " (校正中)" : "";
        Serial.println("  ✓ VOC: " + String(currentReading.voc_equivalent, 1) + "ppm" + vocStatus);
    } else {
        Serial.println("  ⚠ VOC: データなし");
    }
    
    // ステータス情報
    Serial.println("  安定状態: " + String(currentReading.stabilized ? "安定" : "調整中"));
    Serial.println("  慣らし状況: " + String(currentReading.runin_status, 1) + "%");
    
    // 校正状態の説明
    if (currentReading.runin_status < 25) {
        Serial.println("  📝 BME688センサーは初期校正中です（数分～数時間かかります）");
    } else if (currentReading.runin_status < 75) {
        Serial.println("  🔄 センサー校正が進行中です（値が安定するまでお待ちください）");
    } else {
        Serial.println("  ✅ センサー校正がほぼ完了しました");
    }
    
    // ユーザーコールバックの呼び出し
    if (dataCallback) {
        dataCallback(currentReading);
    }
}

void SensorDataCollector::updateCurrentReading(const bsecOutputs& outputs) {
    for (uint8_t i = 0; i < outputs.nOutputs; i++) {
        const bsecData output = outputs.output[i];
        
        switch (output.sensor_id) {
            case BSEC_OUTPUT_RAW_TEMPERATURE:
                currentReading.temperature = output.signal;
                break;
            case BSEC_OUTPUT_RAW_HUMIDITY:
                currentReading.humidity = output.signal;
                break;
            case BSEC_OUTPUT_RAW_PRESSURE:
                currentReading.pressure = output.signal / 100.0f; // Convert to hPa
                break;
            case BSEC_OUTPUT_CO2_EQUIVALENT:
                currentReading.co2_equivalent = output.signal;
                break;
            case BSEC_OUTPUT_IAQ:
                currentReading.iaq = output.signal;
                break;
            case BSEC_OUTPUT_BREATH_VOC_EQUIVALENT:
                currentReading.voc_equivalent = output.signal;
                break;
            case BSEC_OUTPUT_RAW_GAS:
                currentReading.gas_resistance = output.signal;
                break;
            case BSEC_OUTPUT_STABILIZATION_STATUS:
                currentReading.stabilized = (output.signal == 1);
                break;
            case BSEC_OUTPUT_RUN_IN_STATUS:
                currentReading.runin_status = output.signal;
                break;
        }
    }
}

SensorReading SensorDataCollector::getCurrentReading() {
    return currentReading;
}

bool SensorDataCollector::isDataValid() {
    // データが有効かどうかの判定：
    // 1. センサーが初期化されている
    // 2. 最後の読み取りから10分以内
    // 3. タイムスタンプが設定されている
    bool isValid = initialized && 
                   (millis() - lastReadingTime < 600000) && 
                   (currentReading.timestamp > 0);
    
    if (!isValid && initialized) {
        Serial.println("センサーデータが無効です - 最後の読み取りから時間が経過しています");
    }
    
    return isValid;
}

void SensorDataCollector::setCallback(SensorCallback callback) {
    dataCallback = callback;
}

void SensorDataCollector::update() {
    if (!initialized) return;
    
    // Run BSEC processing
    if (!envSensor.run()) {
        checkBsecStatus();
    }
}

bool SensorDataCollector::setSamplingMode(float sampleRate) {
    if (!initialized) {
        Serial.println("エラー: センサーが初期化されていません");
        return false;
    }
    
    // 現在の設定を保持
    bsecSensor sensorList[] = {
        BSEC_OUTPUT_RAW_TEMPERATURE,
        BSEC_OUTPUT_RAW_PRESSURE,
        BSEC_OUTPUT_RAW_HUMIDITY,
        BSEC_OUTPUT_RAW_GAS,
        BSEC_OUTPUT_STABILIZATION_STATUS,
        BSEC_OUTPUT_RUN_IN_STATUS
    };
    
    String modeStr = (sampleRate == BSEC_SAMPLE_RATE_ULP) ? "ULP (5分間隔)" :
                     (sampleRate == BSEC_SAMPLE_RATE_LP) ? "LP (3秒間隔)" : "CONT (1秒間隔)";
    
    Serial.println("サンプリングモードを " + modeStr + " に変更中...");
    
    if (!envSensor.updateSubscription(sensorList, ARRAY_LEN(sensorList), sampleRate)) {
        ErrorHandler::logError(ErrorComponent::SENSOR, "SAMPLING_MODE_CHANGE_FAILED", 
                              "サンプリングモードの変更に失敗しました");
        checkBsecStatus();
        return false;
    }
    
    Serial.println("サンプリングモードの変更が完了しました: " + modeStr);
    return true;
}

bool SensorDataCollector::upgradeToFullMode() {
    if (!initialized) {
        Serial.println("エラー: センサーが初期化されていません");
        return false;
    }
    
    Serial.println("フル機能モード（CO2・IAQ・VOC含む）にアップグレード中...");
    
    // フル機能のセンサーリスト
    bsecSensor fullSensorList[] = {
        BSEC_OUTPUT_IAQ,                    // 室内空気品質指数
        BSEC_OUTPUT_STATIC_IAQ,             // 静的IAQ
        BSEC_OUTPUT_CO2_EQUIVALENT,         // CO2等価値
        BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,  // VOC等価値
        BSEC_OUTPUT_RAW_TEMPERATURE,        // 温度
        BSEC_OUTPUT_RAW_PRESSURE,           // 気圧
        BSEC_OUTPUT_RAW_HUMIDITY,           // 湿度
        BSEC_OUTPUT_RAW_GAS,                // ガス抵抗値
        BSEC_OUTPUT_STABILIZATION_STATUS,   // 安定化状態
        BSEC_OUTPUT_RUN_IN_STATUS           // 慣らし運転状態
    };
    
    // LP mode (3秒間隔) でフル機能を試行
    if (!envSensor.updateSubscription(fullSensorList, ARRAY_LEN(fullSensorList), BSEC_SAMPLE_RATE_LP)) {
        Serial.println("LP mode失敗、ULP mode (5分間隔) でフル機能を試行...");
        
        if (!envSensor.updateSubscription(fullSensorList, ARRAY_LEN(fullSensorList), BSEC_SAMPLE_RATE_ULP)) {
            ErrorHandler::logError(ErrorComponent::SENSOR, "FULL_MODE_UPGRADE_FAILED", 
                                  "フル機能モードへのアップグレードに失敗しました");
            checkBsecStatus();
            return false;
        }
        Serial.println("フル機能モード（ULP: 5分間隔）でアップグレード完了");
    } else {
        Serial.println("フル機能モード（LP: 3秒間隔）でアップグレード完了");
    }
    
    return true;
}

void SensorDataCollector::checkBsecStatus() {
    // BSECライブラリのステータスチェック
    if (envSensor.status < BSEC_OK) {
        String errorMsg = "BSECエラー: " + String(envSensor.status);
        ErrorHandler::logError(ErrorComponent::SENSOR, "BSEC_ERROR", errorMsg);
        Serial.println(errorMsg);
    } else if (envSensor.status > BSEC_OK) {
        String warningMsg = "BSEC警告: " + String(envSensor.status);
        ErrorHandler::logWarning(ErrorComponent::SENSOR, "BSEC_WARNING", warningMsg);
        
        // 特定の警告の詳細説明
        if (envSensor.status == 14) {
            Serial.println("BSEC警告14: タイミング違反が検出されました - ループタイミングを調整してください");
        }
    }
    
    // BME688センサーのステータスチェック
    if (envSensor.sensor.status < BME68X_OK) {
        String errorMsg = "BME688エラー: " + String(envSensor.sensor.status);
        ErrorHandler::logError(ErrorComponent::SENSOR, "BME688_ERROR", errorMsg);
        Serial.println(errorMsg);
    } else if (envSensor.sensor.status > BME68X_OK) {
        String warningMsg = "BME688警告: " + String(envSensor.sensor.status);
        ErrorHandler::logWarning(ErrorComponent::SENSOR, "BME688_WARNING", warningMsg);
        Serial.println(warningMsg);
    }
}