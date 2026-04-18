#include <Adafruit_TinyUSB.h>
#include <HIDHost.h>

#ifndef USE_TINYUSB_HOST
#error This example requires USB stack configured as host.
#endif

Adafruit_USBH_Host USBHost;

static bool g_bound = false;
static uint8_t g_dev = 0;
static uint8_t g_inst = 0;
static uint8_t g_cmd = 0x01;
static uint32_t g_last_ms = 0;

void onGenericMount(uint8_t dev_addr, uint8_t instance, uint16_t vid,
                    uint16_t pid) {
  if (g_bound) {
    return;
  }

  if (!HIDGenericHost.bind(dev_addr, instance)) {
    return;
  }

  g_bound = true;
  g_dev = dev_addr;
  g_inst = instance;
  HIDHost.stopAutoReceive(dev_addr, instance);

  Serial.printf("[GEN] bound dev=%u inst=%u vid=%04x pid=%04x\r\n", dev_addr,
                 instance, vid, pid);
}

void onGenericUmount(uint8_t dev_addr, uint8_t instance) {
  if (g_bound && g_dev == dev_addr && g_inst == instance) {
    HIDGenericHost.unbind();
    g_bound = false;
    Serial.printf("[GEN] unbound dev=%u inst=%u\r\n", dev_addr, instance);
  }
}

void onGenericReport(uint8_t const* data, uint16_t len) {
  Serial.printf("[GEN] rx len=%u :", len);
  uint16_t show = (len < 8) ? len : 8;
  for (uint16_t i = 0; i < show; i++) {
    Serial.printf(" %02x", data[i]);
  }
  if (len != 8) {
    Serial.print("  (expected 8 bytes)");
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(100); }
  Serial.println("HIDHost: generic interrupt command example");
  Serial.println("Send 1-byte command, then request 8-byte response.");

  HIDHost.begin();
  HIDGenericHost.onMount(onGenericMount);
  HIDGenericHost.onUmount(onGenericUmount);
  HIDGenericHost.onReport(onGenericReport);

  USBHost.begin(0);
}

void loop() {
  USBHost.task();

  if (!g_bound) {
    return;
  }

  uint32_t now = millis();
  if ((uint32_t)(now - g_last_ms) < 1000u) {
    return;
  }
  g_last_ms = now;

  uint8_t cmd = g_cmd++;
  bool tx_ok = HIDGenericHost.sendReport(&cmd, 1);
  bool rx_ok = HIDGenericHost.requestInputReport();

  Serial.printf("[GEN] tx cmd=0x%02x tx_ok=%d rx_req_ok=%d\r\n", cmd,
                 tx_ok ? 1 : 0, rx_ok ? 1 : 0);
}



