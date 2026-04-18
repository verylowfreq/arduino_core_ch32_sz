# SerialHost - USB CDC Host Utilities for Adafruit TinyUSB

## Overview
`SerialHost` is an Arduino-friendly wrapper for TinyUSB CDC host callbacks.
It centralizes TinyUSB callback entry points and provides per-device handle APIs via `SerialDevice`.

This library is designed for USB host use (for example, Suzuno32RV native host port).

## Features
- Centralized TinyUSB CDC callback routing (`SerialHost`)
- Per-device handle model (`SerialDevice`) for multi-device use
- Hot-plug-safe handle validity with `attached()`
- Line control and serial settings APIs
  - `connect()/disconnect()`
  - `setDtrRts()`
  - `setBaudrate()`
  - `setLineCoding()`
- Stream-like I/O APIs
  - `available()/peek()/read()`
  - `write()/flush()`

## Dependencies
- Adafruit TinyUSB Library
- USB stack configured as Host

## Basic usage
```cpp
#include <Adafruit_TinyUSB.h>
#include <SerialHost.h>

Adafruit_USBH_Host USBHost;
SerialDevice g_dev;

void setup() {
  Serial.begin(115200);
  SerialHost.begin();

  SerialHost.onMount([](SerialDevice const& dev, SerialHostInterfaceInfo const& info) {
    g_dev = dev; // copy and keep
    g_dev.connect();
    g_dev.setBaudrate(115200);
    Serial.printf("mount idx=%u addr=%u itf=%u\r\n", info.idx, info.dev_addr, info.itf_num);
  });

  SerialHost.onUmount([](SerialDevice const& dev, SerialHostInterfaceInfo const& info) {
    if (g_dev == dev) {
      g_dev = SerialDevice();
      Serial.printf("umount idx=%u\r\n", info.idx);
    }
  });

  USBHost.begin(0);
}

void loop() {
  USBHost.task();

  if (g_dev.attached() && g_dev.connected() && g_dev.available()) {
    int c = g_dev.read();
    if (c >= 0) {
      Serial.write((uint8_t)c);
    }
  }
}
```

## Handle model
`SerialDevice` is a lightweight value handle.

- `attached()`:
  - true when the handle is still bound to a currently mounted CDC interface.
  - false when the device was unplugged or the handle became stale.
- `connected()`:
  - true only when `attached()` is true and CDC DTR is asserted.

Recommended check order in app code:
1. `attached()`
2. `connected()`
3. read/write operations

## Callback API
- `onMount(MountCallback)`
- `onUmount(UmountCallback)`
- `onRx(RxCallback)`
- `onTxComplete(TxCompleteCallback)`
- `onError(ErrorCallback)`

TinyUSB callback bridge is provided internally:
- `tuh_cdc_mount_cb`
- `tuh_cdc_umount_cb`
- `tuh_cdc_rx_cb`
- `tuh_cdc_tx_complete_cb`

## Examples
- `examples/serialhost_mount_info`
- `examples/serialhost_serial_bridge`
- `examples/serialhost_multi_write`

## License
MIT License. See [LICENSE](./LICENSE).

## Author
Mitsumine Suzu (verylowfreq)
