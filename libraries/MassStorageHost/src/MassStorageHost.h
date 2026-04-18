#ifndef MASSSTORAGEHOST_MASSSTORAGEHOST_H_
#define MASSSTORAGEHOST_MASSSTORAGEHOST_H_

#include <stddef.h>
#include <stdint.h>

#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
// Adafruit SdFat library
#include "SdFat.h"

struct MassStorageDeviceInfo {
  uint8_t dev_addr;
  uint8_t max_lun;
  uint16_t vid;
  uint16_t pid;
};

struct MassStorageVolumeInfo {
  uint8_t dev_addr;
  uint8_t lun;
  uint32_t block_count;
  uint32_t block_size;
};

// Default to FAT-only types for smaller footprint.
// Define MASSSTORAGEHOST_USE_EXFAT=1 to use FsVolume/FsFile.
#ifndef MASSSTORAGEHOST_USE_EXFAT
#define MASSSTORAGEHOST_USE_EXFAT 0
#endif

#if MASSSTORAGEHOST_USE_EXFAT
typedef FsVolume MscVolume;
typedef FsFile MscFile;
#else
typedef FatVolume MscVolume;
typedef File32 MscFile;
#endif

class MassStorageHostAPI {
public:
  typedef void (*DeviceMountCallback)(MassStorageDeviceInfo const& info);
  typedef void (*DeviceUmountCallback)(uint8_t dev_addr);
  typedef void (*VolumeMountCallback)(MassStorageVolumeInfo const& info);
  typedef void (*VolumeUmountCallback)(uint8_t dev_addr, uint8_t lun);
  typedef void (*ErrorCallback)(uint8_t dev_addr, uint8_t lun, int32_t code);

  MassStorageHostAPI();

  bool begin();
  void end();
  void task();

  bool deviceMounted() const;
  bool deviceMounted(uint8_t dev_addr) const;

  bool mounted() const;
  bool mounted(uint8_t dev_addr, uint8_t lun) const;
  uint8_t activeDeviceAddress() const;
  uint8_t activeLun() const;

  bool bind(uint8_t dev_addr, uint8_t lun = 0);
  void unbind();
  bool remount();
  void setAutoMountLun0(bool enabled);

  bool getDeviceInfo(uint8_t dev_addr, MassStorageDeviceInfo& out) const;
  bool getVolumeInfo(uint8_t dev_addr, uint8_t lun,
                     MassStorageVolumeInfo& out) const;

  MscVolume* volume();
  bool exists(char const* path) const;
  bool mkdir(char const* path, bool p = true);
  bool remove(char const* path);
  bool rename(char const* old_path, char const* new_path);
  bool open(MscFile& out, char const* path, oflag_t oflag = O_RDONLY);
  MscFile open(char const* path, oflag_t oflag = O_RDONLY);
  void ls(Print* pr = &Serial, uint8_t flags = LS_DATE | LS_SIZE);

  void onDeviceMount(DeviceMountCallback cb);
  void onDeviceUmount(DeviceUmountCallback cb);
  void onVolumeMount(VolumeMountCallback cb);
  void onVolumeUmount(VolumeUmountCallback cb);
  void onError(ErrorCallback cb);

  void handleMount(uint8_t dev_addr);
  void handleUmount(uint8_t dev_addr);

private:
  friend void tuh_msc_mount_cb(uint8_t dev_addr);
  friend void tuh_msc_umount_cb(uint8_t dev_addr);
};

extern MassStorageHostAPI MassStorageHost;

#endif // MASSSTORAGEHOST_MASSSTORAGEHOST_H_
