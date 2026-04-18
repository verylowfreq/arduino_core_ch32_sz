#ifndef HIDHOST_GAMEPADHOST_H_
#define HIDHOST_GAMEPADHOST_H_

#include <stdint.h>

#include "HIDHostTypes.h"

class GamepadHostAPI {
public:
  typedef void (*MountCallback)(uint8_t dev_addr, uint8_t instance, uint16_t vid,
                                uint16_t pid);
  typedef void (*UmountCallback)(uint8_t dev_addr, uint8_t instance);
  typedef void (*ReportCallback)(uint8_t const* data, uint16_t len,
                                 HIDRawReportInfo const& info);

  GamepadHostAPI();

  bool begin();
  void end();

  bool mounted() const;
  bool mounted(uint8_t dev_addr, uint8_t instance) const;

  void onMount(MountCallback cb);
  void onUmount(UmountCallback cb);
  void onReport(ReportCallback cb);

  void setUsageFilter(uint16_t usage_page, uint16_t usage);
};

extern GamepadHostAPI GamepadHost;

#endif // HIDHOST_GAMEPADHOST_H_
