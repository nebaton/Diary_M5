# M5Stack CoreS3 仕様書

## 基本仕様
- **CPU**: ESP32-S3 (Xtensa LX7 dual-core, up to 240MHz)
- **メモリ**: 
  - Flash: 16MB
  - PSRAM: 8MB
  - SRAM: 512KB
- **Wi-Fi**: 802.11 b/g/n (2.4GHz)
- **Bluetooth**: Bluetooth 5, Bluetooth mesh

## ディスプレイ仕様
- **サイズ**: 2.0インチ IPS LCD
- **解像度**: 320×240ピクセル
- **ドライバ**: ILI9342C
- **タッチパネル**: 静電容量式タッチスクリーン (FT6336U)
- **表示領域**: 320px (幅) × 240px (高さ)

## 物理仕様
- **サイズ**: 54.2 × 54.2 × 18.1mm
- **重量**: 47g
- **ケース材質**: PC+ABS

## 電源仕様
- **バッテリー**: 500mAh リチウムポリマー電池
- **充電**: USB Type-C (5V/500mA)
- **動作電圧**: 5V DC
- **消費電流**: 
  - アクティブ時: ~70mA
  - スリープ時: ~2mA

## インターフェース
- **USB**: Type-C (プログラミング・給電・シリアル通信)
- **Grove**: HY2.0-4P コネクタ (I2C)
- **GPIO**: 拡張可能
- **ボタン**: 電源ボタン × 1

## 内蔵センサー
- **IMU**: BMI270 (6軸: 3軸ジャイロ + 3軸加速度)
- **マイク**: SPM1423 (デジタルマイク)
- **スピーカー**: 1W スピーカー内蔵
- **RTC**: BM8563 (リアルタイムクロック)

## 開発環境
- **Arduino IDE**: サポート
- **PlatformIO**: サポート
- **MicroPython**: サポート
- **UIFlow**: サポート

## ピン配置 (Grove ポート)
- **PortA (I2C)**: 
  - SDA: GPIO1
  - SCL: GPIO2
  - 5V, GND

## 参考URL
- 公式ドキュメント: https://docs.m5stack.switch-science.com/ja/core/CoreS3
- GitHub: https://github.com/m5stack/M5CoreS3
- ライブラリ: M5Unified, M5GFX

## 注意事項
- ディスプレイは320×240ピクセルなので、表示内容は画面サイズに合わせて調整が必要
- 日本語フォントを使用する場合、フォントサイズと行数を考慮した設計が重要
- バッテリー駆動時は消費電力を考慮したプログラミングが推奨