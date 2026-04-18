#include "MassStorageHost.h"
#include "MSH_USBH_BlockDevice.h"

#include <string.h>

#if CFG_TUH_ENABLED && CFG_TUH_MSC

namespace {

enum {
  MSC_ERROR_NOT_READY = -1,
  MSC_ERROR_DEVICE_NOT_MOUNTED = -2,
  MSC_ERROR_BIND_FAILED = -3,
  MSC_ERROR_FAT_MOUNT_FAILED = -4
};

struct DeviceSlot {
  bool used;
  MassStorageDeviceInfo info;
};

struct MassStorageHostState {
  bool enabled;
  bool auto_mount_lun0;

  bool active_mounted;
  uint8_t active_dev_addr;
  uint8_t active_lun;

  DeviceSlot devices[CFG_TUH_DEVICE_MAX];

  MSH_USBH_BlockDevice block_dev;
  MscVolume fatfs;

  MassStorageHostAPI::DeviceMountCallback device_mount_cb;
  MassStorageHostAPI::DeviceUmountCallback device_umount_cb;
  MassStorageHostAPI::VolumeMountCallback volume_mount_cb;
  MassStorageHostAPI::VolumeUmountCallback volume_umount_cb;
  MassStorageHostAPI::ErrorCallback error_cb;
};

MassStorageHostState& st() {
  static MassStorageHostState s;
  return s;
}

void emit_error(uint8_t dev_addr, uint8_t lun, int32_t code) {
  MassStorageHostState& s = st();
  if (s.error_cb != NULL) {
    s.error_cb(dev_addr, lun, code);
  }
}

DeviceSlot* find_device_slot(uint8_t dev_addr) {
  MassStorageHostState& s = st();
  for (size_t i = 0; i < CFG_TUH_DEVICE_MAX; i++) {
    DeviceSlot& slot = s.devices[i];
    if (slot.used && slot.info.dev_addr == dev_addr) {
      return &slot;
    }
  }
  return NULL;
}

DeviceSlot* alloc_device_slot(uint8_t dev_addr) {
  DeviceSlot* existing = find_device_slot(dev_addr);
  if (existing != NULL) {
    return existing;
  }

  MassStorageHostState& s = st();
  for (size_t i = 0; i < CFG_TUH_DEVICE_MAX; i++) {
    DeviceSlot& slot = s.devices[i];
    if (!slot.used) {
      memset(&slot, 0, sizeof(slot));
      slot.used = true;
      slot.info.dev_addr = dev_addr;
      return &slot;
    }
  }

  return NULL;
}

bool volume_unmount_internal(bool emit_callback) {
  MassStorageHostState& s = st();
  if (!s.active_mounted) {
    return true;
  }

  uint8_t const dev_addr = s.active_dev_addr;
  uint8_t const lun = s.active_lun;

  s.fatfs.end();
  s.block_dev.end();
  s.active_mounted = false;
  s.active_dev_addr = 0;
  s.active_lun = 0;

  if (emit_callback && s.volume_umount_cb != NULL) {
    s.volume_umount_cb(dev_addr, lun);
  }

  return true;
}

bool volume_mount_internal(uint8_t dev_addr, uint8_t lun, bool emit_callback) {
  MassStorageHostState& s = st();

  if (find_device_slot(dev_addr) == NULL) {
    emit_error(dev_addr, lun, MSC_ERROR_DEVICE_NOT_MOUNTED);
    return false;
  }

  if (!s.block_dev.begin(dev_addr)) {
    emit_error(dev_addr, lun, MSC_ERROR_BIND_FAILED);
    return false;
  }

  if (!s.block_dev.setActiveLUN(lun)) {
    s.block_dev.end();
    emit_error(dev_addr, lun, MSC_ERROR_BIND_FAILED);
    return false;
  }

  if (!s.fatfs.begin(&s.block_dev)) {
    s.block_dev.end();
    emit_error(dev_addr, lun, MSC_ERROR_FAT_MOUNT_FAILED);
    return false;
  }

  s.active_mounted = true;
  s.active_dev_addr = dev_addr;
  s.active_lun = lun;

  if (emit_callback && s.volume_mount_cb != NULL) {
    MassStorageVolumeInfo info;
    info.dev_addr = dev_addr;
    info.lun = lun;
    info.block_count = tuh_msc_get_block_count(dev_addr, lun);
    info.block_size = tuh_msc_get_block_size(dev_addr, lun);
    s.volume_mount_cb(info);
  }

  return true;
}

} // namespace

MassStorageHostAPI::MassStorageHostAPI() {}

bool MassStorageHostAPI::begin() {
  MassStorageHostState& s = st();
  memset(s.devices, 0, sizeof(s.devices));
  s.auto_mount_lun0 = true;
  s.active_mounted = false;
  s.active_dev_addr = 0;
  s.active_lun = 0;
  s.enabled = true;
  return true;
}

void MassStorageHostAPI::end() {
  MassStorageHostState& s = st();
  (void) volume_unmount_internal(false);
  memset(s.devices, 0, sizeof(s.devices));
  s.enabled = false;
}

void MassStorageHostAPI::task() {}

bool MassStorageHostAPI::deviceMounted() const {
  MassStorageHostState& s = st();
  for (size_t i = 0; i < CFG_TUH_DEVICE_MAX; i++) {
    if (s.devices[i].used) {
      return true;
    }
  }
  return false;
}

bool MassStorageHostAPI::deviceMounted(uint8_t dev_addr) const {
  return find_device_slot(dev_addr) != NULL;
}

bool MassStorageHostAPI::mounted() const { return st().active_mounted; }

bool MassStorageHostAPI::mounted(uint8_t dev_addr, uint8_t lun) const {
  MassStorageHostState& s = st();
  return s.active_mounted && s.active_dev_addr == dev_addr && s.active_lun == lun;
}

uint8_t MassStorageHostAPI::activeDeviceAddress() const {
  return st().active_mounted ? st().active_dev_addr : 0;
}

uint8_t MassStorageHostAPI::activeLun() const {
  return st().active_mounted ? st().active_lun : 0;
}

bool MassStorageHostAPI::bind(uint8_t dev_addr, uint8_t lun) {
  MassStorageHostState& s = st();

  if (!s.enabled) {
    emit_error(dev_addr, lun, MSC_ERROR_NOT_READY);
    return false;
  }

  if (s.active_mounted && s.active_dev_addr == dev_addr && s.active_lun == lun) {
    return true;
  }

  if (s.active_mounted) {
    (void) volume_unmount_internal(true);
  }

  return volume_mount_internal(dev_addr, lun, true);
}

void MassStorageHostAPI::unbind() { (void) volume_unmount_internal(true); }

bool MassStorageHostAPI::remount() {
  MassStorageHostState& s = st();
  if (!s.enabled || !s.active_mounted) {
    emit_error(s.active_dev_addr, s.active_lun, MSC_ERROR_NOT_READY);
    return false;
  }

  uint8_t const dev_addr = s.active_dev_addr;
  uint8_t const lun = s.active_lun;

  (void) volume_unmount_internal(true);
  return volume_mount_internal(dev_addr, lun, true);
}

void MassStorageHostAPI::setAutoMountLun0(bool enabled) {
  st().auto_mount_lun0 = enabled;
}

bool MassStorageHostAPI::getDeviceInfo(uint8_t dev_addr,
                                       MassStorageDeviceInfo& out) const {
  DeviceSlot* slot = find_device_slot(dev_addr);
  if (slot == NULL) {
    return false;
  }
  out = slot->info;
  return true;
}

bool MassStorageHostAPI::getVolumeInfo(uint8_t dev_addr, uint8_t lun,
                                       MassStorageVolumeInfo& out) const {
  if (find_device_slot(dev_addr) == NULL) {
    return false;
  }

  out.dev_addr = dev_addr;
  out.lun = lun;
  out.block_count = tuh_msc_get_block_count(dev_addr, lun);
  out.block_size = tuh_msc_get_block_size(dev_addr, lun);
  return true;
}

MscVolume* MassStorageHostAPI::volume() {
  return st().active_mounted ? &st().fatfs : NULL;
}

bool MassStorageHostAPI::exists(char const* path) const {
  MassStorageHostState& s = st();
  if (!s.active_mounted || path == NULL) {
    return false;
  }
  return const_cast<MscVolume&>(s.fatfs).exists(path);
}

bool MassStorageHostAPI::mkdir(char const* path, bool p) {
  MassStorageHostState& s = st();
  if (!s.active_mounted || path == NULL) {
    return false;
  }
  return s.fatfs.mkdir(path, p);
}

bool MassStorageHostAPI::remove(char const* path) {
  MassStorageHostState& s = st();
  if (!s.active_mounted || path == NULL) {
    return false;
  }
  return s.fatfs.remove(path);
}

bool MassStorageHostAPI::rename(char const* old_path, char const* new_path) {
  MassStorageHostState& s = st();
  if (!s.active_mounted || old_path == NULL || new_path == NULL) {
    return false;
  }
  return s.fatfs.rename(old_path, new_path);
}

bool MassStorageHostAPI::open(MscFile& out, char const* path, oflag_t oflag) {
  out = MscFile();
  MassStorageHostState& s = st();
  if (!s.active_mounted || path == NULL) {
    return false;
  }

  out = s.fatfs.open(path, oflag);
  return (bool) out;
}

MscFile MassStorageHostAPI::open(char const* path, oflag_t oflag) {
  MassStorageHostState& s = st();
  if (!s.active_mounted || path == NULL) {
    return MscFile();
  }
  return s.fatfs.open(path, oflag);
}

void MassStorageHostAPI::ls(Print* pr, uint8_t flags) {
  MassStorageHostState& s = st();
  if (!s.active_mounted) {
    return;
  }

  if (pr == NULL) {
    pr = &Serial;
  }
  s.fatfs.ls(pr, flags);
}

void MassStorageHostAPI::onDeviceMount(DeviceMountCallback cb) {
  st().device_mount_cb = cb;
}

void MassStorageHostAPI::onDeviceUmount(DeviceUmountCallback cb) {
  st().device_umount_cb = cb;
}

void MassStorageHostAPI::onVolumeMount(VolumeMountCallback cb) {
  st().volume_mount_cb = cb;
}

void MassStorageHostAPI::onVolumeUmount(VolumeUmountCallback cb) {
  st().volume_umount_cb = cb;
}

void MassStorageHostAPI::onError(ErrorCallback cb) { st().error_cb = cb; }

void MassStorageHostAPI::handleMount(uint8_t dev_addr) {
  MassStorageHostState& s = st();
  if (!s.enabled) {
    return;
  }

  DeviceSlot* slot = alloc_device_slot(dev_addr);
  if (slot == NULL) {
    emit_error(dev_addr, 0, MSC_ERROR_BIND_FAILED);
    return;
  }

  MassStorageDeviceInfo& info = slot->info;
  info.dev_addr = dev_addr;
  info.max_lun = tuh_msc_get_maxlun(dev_addr);
  tuh_vid_pid_get(dev_addr, &info.vid, &info.pid);

  if (s.device_mount_cb != NULL) {
    s.device_mount_cb(info);
  }

  if (s.auto_mount_lun0 && !s.active_mounted) {
    (void) volume_mount_internal(dev_addr, 0, true);
  }
}

void MassStorageHostAPI::handleUmount(uint8_t dev_addr) {
  MassStorageHostState& s = st();

  if (s.active_mounted && s.active_dev_addr == dev_addr) {
    (void) volume_unmount_internal(true);
  }

  DeviceSlot* slot = find_device_slot(dev_addr);
  if (slot != NULL) {
    memset(slot, 0, sizeof(*slot));
  }

  if (s.device_umount_cb != NULL) {
    s.device_umount_cb(dev_addr);
  }
}

extern "C" {

void tuh_msc_mount_cb(uint8_t dev_addr) { MassStorageHost.handleMount(dev_addr); }

void tuh_msc_umount_cb(uint8_t dev_addr) { MassStorageHost.handleUmount(dev_addr); }

} // extern "C"

#else

MassStorageHostAPI::MassStorageHostAPI() {}
bool MassStorageHostAPI::begin() { return false; }
void MassStorageHostAPI::end() {}
void MassStorageHostAPI::task() {}
bool MassStorageHostAPI::deviceMounted() const { return false; }
bool MassStorageHostAPI::deviceMounted(uint8_t) const { return false; }
bool MassStorageHostAPI::mounted() const { return false; }
bool MassStorageHostAPI::mounted(uint8_t, uint8_t) const { return false; }
uint8_t MassStorageHostAPI::activeDeviceAddress() const { return 0; }
uint8_t MassStorageHostAPI::activeLun() const { return 0; }
bool MassStorageHostAPI::bind(uint8_t, uint8_t) { return false; }
void MassStorageHostAPI::unbind() {}
bool MassStorageHostAPI::remount() { return false; }
void MassStorageHostAPI::setAutoMountLun0(bool) {}
bool MassStorageHostAPI::getDeviceInfo(uint8_t, MassStorageDeviceInfo&) const {
  return false;
}
bool MassStorageHostAPI::getVolumeInfo(uint8_t, uint8_t,
                                       MassStorageVolumeInfo&) const {
  return false;
}
MscVolume* MassStorageHostAPI::volume() { return NULL; }
bool MassStorageHostAPI::exists(char const*) const { return false; }
bool MassStorageHostAPI::mkdir(char const*, bool) { return false; }
bool MassStorageHostAPI::remove(char const*) { return false; }
bool MassStorageHostAPI::rename(char const*, char const*) { return false; }
bool MassStorageHostAPI::open(MscFile& out, char const*, oflag_t) {
  out = MscFile();
  return false;
}
MscFile MassStorageHostAPI::open(char const*, oflag_t) { return MscFile(); }
void MassStorageHostAPI::ls(Print*, uint8_t) {}
void MassStorageHostAPI::onDeviceMount(DeviceMountCallback) {}
void MassStorageHostAPI::onDeviceUmount(DeviceUmountCallback) {}
void MassStorageHostAPI::onVolumeMount(VolumeMountCallback) {}
void MassStorageHostAPI::onVolumeUmount(VolumeUmountCallback) {}
void MassStorageHostAPI::onError(ErrorCallback) {}
void MassStorageHostAPI::handleMount(uint8_t) {}
void MassStorageHostAPI::handleUmount(uint8_t) {}

#endif

MassStorageHostAPI MassStorageHost;
