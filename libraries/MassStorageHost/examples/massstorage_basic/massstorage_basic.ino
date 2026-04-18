#include <Adafruit_TinyUSB.h>
#include <MassStorageHost.h>

Adafruit_USBH_Host USBHost;

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(100); }

  MassStorageHost.begin();

  MassStorageHost.onDeviceMount([](MassStorageDeviceInfo const& info) {
    Serial.printf("MSC device mount: addr=%u vid=0x%04x pid=0x%04x max_lun=%u\r\n",
                   info.dev_addr, info.vid, info.pid, info.max_lun);
  });

  MassStorageHost.onVolumeMount([](MassStorageVolumeInfo const& info) {
    Serial.printf("MSC volume mount: addr=%u lun=%u blocks=%lu size=%lu\r\n",
                   info.dev_addr, info.lun, info.block_count, info.block_size);
    MassStorageHost.ls(&Serial, LS_SIZE);
  });

  MassStorageHost.onVolumeUmount([](uint8_t dev_addr, uint8_t lun) {
    Serial.printf("MSC volume umount: addr=%u lun=%u\r\n", dev_addr, lun);
  });

  MassStorageHost.onDeviceUmount([](uint8_t dev_addr) {
    Serial.printf("MSC device umount: addr=%u\r\n", dev_addr);
  });

  USBHost.begin(0);
}

void loop() {
  USBHost.task();
}
