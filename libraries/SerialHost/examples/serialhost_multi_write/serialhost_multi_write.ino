#include <Adafruit_TinyUSB.h>
#include <SerialHost.h>

#ifndef USE_TINYUSB_HOST
#error This example requires USB stack configured as host.
#endif

Adafruit_USBH_Host USBHost;

static constexpr uint8_t MAX_TRACKED_DEVICES = 8;
SerialDevice g_dev[MAX_TRACKED_DEVICES];
SerialHostInterfaceInfo g_info[MAX_TRACKED_DEVICES];

uint32_t g_last_ms = 0;
uint32_t g_tick = 0;

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

  Serial.println("SerialHost: multi-device periodic write (handle copy model)");

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
    g_info[slot] = info;

    Serial.printf("[CDC] mount slot=%d idx=%u addr=%u itf=%u\r\n", slot,
                  info.idx, info.dev_addr, info.itf_num);

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

  uint32_t now = millis();
  if ((uint32_t)(now - g_last_ms) < 1000u) {
    return;
  }
  g_last_ms = now;

  for (uint8_t i = 0; i < MAX_TRACKED_DEVICES; i++) {
    if (!g_dev[i].attached() || !g_dev[i].connected()) {
      continue;
    }

    char line[96];
    int len = snprintf(line, sizeof(line),
                       "tick=%lu slot=%u idx=%u addr=%u itf=%u\r\n",
                       (unsigned long)g_tick,
                       i,
                       g_info[i].idx,
                       g_info[i].dev_addr,
                       g_info[i].itf_num);

    if (len > 0) {
      g_dev[i].write((uint8_t const*)line, (size_t)len);
      g_dev[i].flush();
    }
  }

  g_tick++;
}
