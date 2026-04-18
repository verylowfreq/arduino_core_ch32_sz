#ifndef HIDHOST_HIDGENERICHOST_H_
#define HIDHOST_HIDGENERICHOST_H_

#include <stdint.h>

class HIDGenericHostAPI {
public:
  typedef void (*MountCallback)(uint8_t dev_addr, uint8_t instance, uint16_t vid,
                                uint16_t pid);
  typedef void (*UmountCallback)(uint8_t dev_addr, uint8_t instance);
  typedef void (*ReportCallback)(uint8_t const* data, uint16_t len);
  typedef void (*FeatureCallback)(uint8_t const* data, uint16_t len);

  HIDGenericHostAPI();

  bool begin();
  void end();

  bool bind(uint8_t dev_addr, uint8_t instance);
  void unbind();
  bool bound() const;

  bool requestInputReport();
  bool sendReport(uint8_t const* data, uint16_t datalen, uint8_t report_id = 0);

  bool getFeatureReport(uint8_t report_id, uint8_t* data, uint16_t datalen);
  bool setFeatureReport(uint8_t report_id, uint8_t const* data, uint16_t datalen);

  void onMount(MountCallback cb);
  void onUmount(UmountCallback cb);
  void onReport(ReportCallback cb);
  void onFeatureReport(FeatureCallback cb);
};

extern HIDGenericHostAPI HIDGenericHost;

#endif // HIDHOST_HIDGENERICHOST_H_
