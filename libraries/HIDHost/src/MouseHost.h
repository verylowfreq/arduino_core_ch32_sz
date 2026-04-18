#ifndef HIDHOST_MOUSEHOST_H_
#define HIDHOST_MOUSEHOST_H_

#include <stdint.h>

class MouseHostAPI {
public:
  typedef void (*MountCallback)(uint8_t dev_addr, uint8_t instance, uint16_t vid,
                                uint16_t pid);
  typedef void (*UmountCallback)(uint8_t dev_addr, uint8_t instance);
  typedef void (*MoveCallback)(int16_t dx, int16_t dy, uint8_t buttons);
  typedef void (*WheelCallback)(int8_t delta);

  MouseHostAPI();

  bool begin();
  void end();

  bool mounted() const;
  bool mounted(uint8_t dev_addr, uint8_t instance) const;

  void onMount(MountCallback cb);
  void onUmount(UmountCallback cb);
  void onMove(MoveCallback cb);
  void onWheel(WheelCallback cb);
  void onHWheel(WheelCallback cb);
};

extern MouseHostAPI MouseHost;

#endif // HIDHOST_MOUSEHOST_H_
