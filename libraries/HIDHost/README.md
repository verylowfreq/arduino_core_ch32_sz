# HIDHost - USB HID Host Utilities for Adafruit TinyUSB

## 概要 / Overview
`HIDHost` は、Adafruit TinyUSB Host の HID コールバックを Arduino から扱いやすい API にまとめたライブラリです。  
This library provides easy-to-use Arduino APIs for USB HID Host devices on top of Adafruit TinyUSB Host.

このライブラリは **Suzuno32RV** を想定して作られています。

## 対応クラス
- `KeyboardHost`（ブートプロトコル）
- `MouseHost`（ブートプロトコル）
- `GamepadHost`（生レポート受け取り）
- `HIDGenericHost`（汎用 HID 通信）
- `HIDHost`（内部ディスパッチャ）

## 依存関係
- Adafruit TinyUSB Library
- USB Stack 設定を Host にすること

## 使い方（最小）
```cpp
#include <Adafruit_TinyUSB.h>
#include <HIDHost.h>

Adafruit_USBH_Host USBHost;

void setup() {
  Serial1.begin(115200);

  HIDHost.begin();

  KeyboardHost.onKeyDown([](uint8_t key, uint8_t mod) {
    Serial1.printf("down key=0x%02x mod=0x%02x\r\n", key, mod);
  });

  USBHost.begin(0); // Suzuno32RV の native host port
}

void loop() {
  USBHost.task();
}
```

## グローバル API
クラス名と同名のグローバルオブジェクトを提供します。
- `HIDHost`
- `KeyboardHost`
- `MouseHost`
- `GamepadHost`
- `HIDGenericHost`

初期化は `HIDHost.begin()` または各クラスの `begin()` で行えます。

## KeyboardHost の ASCII ストリーム
`KeyboardHost` はキー押下から ASCII を生成して 16 バイトのリングバッファに保持します。

- `int available()`
- `int peak()`（データなしで `-1`）
- `int read()`（データなしで `-1`）
- `size_t read(uint8_t* buf, size_t len)`
- `void flush()`
- `void setLayout(KeyboardLayout::US_ASCII / KeyboardLayout::JP_JIS)`
- `void clearStateOnUmount(bool enabled = true)`

補足:
- ASCII は新規 key down 時のみ生成します。
- 修飾キーのみの変化では ASCII は生成しません。
- `readAscii()` はありません（`read()` を使用）。

## MouseHost の注意
- ブートプロトコル前提です。
- `onMove(dx, dy, buttons)` はボタンのみ変化時（`dx=0,dy=0`）でも呼ばれます。
- `onHWheel()` は、ブートレポートで横ホイールを返す非標準マウス向けです。

## HIDGenericHost の使い方
- `onMount()` で `dev_addr`/`instance` を受け取り `bind()` します。
- `sendReport()` / `requestInputReport()` / `getFeatureReport()` / `setFeatureReport()` を利用します。
- デバイス抜去時は `onUmount()` で `unbind()` します。

## サンプル
- `examples/keyboard_mouse_dump`
- `examples/gamepad_buttons`
- `examples/hid_generic_command`
- `examples/hid_mount_info`
- `examples/keyboard_ascii_stream`

## 既知の前提
- キーボード/マウスはブートプロトコル想定です。
- 複雑な Report Protocol 解析が必要なデバイスは `HIDGenericHost` または `GamepadHost` を使ってください。

## 複数デバイス管理の制約
- 内部では `dev_addr + instance` で複数 HID インターフェースを同時管理します。
- `KeyboardHost` / `MouseHost` / `GamepadHost` はクラスごとにコールバック1本の集約型です。
- `GamepadHost.onReport()` は `HIDRawReportInfo` に `dev_addr/instance` を含むため、アプリ側でデバイス識別できます。
- `KeyboardHost.onKeyDown/onKeyUp` と `MouseHost.onMove/onWheel/onHWheel` はデバイスIDを直接渡さないため、複数台同時利用時は入力が合流します。
- `KeyboardHost.setLeds(uint8_t leds)` は最初に見つかったキーボードへ送信します。複数台を明示的に制御する場合は `setLeds(dev_addr, instance, leds)` を使ってください。
- `HIDGenericHost` は `bind(dev_addr, instance)` で選んだ1インターフェースのみを対象にする設計です。
