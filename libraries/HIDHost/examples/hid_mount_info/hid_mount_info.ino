#include <Adafruit_TinyUSB.h>
#include <HIDHost.h>

#ifndef USE_TINYUSB_HOST
#error This example requires USB stack configured as host.
#endif

Adafruit_USBH_Host USBHost;

void printProtocol(uint8_t dev_addr, uint8_t instance) {
  uint8_t p = tuh_hid_interface_protocol(dev_addr, instance);
  if (p == HID_ITF_PROTOCOL_KEYBOARD) {
    Serial.print("protocol=KEYBOARD");
  } else if (p == HID_ITF_PROTOCOL_MOUSE) {
    Serial.print("protocol=MOUSE");
  } else {
    Serial.print("protocol=NONE/REPORT");
  }
}

void onAnyMount(uint8_t dev_addr, uint8_t instance, uint16_t vid, uint16_t pid) {
  Serial.printf("[HID] mount dev=%u inst=%u vid=%04x pid=%04x ", dev_addr,
                 instance, vid, pid);
  printProtocol(dev_addr, instance);
  Serial.println();

  Serial.print("      type=");
  bool printed = false;
  if (KeyboardHost.mounted(dev_addr, instance)) {
    Serial.print("Keyboard");
    printed = true;
  }
  if (MouseHost.mounted(dev_addr, instance)) {
    if (printed) {
      Serial.print("|");
    }
    Serial.print("Mouse");
    printed = true;
  }
  if (GamepadHost.mounted(dev_addr, instance)) {
    if (printed) {
      Serial.print("|");
    }
    Serial.print("Gamepad");
    printed = true;
  }
  if (!printed) {
    Serial.print("GenericHID");
  }
  Serial.println();
}

void onAnyUmount(uint8_t dev_addr, uint8_t instance) {
  Serial.printf("[HID] umount dev=%u inst=%u\r\n", dev_addr, instance);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(100); }
  Serial.println("HIDHost: mount info / HID type");

  HIDHost.begin();
  HIDGenericHost.onMount(onAnyMount);
  HIDGenericHost.onUmount(onAnyUmount);

  USBHost.begin(0);
}

void loop() {
  USBHost.task();
}



