#include <Adafruit_TinyUSB.h>
#include <SerialHost.h>

#ifndef USE_TINYUSB_HOST
#error This example requires USB stack configured as host.
#endif

Adafruit_USBH_Host USBHost;

static constexpr uint8_t MAX_TRACKED_DEVICES = 8;
SerialDevice g_dev[MAX_TRACKED_DEVICES];
SerialHostInterfaceInfo g_info[MAX_TRACKED_DEVICES];

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

void printTrackedList() {
  Serial.println("tracked devices:");
  for (uint8_t i = 0; i < MAX_TRACKED_DEVICES; i++) {
    if (!g_dev[i].attached()) {
      continue;
    }

    Serial.printf("  slot=%u idx=%u addr=%u itf=%u vid=%04x pid=%04x attached=%u connected=%u\r\n",
                  i,
                  g_info[i].idx,
                  g_info[i].dev_addr,
                  g_info[i].itf_num,
                  g_info[i].vid,
                  g_info[i].pid,
                  g_dev[i].attached() ? 1 : 0,
                  g_dev[i].connected() ? 1 : 0);
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(100); }

  Serial.println("SerialHost: mount info (handle copy model)");

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

    Serial.printf("[CDC] mount slot=%d idx=%u addr=%u itf=%u vid=%04x pid=%04x\r\n",
                  slot, info.idx, info.dev_addr, info.itf_num, info.vid, info.pid);

    if (!g_dev[slot].connect()) {
      Serial.printf("[CDC] connect failed idx=%u\r\n", info.idx);
    }

    printTrackedList();
  });

  SerialHost.onUmount([](SerialDevice const& dev,
                         SerialHostInterfaceInfo const& info) {
    int slot = findTracked(dev);
    if (slot >= 0) {
      g_dev[slot] = SerialDevice();
    }

    Serial.printf("[CDC] umount idx=%u addr=%u itf=%u\r\n", info.idx,
                  info.dev_addr, info.itf_num);
    printTrackedList();
  });

  USBHost.begin(0);
}

void loop() {
  USBHost.task();
}
