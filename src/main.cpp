#include <M5Unified.h>
#include "YokanAISystem.h"

// グローバルシステムインスタンス
YokanAISystem yokanSystem;

void setup() {
    // M5Stackの初期化
    auto cfg = M5.config();
    M5.begin(cfg);
    Serial.begin(115200);
    
    // ディスプレイの向きと画面クリア
    M5.Display.setRotation(1);
    M5.Display.clear();
    M5.Display.setFont(&fonts::lgfxJapanGothic_12);
    
    // 起動メッセージを表示
    M5.Display.setCursor(10, 10);
    M5.Display.setTextColor(GREEN);
    M5.Display.println("予感AIちゃん システム起動中...");
    
    Serial.println("=== Yokan AI System Starting ===");
    
    // システムの初期化
    if (yokanSystem.initialize()) {
        Serial.println("システムの初期化に成功しました");
        M5.Display.setCursor(10, 40);
        M5.Display.setTextColor(GREEN);
        M5.Display.println("システム初期化完了!");
    } else {
        Serial.println("システムの初期化に失敗しました");
        M5.Display.setCursor(10, 40);
        M5.Display.setTextColor(RED);
        M5.Display.println("初期化エラー!");
        
        // エラー詳細を表示
        auto errors = ErrorHandler::getRecentErrors(5);
        int y = 70;
        for (const auto& error : errors) {
            M5.Display.setCursor(10, y);
            M5.Display.setTextColor(YELLOW);
            M5.Display.println(error.message.substring(0, 30));
            y += 20;
        }
        
        // 制限された機能で続行を試行
        delay(5000);
    }
    
    Serial.println("=== System Ready ===");
}

void loop() {
    // M5Stackハードウェア状態の更新
    M5.update();
    
    // メインシステムの更新
    yokanSystem.update();
    
    // システムを圧迫しないよう小さな遅延
    delay(50);
}