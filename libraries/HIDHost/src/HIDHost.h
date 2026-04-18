#ifndef HIDHOST_HIDHOST_H_
#define HIDHOST_HIDHOST_H_

#include <stdint.h>

#include "GamepadHost.h"
#include "HIDGenericHost.h"
#include "KeyboardHost.h"
#include "MouseHost.h"

class HIDHostAPI {
public:
  bool begin();
  void end();
  void task();

  KeyboardHostAPI& keyboard();
  MouseHostAPI& mouse();
  GamepadHostAPI& gamepad();
  HIDGenericHostAPI& generic();

  bool startAutoReceive(uint8_t dev_addr, uint8_t instance);
  bool stopAutoReceive(uint8_t dev_addr, uint8_t instance);
  bool isAutoReceiveEnabled(uint8_t dev_addr, uint8_t instance) const;

  void handleMount(uint8_t dev_addr, uint8_t instance, uint8_t const* report_desc,
                   uint16_t desc_len);
  void handleUmount(uint8_t dev_addr, uint8_t instance);
  void handleReport(uint8_t dev_addr, uint8_t instance, uint8_t const* report,
                    uint16_t len);
  void handleGetReportComplete(uint8_t dev_addr, uint8_t instance,
                               uint8_t report_id, uint8_t report_type,
                               uint16_t len);

private:
  friend class KeyboardHostAPI;
  friend class MouseHostAPI;
  friend class GamepadHostAPI;
  friend class HIDGenericHostAPI;
};

extern HIDHostAPI HIDHost;

#endif // HIDHOST_HIDHOST_H_
