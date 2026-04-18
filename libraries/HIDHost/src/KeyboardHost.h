#ifndef HIDHOST_KEYBOARDHOST_H_
#define HIDHOST_KEYBOARDHOST_H_

#include <stddef.h>
#include <stdint.h>

enum class KeyboardLayout : uint8_t {
  US_ASCII = 0,
  JP_JIS = 1,
};

class KeyboardHostAPI {
public:
  typedef void (*MountCallback)(uint8_t dev_addr, uint8_t instance, uint16_t vid,
                                uint16_t pid);
  typedef void (*UmountCallback)(uint8_t dev_addr, uint8_t instance);
  typedef void (*KeyCallback)(uint8_t keycode, uint8_t modifiers);

  KeyboardHostAPI();

  bool begin();
  void end();

  bool mounted() const;
  bool mounted(uint8_t dev_addr, uint8_t instance) const;

  void onMount(MountCallback cb);
  void onUmount(UmountCallback cb);
  void onKeyDown(KeyCallback cb);
  void onKeyUp(KeyCallback cb);

  bool setLeds(uint8_t leds);
  bool setLeds(uint8_t dev_addr, uint8_t instance, uint8_t leds);

  int available();
  int peak();
  int read();
  size_t read(uint8_t* buf, size_t len);
  void flush();

  void setLayout(KeyboardLayout layout);
  void clearStateOnUmount(bool enabled = true);
};

extern KeyboardHostAPI KeyboardHost;

#endif // HIDHOST_KEYBOARDHOST_H_
