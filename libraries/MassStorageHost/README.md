# MassStorageHost - USB MSC Host Utilities for Adafruit TinyUSB

## 概要 / Overview
`MassStorageHost` は TinyUSB MSC host callback を Arduino から扱いやすくするラッパーです。  
`MassStorageHost` provides an Arduino-friendly wrapper for TinyUSB MSC host callbacks.

このライブラリは以下の環境を前提としています:
- Adafruit TinyUSB host stack
- Adafruit SdFat fork (`SdFat_Adafruit_Fork.h`)

## クラスとグローバル
- `MassStorageHostAPI`
- グローバルオブジェクト: `MassStorageHost`

## 機能 (MVP)
- 最初に接続された MSC デバイスの `LUN0` を自動マウント
- 最初にマウントされたボリュームを操作対象として維持（後から接続されたデバイスで自動切り替えしない）
- アクティブボリュームを `MscVolume` ラッパー経由で操作:
  - `exists`, `mkdir`, `remove`, `rename`
  - `open(MscFile& out, ...)`
  - `open(...) -> MscFile`
  - `ls()`, `ls(&stream)`

## 最小使用例
```cpp
#include <Adafruit_TinyUSB.h>
#include <MassStorageHost.h>

Adafruit_USBH_Host USBHost;

void setup() {
  Serial.begin(115200);
  MassStorageHost.begin();

  MassStorageHost.onVolumeMount([](MassStorageVolumeInfo const& info) {
    Serial.printf("Mounted: addr=%u lun=%u blocks=%lu block_size=%lu\r\n",
                  info.dev_addr, info.lun, info.block_count, info.block_size);
  });

  USBHost.begin(0);
}

void loop() {
  USBHost.task();
}
```

## サンプル
- `examples/massstorage_basic`
- `examples/massstorage_create_write`

## 注意事項
- `loop()` などで `USBHost.task()` を継続的に高頻度に呼び出してください。
- 既定では `MscVolume=FatVolume`, `MscFile=File32` です。
- スケッチの先頭で `MASSSTORAGEHOST_USE_EXFAT=1` を定義すると exFATに対応します。内部的には `MscVolume=FsVolume`, `MscFile=FsFile` に切り替わります。

## 開発者向け

### MSH_USBH_BlockDevice について

- `src/MSH_USBH_BlockDevice.h` / `src/MSH_USBH_BlockDevice.cpp` は、Adafruit TinyUSB の MSC host block device 実装をベースに、クラス名を `MSH_USBH_BlockDevice` へ変更して取り込んだものです。
- 参照元コミット:
  - https://github.com/adafruit/Adafruit_TinyUSB_Arduino/commit/639002c3d95f689e1a19ee6c55cafa41a98e80dd
- 参照元ファイル:
  - https://github.com/adafruit/Adafruit_TinyUSB_Arduino/blob/639002c3d95f689e1a19ee6c55cafa41a98e80dd/src/arduino/msc/Adafruit_USBH_MSC.h
  - https://github.com/adafruit/Adafruit_TinyUSB_Arduino/blob/639002c3d95f689e1a19ee6c55cafa41a98e80dd/src/arduino/msc/Adafruit_USBH_MSC.cpp
