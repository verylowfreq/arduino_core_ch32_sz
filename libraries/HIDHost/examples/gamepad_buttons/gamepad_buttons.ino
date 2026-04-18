#include <Adafruit_TinyUSB.h>
#include <HIDHost.h>

#ifndef USE_TINYUSB_HOST
#error This example requires USB stack configured as host.
#endif

Adafruit_USBH_Host USBHost;

static uint16_t last_buttons = 0;
static bool has_last_buttons = false;

static char const* kBtnNames[16] = {"A",      "B",      "X",      "Y",
                                    "L1",     "R1",     "L2",     "R2",
                                    "Select", "Start",  "LStick", "RStick",
                                    "Home",   "Btn13",  "Btn14",  "Btn15"};

void printButtonDiff(uint16_t prev, uint16_t cur) {
  uint16_t changed = (uint16_t)(prev ^ cur);
  if (changed == 0) {
    return;
  }
  for (uint8_t i = 0; i < 16; i++) {
    if (changed & (1u << i)) {
      bool down = (cur & (1u << i)) != 0;
      Serial.printf("[GP] %s %s\r\n", kBtnNames[i], down ? "DOWN" : "UP");
    }
  }
}

void onGamepadMount(uint8_t dev_addr, uint8_t instance, uint16_t vid,
                    uint16_t pid) {
  Serial.printf("[GP] mount dev=%u inst=%u vid=%04x pid=%04x\r\n", dev_addr,
                 instance, vid, pid);
  has_last_buttons = false;
}

void onGamepadUmount(uint8_t dev_addr, uint8_t instance) {
  Serial.printf("[GP] umount dev=%u inst=%u\r\n", dev_addr, instance);
  has_last_buttons = false;
}

void onGamepadReport(uint8_t const* data, uint16_t len,
                     HIDRawReportInfo const& info) {
  uint16_t off = (info.report_id != 0) ? 1 : 0;
  if (len < off + 2) {
    return;
  }

  uint16_t buttons = (uint16_t)data[off] | ((uint16_t)data[off + 1] << 8);
  if (!has_last_buttons) {
    Serial.printf("[GP] init button_bits=0x%04x (dev=%u inst=%u)\r\n", buttons,
                   info.dev_addr, info.instance);
    has_last_buttons = true;
    last_buttons = buttons;
    return;
  }

  printButtonDiff(last_buttons, buttons);
  last_buttons = buttons;
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(100); }
  Serial.println("HIDHost: gamepad buttons");
  Serial.println("Note: button decoding assumes first 16 bits are button flags.");

  HIDHost.begin();
  GamepadHost.onMount(onGamepadMount);
  GamepadHost.onUmount(onGamepadUmount);
  GamepadHost.onReport(onGamepadReport);

  USBHost.begin(0);
}

void loop() {
  USBHost.task();
}



