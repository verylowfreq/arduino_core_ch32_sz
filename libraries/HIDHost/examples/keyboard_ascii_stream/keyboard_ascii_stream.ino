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

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(100); }
  Serial.println("HIDHost: keyboard ascii stream");
  Serial.println("Type on a USB keyboard JP-layout. ASCII comes from KeyboardHost.read().");

  HIDHost.begin();

  KeyboardHost.onMount(onKbMount);
  KeyboardHost.onUmount(onKbUmount);
  KeyboardHost.setLayout(KeyboardLayout::JP_JIS); // or KeyboardLayout::US_ASCII

  USBHost.begin(0);
}

void loop() {
  USBHost.task();

  while (KeyboardHost.available() > 0) {
    int ch = KeyboardHost.read();
    if (ch < 0) {
      break;
    }

    if (ch == '\n') {
      Serial.print("\\n");
    } else if (ch == '\r') {
      Serial.print("\\r");
    } else if (ch == '\t') {
      Serial.print("\\t");
    } else if (ch == '\b') {
      Serial.print("\\b");
    } else {
      Serial.write((char)ch);
    }

    Serial.printf(" [0x%02X]\r\n", (uint8_t)ch);
  }
}

