#include <Adafruit_TinyUSB.h>
#include <HIDHost.h>

#ifndef USE_TINYUSB_HOST
#error This example requires USB stack configured as host.
#endif

Adafruit_USBH_Host USBHost;

void onKbMount(uint8_t dev_addr, uint8_t instance, uint16_t vid, uint16_t pid) {
  Serial.printf("[KB] mount dev=%u inst=%u vid=%04x pid=%04x\r\n",
                 dev_addr, instance, vid, pid);
}

void onKbUmount(uint8_t dev_addr, uint8_t instance) {
  Serial.printf("[KB] umount dev=%u inst=%u\r\n", dev_addr, instance);
}

void onKeyDown(uint8_t keycode, uint8_t modifiers) {
  Serial.printf("[KB] down key=0x%02x mod=0x%02x\r\n", keycode, modifiers);
}

void onKeyUp(uint8_t keycode, uint8_t modifiers) {
  Serial.printf("[KB] up   key=0x%02x mod=0x%02x\r\n", keycode, modifiers);
}

void onMouseMount(uint8_t dev_addr, uint8_t instance, uint16_t vid,
                  uint16_t pid) {
  Serial.printf("[MS] mount dev=%u inst=%u vid=%04x pid=%04x\r\n",
                 dev_addr, instance, vid, pid);
}

void onMouseUmount(uint8_t dev_addr, uint8_t instance) {
  Serial.printf("[MS] umount dev=%u inst=%u\r\n", dev_addr, instance);
}

void onMouseMove(int16_t dx, int16_t dy, uint8_t buttons) {
  Serial.printf("[MS] move dx=%d dy=%d btn=0x%02x\r\n", dx, dy, buttons);
}

void onMouseWheel(int8_t v) { Serial.printf("[MS] wheel v=%d\r\n", (int)v); }

void onMouseHWheel(int8_t h) {
  Serial.printf("[MS] hwheel h=%d (non-standard boot mouse)\r\n", (int)h);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(100); }
  Serial.println("HIDHost: keyboard/mouse dump");

  HIDHost.begin();
  KeyboardHost.onMount(onKbMount);
  KeyboardHost.onUmount(onKbUmount);
  KeyboardHost.onKeyDown(onKeyDown);
  KeyboardHost.onKeyUp(onKeyUp);

  MouseHost.onMount(onMouseMount);
  MouseHost.onUmount(onMouseUmount);
  MouseHost.onMove(onMouseMove);
  MouseHost.onWheel(onMouseWheel);
  MouseHost.onHWheel(onMouseHWheel);

  USBHost.begin(0);
}

void loop() {
  USBHost.task();
}



