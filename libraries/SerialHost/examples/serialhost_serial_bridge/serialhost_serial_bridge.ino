#include <Adafruit_TinyUSB.h>
#include <SerialHost.h>

#ifndef USE_TINYUSB_HOST
#error This example requires USB stack configured as host.
#endif

Adafruit_USBH_Host USBHost;

static constexpr uint8_t MAX_TRACKED_DEVICES = 8;
SerialDevice g_dev[MAX_TRACKED_DEVICES];

int findTracked(SerialDevice const& dev) {
  for (uint8_t i = 0; i < MAX_TRACKED_DEVICES; i++) {
    if (g_dev[i] == dev) {
      return i;
    }
  }
  return -1;
}

int allocTracked() {
  for (uint8_t i = 0; i < MAX_TRACKED_DEVICES; i++) {
    if (!g_dev[i].attached()) {
      return (int)i;
    }
  }
  return -1;
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(100); }

  Serial.println("SerialHost: Serial <-> USB CDC bridge (handle copy model)");

  SerialHost.begin();

  SerialHost.onMount([](SerialDevice const& dev,
                        SerialHostInterfaceInfo const& info) {
    int slot = findTracked(dev);
    if (slot < 0) {
      slot = allocTracked();
    }
    if (slot < 0) {
      Serial.println("[CDC] no free tracking slot");
      return;
    }

    g_dev[slot] = dev;

    Serial.printf("[CDC] mount slot=%d idx=%u addr=%u itf=%u vid=%04x pid=%04x\r\n",
                  slot, info.idx, info.dev_addr, info.itf_num, info.vid, info.pid);

    g_dev[slot].connect();
    g_dev[slot].setBaudrate(115200);
  });

  SerialHost.onUmount([](SerialDevice const& dev,
                         SerialHostInterfaceInfo const& info) {
    int slot = findTracked(dev);
    if (slot >= 0) {
      g_dev[slot] = SerialDevice();
    }

    Serial.printf("[CDC] umount idx=%u addr=%u itf=%u\r\n", info.idx,
                  info.dev_addr, info.itf_num);
  });

  USBHost.begin(0);
}

void loop() {
  USBHost.task();

  uint8_t buf[64];

  // PC Serial -> all tracked devices
  if (Serial.available()) {
    size_t n = Serial.readBytes((char*)buf, sizeof(buf));

    for (uint8_t i = 0; i < MAX_TRACKED_DEVICES; i++) {
      if (!g_dev[i].attached() || !g_dev[i].connected()) {
        continue;
      }

      g_dev[i].write(buf, n);
      g_dev[i].flush();
    }
  }

  // tracked devices -> PC Serial
  for (uint8_t i = 0; i < MAX_TRACKED_DEVICES; i++) {
    if (!g_dev[i].attached() || !g_dev[i].connected()) {
      continue;
    }

    while (g_dev[i].available() > 0) {
      size_t n = g_dev[i].read(buf, sizeof(buf));
      if (!n) {
        break;
      }

      Serial.printf("[slot=%u idx=%u] ", i, g_dev[i].index());
      Serial.write(buf, n);
    }
  }
}
