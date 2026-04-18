#ifndef HIDHOST_TYPES_H_
#define HIDHOST_TYPES_H_

#include <stdint.h>

struct HIDDeviceRef {
  uint8_t dev_addr;
  uint8_t instance;
};

struct HIDRawReportInfo {
  uint16_t vid;
  uint16_t pid;
  uint8_t dev_addr;
  uint8_t instance;
  uint8_t report_id;
  uint16_t usage_page;
  uint16_t usage;
};

#endif // HIDHOST_TYPES_H_
