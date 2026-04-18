#include <Adafruit_TinyUSB.h>
#include <MassStorageHost.h>

Adafruit_USBH_Host USBHost;

bool g_written = false;

void writeDemoFiles() {
  if (!MassStorageHost.mounted() || g_written) {
    return;
  }

  if (!MassStorageHost.exists("/demo")) {
    if (!MassStorageHost.mkdir("/demo", true)) {
      Serial.println("mkdir /demo failed");
      return;
    }
  }

  // Append a line to demo log.
  MscFile log_file =
      MassStorageHost.open("/demo/log.txt", O_CREAT | O_RDWR | O_APPEND);
  if (!log_file) {
    Serial.println("open /demo/log.txt failed");
    return;
  }
  log_file.printf("USB MSC log line, millis=%lu\r\n", (unsigned long) millis());
  log_file.close();

  // Create/overwrite a status file.
  MscFile status_file;
  if (!MassStorageHost.open(status_file, "/demo/status.txt",
                            O_CREAT | O_TRUNC | O_WRITE)) {
    Serial.println("open /demo/status.txt failed");
    return;
  }
  status_file.println("MassStorageHost create/write example");
  status_file.println("Files were generated on mount event.");
  status_file.close();

  Serial.println("File create/write completed");
  MassStorageHost.ls(&Serial1, LS_DATE | LS_SIZE);

  g_written = true;
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(100); }

  MassStorageHost.begin();

  MassStorageHost.onVolumeMount([](MassStorageVolumeInfo const& info) {
    Serial.printf("Volume mounted: addr=%u lun=%u blocks=%lu size=%lu\r\n",
                   info.dev_addr, info.lun, info.block_count, info.block_size);
    writeDemoFiles();
  });

  MassStorageHost.onVolumeUmount([](uint8_t dev_addr, uint8_t lun) {
    Serial.printf("Volume unmounted: addr=%u lun=%u\r\n", dev_addr, lun);
    g_written = false;
  });

  MassStorageHost.onError([](uint8_t dev_addr, uint8_t lun, int32_t code) {
    Serial.printf("MassStorageHost error: addr=%u lun=%u code=%ld\r\n", dev_addr,
                   lun, (long) code);
  });

  USBHost.begin(0);
}

void loop() {
  USBHost.task();
}
