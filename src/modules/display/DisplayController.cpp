#include "DisplayController.h"
#include "ErrorHandler.h"

DisplayController::DisplayController() :
    currentPage(DisplayPage::SENSOR_DATA_1),
    lastPageChange(0),
    touchPressed(false) {
}

DisplayController::~DisplayController() {
}

void DisplayController::initialize() {
    // M5Stackディスプレイの初期設定
    M5.Display.setRotation(1);  // 横向き
    M5.Display.setTextSize(2);
    clearScreen();
    
    // 日本語フォントを設定
    M5.Display.setFont(&fonts::lgfxJapanGothic_12);
    
    Serial.println("ディスプレイコントローラーを初期化しました");
}

void DisplayController::clearScreen() {
    M5.Display.fillScreen(BLACK);  // 背景を黒で塗りつぶし
    M5.Display.setFont(&fonts::lgfxJapanGothic_12);  // 日本語ゴシック体12px
}

void DisplayController::lcdPrint(int y, const String& msg, uint32_t color) {
    M5.Display.setCursor(5, y);  // 左マージンを5pxに
    M5.Display.setTextColor(color, BLACK);
    M5.Display.print(msg);
}

void DisplayController::lcdPrint(int x, int y, const String& msg, uint32_t color) {
    M5.Display.setCursor(x, y);
    M5.Display.setTextColor(color, BLACK);
    M5.Display.print(msg);
}

void DisplayController::showSensorData(const SensorReading& data, DisplayPage page) {
    lastSensorData = data;
    
    // 現在のページに応じてデータを表示
    if (page == DisplayPage::SENSOR_DATA_1) {
        renderSensorDataPage1();
    } else if (page == DisplayPage::SENSOR_DATA_2) {
        renderSensorDataPage2();
    }
}

void DisplayController::renderSensorDataPage1() {
    clearScreen();
    
    int y = 15;  // 上部マージン
    
    // ページヘッダー
    lcdPrint(y, "環境データ (1/2)", CYAN);
    y += LINE_HEIGHT + 4;
    
    // センサーデータを表示
    lcdPrint(y, "温度: " + String(lastSensorData.temperature, 1) + "℃", WHITE);
    y += LINE_HEIGHT;
    lcdPrint(y, "湿度: " + String(lastSensorData.humidity, 1) + "%", WHITE);
    y += LINE_HEIGHT;
    lcdPrint(y, "気圧: " + String(lastSensorData.pressure, 0) + "hPa", WHITE);
    y += LINE_HEIGHT;
    lcdPrint(y, "CO2: " + String(lastSensorData.co2_equivalent, 0) + "ppm", WHITE);
    
    drawPageFooter();
}

void DisplayController::renderSensorDataPage2() {
    clearScreen();
    
    int y = 15;  // 上部マージン
    
    // ページヘッダー
    lcdPrint(y, "空気質データ (2/2)", CYAN);
    y += LINE_HEIGHT + 4;
    
    // 空気質データを表示
    lcdPrint(y, "空気質: " + String(lastSensorData.iaq, 0), WHITE);
    y += LINE_HEIGHT;
    lcdPrint(y, "VOC: " + String(lastSensorData.voc_equivalent, 1) + "ppm", WHITE);
    y += LINE_HEIGHT;
    lcdPrint(y, "状態: " + String(lastSensorData.stabilized ? "安定" : "調整中"), WHITE);
    y += LINE_HEIGHT;
    lcdPrint(y, "慣らし: " + String(lastSensorData.runin_status, 0) + "%", WHITE);
    y += LINE_HEIGHT;
    
    // 慣らしの説明を追加
    if (lastSensorData.runin_status < 50) {
        lcdPrint(y, "※長期慣らし運転中", YELLOW);
    }
    
    drawPageFooter();
}

void DisplayController::drawPageFooter() {
    int y = SCREEN_HEIGHT - 50;
    
    // ページ切り替えボタン
    String buttonText = (currentPage == DisplayPage::SENSOR_DATA_1) ? "次へ >" : "< 前へ";
    int buttonX = 240;  // 右端から余裕を持たせる
    
    M5.Display.setTextColor(WHITE, DARKGREY);  // 白文字、グレー背景
    M5.Display.setCursor(buttonX, y);
    M5.Display.print(buttonText);
    
    // ページインジケーター（左下）
    int pageNum = (currentPage == DisplayPage::SENSOR_DATA_1) ? 1 : 2;
    lcdPrint(10, y, "ページ " + String(pageNum) + "/2", GREEN);
    
    // タイムスタンプ（左下）
    y += 20;
    lcdPrint(10, y, "更新: " + String(millis()/1000) + "秒", YELLOW);
}

void DisplayController::showStatus(const SystemStatus& status) {
    lastSystemStatus = status;
    renderStatusPage();
}

void DisplayController::renderStatusPage() {
    clearScreen();
    
    int y = 15;
    lcdPrint(y, "システム状態", CYAN);
    y += LINE_HEIGHT + 4;
    
    // システム状態を表示
    lcdPrint(y, "センサー: " + String(lastSystemStatus.sensor_healthy ? "正常" : "エラー"), 
             lastSystemStatus.sensor_healthy ? GREEN : RED);
    y += LINE_HEIGHT;
    
    String connStatus = "未接続";
    uint32_t connColor = RED;
    switch (lastSystemStatus.connection_status) {
        case ConnectionStatus::CONNECTED:
            connStatus = "接続中";
            connColor = GREEN;
            break;
        case ConnectionStatus::CONNECTING:
            connStatus = "接続試行中";
            connColor = YELLOW;
            break;
        case ConnectionStatus::ERROR:
            connStatus = "接続エラー";
            connColor = RED;
            break;
    }
    lcdPrint(y, "ネットワーク: " + connStatus, connColor);
    y += LINE_HEIGHT;
    
    lcdPrint(y, "ストレージ: " + String(lastSystemStatus.storage_usage_percent) + "%", WHITE);
    y += LINE_HEIGHT;
    lcdPrint(y, "バッテリー: " + String(lastSystemStatus.battery_level) + "%", WHITE);
    y += LINE_HEIGHT;
    lcdPrint(y, "稼働時間: " + String(lastSystemStatus.uptime_seconds) + "秒", WHITE);
}

void DisplayController::update() {
    handleTouch();
}

void DisplayController::handleTouch() {
    M5.update();  // M5のタッチ状態を更新
    
    if (M5.Touch.getCount() > 0) {
        if (!touchPressed) {  // 新しいタッチが検出された場合のみ
            auto touch = M5.Touch.getDetail();
            
            // 画面右半分のタッチでページ切り替え
            if (touch.x > SCREEN_WIDTH / 2 && millis() - lastPageChange > TOUCH_DEBOUNCE_MS) {
                // センサーデータページ間の切り替え
                if (currentPage == DisplayPage::SENSOR_DATA_1) {
                    currentPage = DisplayPage::SENSOR_DATA_2;
                } else if (currentPage == DisplayPage::SENSOR_DATA_2) {
                    currentPage = DisplayPage::SENSOR_DATA_1;
                }
                
                lastPageChange = millis();
                touchPressed = true;
                
                // 現在のページを再描画
                if (currentPage == DisplayPage::SENSOR_DATA_1) {
                    renderSensorDataPage1();
                } else {
                    renderSensorDataPage2();
                }
                
                Serial.println("ページを切り替えました: " + String((int)currentPage + 1));
            }
        }
    } else {
        touchPressed = false;  // タッチが離されたらリセット
    }
}

void DisplayController::setCurrentPage(DisplayPage page) {
    currentPage = page;
}

void DisplayController::showMessage(const String& message, uint32_t color, uint32_t duration) {
    clearScreen();
    lcdPrint(SCREEN_HEIGHT / 2 - 10, message, color);
    delay(duration);
}

void DisplayController::showError(const String& error) {
    showMessage("エラー: " + error, RED, 3000);
}

void DisplayController::showWarning(const String& warning) {
    showMessage("警告: " + warning, YELLOW, 2000);
}

void DisplayController::setBrightness(uint8_t brightness) {
    M5.Display.setBrightness(brightness);
}