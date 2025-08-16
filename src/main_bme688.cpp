// #include <Wire.h>
// #include <M5CoreS3.h>
// #define BME688_I2C_ADDRESS 0x77
// #define BME688_CHIP_ID_REG 0xD0  // チップIDのレジスタ

// void setup() {
//   M5.begin();
//   M5.Lcd.fillScreen(TFT_BLACK);
//   M5.Lcd.setTextColor(TFT_WHITE);
//   M5.Lcd.setTextSize(2);
//   M5.Lcd.setCursor(0, 0);
//   M5.Lcd.println("I2C Scanner starting...");

//   Wire.begin();
//   Wire.beginTransmission(BME688_I2C_ADDRESS);
//   Wire.write(BME688_CHIP_ID_REG);
//   Wire.endTransmission();
//   Wire.requestFrom(BME688_I2C_ADDRESS, 1);

//   if (Wire.available()) {
//     uint8_t chipID = Wire.read();
//     M5.Lcd.print("Chip ID: 0x");
//     M5.Lcd.println(chipID, HEX);
//     if (chipID == 0x61) {
//       M5.Lcd.println("これはBME688です！");
//     } else {
//       M5.Lcd.println("未知のデバイスまたは別モデル");
//     }
//   } else {
//     M5.Lcd.println("デバイスからの応答なし");
//   }
//   M5.Lcd.println("\nDone.");
// }

// void loop() {}

#include <M5Unified.h>
#include <Wire.h>
#include <bsec2.h>

Bsec2 envSensor;  // BSEC2ライブラリ用インスタンス
int currentPage = 0;  // 現在のページ（0: ページ1, 1: ページ2）
unsigned long lastPageChange = 0;  // 最後にページを変更した時刻
bool touchPressed = false;  // タッチ状態を記録

// LCDにメッセージを表示（位置指定）
void lcdPrint(int y, const String& msg, uint32_t color = GREEN) {
  M5.Display.setCursor(5, y);  // 左マージンを5pxに
  M5.Display.setTextColor(color, BLACK);
  M5.Display.print(msg);
}

// LCDにメッセージを表示（x,y座標指定）
void lcdPrint(int x, int y, const String& msg, uint32_t color = GREEN) {
  M5.Display.setCursor(x, y);
  M5.Display.setTextColor(color, BLACK);
  M5.Display.print(msg);
}

// 画面クリア（フォント設定も再適用）
void clearScreen() {
  M5.Display.fillScreen(BLACK);  // 背景を黒で塗りつぶし
  M5.Display.setFont(&fonts::lgfxJapanGothic_12);  // 日本語ゴシック体12px
}

void checkBsecStatus(Bsec2& bsec) {
  if (bsec.status < BSEC_OK) {
    lcdPrint(10, "BSEC error: " + String(bsec.status), RED);
    Serial.println("BSEC Error: " + String(bsec.status));
  } else if (bsec.status > BSEC_OK) {
    lcdPrint(10, "BSEC warn: " + String(bsec.status), YELLOW);
    Serial.println("BSEC Warning: " + String(bsec.status));
    
    // warn: 14 (BSEC_W_SC_CALL_TIMING_VIOLATION) の詳細説明
    if (bsec.status == 14) {
      lcdPrint(200, "Timing violation detected", YELLOW);
      Serial.println("BSEC Warning 14: Call timing violation - adjust loop timing");
    }
  }

  if (bsec.sensor.status < BME68X_OK) {
    lcdPrint(30, "BME688 error: " + String(bsec.sensor.status), RED);
    Serial.println("BME688 Error: " + String(bsec.sensor.status));
  } else if (bsec.sensor.status > BME68X_OK) {
    lcdPrint(30, "BME688 warn: " + String(bsec.sensor.status), YELLOW);
    Serial.println("BME688 Warning: " + String(bsec.sensor.status));
  }
}

// センサーデータを格納する構造体
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
};

SensorData sensorData;

// 関数の前方宣言
void displayCurrentPage();

// 現在のページを表示
void displayCurrentPage() {
  if (!sensorData.dataValid) return;
  
  // 画面をクリアして日本語フォントを再設定
  clearScreen();
  
  // 320×240画面に最適化した表示（見やすい行間に戻す）
  int y = 15;  // 上部マージン
  int lineHeight = 24;  // 見やすい行間
  
  // ページヘッダー
  String pageTitle = (currentPage == 0) ? "環境データ (1/2)" : "空気質データ (2/2)";
  lcdPrint(y, pageTitle, CYAN);
  y += lineHeight + 4;
  
  if (currentPage == 0) {
    // ページ1: 温度、湿度、気圧、CO2
    lcdPrint(y, "温度: " + String(sensorData.temperature, 1) + "℃", WHITE);
    y += lineHeight;
    lcdPrint(y, "湿度: " + String(sensorData.humidity, 1) + "%", WHITE);
    y += lineHeight;
    lcdPrint(y, "気圧: " + String(sensorData.pressure, 0) + "hPa", WHITE);
    y += lineHeight;
    lcdPrint(y, "CO2: " + String(sensorData.co2, 0) + "ppm", WHITE);
  } else {
    // ページ2: 空気質、VOC、状態、慣らし
    lcdPrint(y, "空気質: " + String(sensorData.iaq, 0), WHITE);
    y += lineHeight;
    lcdPrint(y, "VOC: " + String(sensorData.voc, 1) + "ppm", WHITE);
    y += lineHeight;
    lcdPrint(y, "状態: " + String(sensorData.stabilized ? "安定" : "調整中"), WHITE);
    y += lineHeight;
    lcdPrint(y, "慣らし: " + String(sensorData.runin, 0) + "%", WHITE);
    y += lineHeight;
    
    // 慣らしの説明を追加
    if (sensorData.runin < 50) {
      lcdPrint(y, "※長期慣らし運転中", YELLOW);
    }
  }
  
  // 画面下部にページ切り替えボタンとタイムスタンプ
  y = 240 - 50;
  
  // ページ切り替えボタン（テキスト背景のみ）
  String buttonText = (currentPage == 0) ? "次へ >" : "< 前へ";
  
  // ボタンテキストを右下に配置
  int buttonX = 240;  // 右端から余裕を持たせる
  int buttonY = y;
  
  M5.Display.setFont(&fonts::lgfxJapanGothic_12);
  M5.Display.setTextColor(WHITE, DARKGREY);  // 白文字、グレー背景
  M5.Display.setCursor(buttonX, buttonY);
  M5.Display.print(buttonText);
  
  // ページインジケーター（左下）
  M5.Display.setFont(&fonts::lgfxJapanGothic_12);  // フォントを統一
  lcdPrint(10, y, "ページ " + String(currentPage + 1) + "/2", GREEN);
  
  // タイムスタンプ（左下）
  y += 20;
  lcdPrint(10, y, "更新: " + String(millis()/1000) + "秒", YELLOW);
}

// 新しいデータが取得されたときの処理
void newDataCallback(const bme68xData data, const bsecOutputs outputs, Bsec2 bsec) {
  if (!outputs.nOutputs) return;

  // データを構造体に格納
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
        sensorData.pressure = output.signal / 100;  // hPaに変換
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
  displayCurrentPage();
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  Serial.begin(115200);
  M5.Display.setRotation(1);
  M5.Display.setTextSize(2);
  M5.Display.clear();
  
  // 日本語フォント設定（12ピクセル ゴシック体）
  M5.Display.setFont(&fonts::lgfxJapanGothic_12);
  
  M5.Display.setCursor(10, 10);
  M5.Display.setTextColor(GREEN);
  M5.Display.println("BSEC2 初期化中...");

  Wire.begin();  // CoreS3のPortAを利用（SDA=1, SCL=2）
  Serial.println("Starting BME688 BSEC2 initialization...");
  
  // センサー安定化のため少し待機
  delay(1000);

  // BME688の初期化（アドレスを試行）
  if (!envSensor.begin(BME68X_I2C_ADDR_HIGH, Wire)) {
    lcdPrint(30, "Trying LOW addr...", YELLOW);
    if (!envSensor.begin(BME68X_I2C_ADDR_LOW, Wire)) {
      checkBsecStatus(envSensor);
      lcdPrint(50, "BME688 Init Failed!", RED);
      return;
    }
  }

  // BSEC出力の登録（確実に動作する基本データのみ）
  bsecSensor sensorList[] = {
    BSEC_OUTPUT_IAQ,                                        // 室内空気品質指数 (0-500)
    BSEC_OUTPUT_STATIC_IAQ,                                 // 静的IAQ
    BSEC_OUTPUT_CO2_EQUIVALENT,                             // CO2等価値 (ppm)
    BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,                      // 呼気VOC等価値 (ppm)
    BSEC_OUTPUT_RAW_TEMPERATURE,                            // 生温度 (°C)
    BSEC_OUTPUT_RAW_PRESSURE,                               // 生気圧 (Pa)
    BSEC_OUTPUT_RAW_HUMIDITY,                               // 生湿度 (%)
    BSEC_OUTPUT_RAW_GAS,                                    // 生ガス抵抗値 (Ohm)
    BSEC_OUTPUT_STABILIZATION_STATUS,                       // 安定化状態 (0/1)
    BSEC_OUTPUT_RUN_IN_STATUS                               // 慣らし運転状態 (0-100%)
  };

  // サンプルレート設定
  // BSEC_SAMPLE_RATE_ULP    = 5分間隔（超低消費電力）
  // BSEC_SAMPLE_RATE_LP     = 3秒間隔（低消費電力）
  // BSEC_SAMPLE_RATE_CONT   = 1秒間隔（連続測定）
  
  // タイミング問題を避けるため、まずULPモードで試す
  if (!envSensor.updateSubscription(sensorList, ARRAY_LEN(sensorList), BSEC_SAMPLE_RATE_ULP)) {
    checkBsecStatus(envSensor);
    lcdPrint(70, "Subscription Failed!", RED);
    return;
  }

  envSensor.attachCallback(newDataCallback);
  lcdPrint(10, "BSEC準備完了!", GREEN);
  
  // 初期画面表示
  delay(2000);
  clearScreen();
  lcdPrint(50, "BME688 センサー", CYAN);
  lcdPrint(80, "データ取得中...", WHITE);
  lcdPrint(110, "画面右側タッチで", GREEN);
  lcdPrint(130, "ページ切り替え", GREEN);
}

void loop() {
  M5.update();  // M5のタッチやボタンの状態を更新
  
  // タッチでページ切り替え（改善版）
  if (M5.Touch.getCount() > 0) {
    if (!touchPressed) {  // タッチが新しく検出された場合のみ
      auto touch = M5.Touch.getDetail();
      
      // ボタン領域の定義（テキスト領域に合わせて調整）
      int buttonX = 240;
      int buttonY = 240 - 50;
      int buttonW = 70;  // テキスト幅に合わせて調整
      int buttonH = 20;  // テキスト高さに合わせて調整
      
      // ボタン領域内のタッチまたは画面右半分のタッチ
      bool inButtonArea = (touch.x >= buttonX && touch.x <= buttonX + buttonW && 
                          touch.y >= buttonY - 5 && touch.y <= buttonY + buttonH);
      bool inRightHalf = (touch.x > 160);
      
      if ((inButtonArea || inRightHalf) && millis() - lastPageChange > 1000) {
        currentPage = (currentPage == 0) ? 1 : 0;
        lastPageChange = millis();
        touchPressed = true;  // タッチ状態を記録
        displayCurrentPage();
        Serial.println("Page changed to: " + String(currentPage + 1));
        Serial.println("Touch at: " + String(touch.x) + ", " + String(touch.y));
        if (inButtonArea) Serial.println("Button area touched");
      }
    }
  } else {
    touchPressed = false;  // タッチが離されたらリセット
  }
  
  // BSECライブラリを定期的に呼び出す
  if (!envSensor.run()) {
    checkBsecStatus(envSensor);
    delay(5000); // エラー時は5秒待機
    return;
  }
  
  // BSEC_SAMPLE_RATE_ULPでは5分間隔が推奨
  // タッチ反応を良くするため短い間隔でチェック
  delay(100); // 100ms間隔でチェック（タッチ反応を向上）
}
