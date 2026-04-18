#include "SerialHost.h"

#include <string.h>

namespace {

#if defined(CFG_TUH_ENABLED) && CFG_TUH_ENABLED && defined(CFG_TUH_CDC) && CFG_TUH_CDC

enum {
  SH_OK = 0,
  SH_ERR_INFO = -1,
  SH_ERR_VID_PID = -2,
  SH_ERR_CONNECT = -3,
};

struct SerialSlotState {
  bool mounted;
  uint16_t generation;
  SerialHostInterfaceInfo info;
  int32_t last_error;
};

struct SerialHostState {
  bool enabled;
  bool auto_connect;
  SerialSlotState slots[CFG_TUH_CDC];

  SerialHostAPI::MountCallback mount_cb;
  SerialHostAPI::UmountCallback umount_cb;
  SerialHostAPI::RxCallback rx_cb;
  SerialHostAPI::TxCompleteCallback tx_complete_cb;
  SerialHostAPI::ErrorCallback error_cb;
};

SerialHostState& st() {
  static SerialHostState s;
  return s;
}

uint16_t next_generation(uint16_t cur) {
  cur = (uint16_t) (cur + 1u);
  if (cur == 0u) {
    cur = 1u;
  }
  return cur;
}

bool slot_valid_index(uint8_t slot) {
  return slot < CFG_TUH_CDC;
}

uint8_t slot_from_idx(uint8_t idx) {
  return idx;
}

void clear_slot(SerialSlotState& slot) {
  slot.mounted = false;
  slot.last_error = SH_OK;
  memset(&slot.info, 0, sizeof(slot.info));
  slot.generation = next_generation(slot.generation);
}

void emit_error(uint8_t slot, int32_t code) {
  SerialHostState& s = st();
  if (!slot_valid_index(slot)) {
    return;
  }

  s.slots[slot].last_error = code;
  if (s.error_cb != NULL) {
    // Error callback may happen before a fully attached SerialDevice handle exists.
    s.error_cb(SerialDevice(), code);
  }
}

#else

struct SerialHostState {
  bool enabled;
  bool auto_connect;

  SerialHostAPI::MountCallback mount_cb;
  SerialHostAPI::UmountCallback umount_cb;
  SerialHostAPI::RxCallback rx_cb;
  SerialHostAPI::TxCompleteCallback tx_complete_cb;
  SerialHostAPI::ErrorCallback error_cb;
};

SerialHostState& st() {
  static SerialHostState s;
  return s;
}

#endif

} // namespace

SerialDevice::SerialDevice() : _host(NULL), _slot(0xff), _generation(0) {}

SerialDevice::SerialDevice(SerialHostAPI* host, uint8_t slot, uint16_t generation)
    : _host(host), _slot(slot), _generation(generation) {}

SerialDevice::operator bool() const { return attached(); }

bool SerialDevice::attached() const {
  return (_host != NULL) && _host->slotAttached(_slot, _generation);
}

bool SerialDevice::operator==(SerialDevice const& rhs) const {
  return _host == rhs._host && _slot == rhs._slot && _generation == rhs._generation;
}

bool SerialDevice::operator!=(SerialDevice const& rhs) const { return !(*this == rhs); }

uint8_t SerialDevice::index() const {
  SerialHostInterfaceInfo i;
  return info(i) ? i.idx : 0xff;
}

bool SerialDevice::info(SerialHostInterfaceInfo& out) const {
  if (_host == NULL) {
    return false;
  }
  return _host->slotInfo(_slot, _generation, out);
}

bool SerialDevice::connected() const {
  if (_host == NULL) {
    return false;
  }
  return _host->slotConnected(_slot, _generation);
}

bool SerialDevice::getLineCoding(SerialHostLineCoding& out) const {
  if (_host == NULL) {
    return false;
  }
  return _host->slotGetLineCoding(_slot, _generation, out);
}

bool SerialDevice::setLineCoding(SerialHostLineCoding const& coding) const {
  if (_host == NULL) {
    return false;
  }
  return _host->slotSetLineCoding(_slot, _generation, coding);
}

bool SerialDevice::setBaudrate(uint32_t baudrate) const {
  if (_host == NULL) {
    return false;
  }
  return _host->slotSetBaudrate(_slot, _generation, baudrate);
}

bool SerialDevice::setDtrRts(bool dtr, bool rts) const {
  if (_host == NULL) {
    return false;
  }
  return _host->slotSetDtrRts(_slot, _generation, dtr, rts);
}

bool SerialDevice::connect() const {
  if (_host == NULL) {
    return false;
  }
  return _host->slotConnect(_slot, _generation);
}

bool SerialDevice::disconnect() const {
  if (_host == NULL) {
    return false;
  }
  return _host->slotDisconnect(_slot, _generation);
}

int SerialDevice::available() const {
  if (_host == NULL) {
    return 0;
  }
  return _host->slotAvailable(_slot, _generation);
}

int SerialDevice::peek() const {
  if (_host == NULL) {
    return -1;
  }
  return _host->slotPeek(_slot, _generation);
}

int SerialDevice::read() const {
  if (_host == NULL) {
    return -1;
  }
  return _host->slotRead(_slot, _generation);
}

size_t SerialDevice::read(uint8_t* buffer, size_t size) const {
  if (_host == NULL) {
    return 0;
  }
  return _host->slotRead(_slot, _generation, buffer, size);
}

void SerialDevice::flush() const {
  if (_host == NULL) {
    return;
  }
  _host->slotFlush(_slot, _generation);
}

size_t SerialDevice::write(uint8_t ch) const {
  if (_host == NULL) {
    return 0;
  }
  return _host->slotWrite(_slot, _generation, ch);
}

size_t SerialDevice::write(uint8_t const* buffer, size_t size) const {
  if (_host == NULL) {
    return 0;
  }
  return _host->slotWrite(_slot, _generation, buffer, size);
}

int SerialDevice::availableForWrite() const {
  if (_host == NULL) {
    return 0;
  }
  return _host->slotAvailableForWrite(_slot, _generation);
}

bool SerialDevice::clearRx() const {
  if (_host == NULL) {
    return false;
  }
  return _host->slotClearRx(_slot, _generation);
}

bool SerialDevice::clearTx() const {
  if (_host == NULL) {
    return false;
  }
  return _host->slotClearTx(_slot, _generation);
}

SerialHostAPI::SerialHostAPI() {}

bool SerialHostAPI::begin() {
  SerialHostState& s = st();
#if defined(CFG_TUH_ENABLED) && CFG_TUH_ENABLED && defined(CFG_TUH_CDC) && CFG_TUH_CDC
  for (uint8_t i = 0; i < CFG_TUH_CDC; i++) {
    if (s.slots[i].generation == 0u) {
      s.slots[i].generation = 1u;
    } else {
      s.slots[i].generation = next_generation(s.slots[i].generation);
    }
    s.slots[i].mounted = false;
    s.slots[i].last_error = SH_OK;
    memset(&s.slots[i].info, 0, sizeof(s.slots[i].info));
  }
#endif
  s.auto_connect = false;
  s.enabled = true;
  return true;
}

void SerialHostAPI::end() {
  SerialHostState& s = st();
  s.enabled = false;

#if defined(CFG_TUH_ENABLED) && CFG_TUH_ENABLED && defined(CFG_TUH_CDC) && CFG_TUH_CDC
  for (uint8_t i = 0; i < CFG_TUH_CDC; i++) {
    if (s.slots[i].generation == 0u) {
      s.slots[i].generation = 1u;
    }
    s.slots[i].mounted = false;
    memset(&s.slots[i].info, 0, sizeof(s.slots[i].info));
  }
#endif
}

void SerialHostAPI::task() {}

SerialDevice SerialHostAPI::findByAddress(uint8_t dev_addr, uint8_t itf_num) const {
#if defined(CFG_TUH_ENABLED) && CFG_TUH_ENABLED && defined(CFG_TUH_CDC) && CFG_TUH_CDC
  SerialHostState const& s = st();
  if (!s.enabled) {
    return SerialDevice();
  }

  for (uint8_t i = 0; i < CFG_TUH_CDC; i++) {
    SerialSlotState const& slot = s.slots[i];
    if (!slot.mounted) {
      continue;
    }
    if (slot.info.dev_addr == dev_addr && slot.info.itf_num == itf_num) {
      return SerialDevice((SerialHostAPI*) this, i, slot.generation);
    }
  }
#else
  (void) dev_addr;
  (void) itf_num;
#endif
  return SerialDevice();
}

void SerialHostAPI::onMount(MountCallback cb) { st().mount_cb = cb; }
void SerialHostAPI::onUmount(UmountCallback cb) { st().umount_cb = cb; }
void SerialHostAPI::onRx(RxCallback cb) { st().rx_cb = cb; }
void SerialHostAPI::onTxComplete(TxCompleteCallback cb) { st().tx_complete_cb = cb; }
void SerialHostAPI::onError(ErrorCallback cb) { st().error_cb = cb; }

void SerialHostAPI::setAutoConnect(bool enabled) { st().auto_connect = enabled; }

void SerialHostAPI::handleMount(uint8_t idx) {
#if defined(CFG_TUH_ENABLED) && CFG_TUH_ENABLED && defined(CFG_TUH_CDC) && CFG_TUH_CDC
  SerialHostState& s = st();
  if (!s.enabled) {
    return;
  }

  uint8_t slot_idx = slot_from_idx(idx);
  if (!slot_valid_index(slot_idx)) {
    return;
  }

  tuh_itf_info_t itf_info;
  if (!tuh_cdc_itf_get_info(idx, &itf_info)) {
    emit_error(slot_idx, SH_ERR_INFO);
    return;
  }

  SerialSlotState& slot = s.slots[slot_idx];
  slot.generation = next_generation(slot.generation);
  slot.mounted = true;
  slot.last_error = SH_OK;

  memset(&slot.info, 0, sizeof(slot.info));
  slot.info.idx = idx;
  slot.info.dev_addr = itf_info.daddr;
  slot.info.itf_num = itf_info.desc.bInterfaceNumber;
  slot.info.subclass = itf_info.desc.bInterfaceSubClass;
  slot.info.protocol = itf_info.desc.bInterfaceProtocol;

  if (!tuh_vid_pid_get(slot.info.dev_addr, &slot.info.vid, &slot.info.pid)) {
    emit_error(slot_idx, SH_ERR_VID_PID);
  }

  SerialDevice dev(this, slot_idx, slot.generation);

  if (s.mount_cb != NULL) {
    s.mount_cb(dev, slot.info);
  }

  if (s.auto_connect && !slotConnect(slot_idx, slot.generation)) {
    emit_error(slot_idx, SH_ERR_CONNECT);
  }
#else
  (void) idx;
#endif
}

void SerialHostAPI::handleUmount(uint8_t idx) {
#if defined(CFG_TUH_ENABLED) && CFG_TUH_ENABLED && defined(CFG_TUH_CDC) && CFG_TUH_CDC
  SerialHostState& s = st();
  uint8_t slot_idx = slot_from_idx(idx);
  if (!slot_valid_index(slot_idx)) {
    return;
  }

  SerialSlotState& slot = s.slots[slot_idx];
  if (!slot.mounted) {
    return;
  }

  SerialDevice dev(this, slot_idx, slot.generation);
  SerialHostInterfaceInfo last_info = slot.info;

  if (s.umount_cb != NULL) {
    s.umount_cb(dev, last_info);
  }

  clear_slot(slot);
#else
  (void) idx;
#endif
}

void SerialHostAPI::handleRx(uint8_t idx) {
#if defined(CFG_TUH_ENABLED) && CFG_TUH_ENABLED && defined(CFG_TUH_CDC) && CFG_TUH_CDC
  SerialHostState& s = st();
  uint8_t slot_idx = slot_from_idx(idx);
  if (!slot_valid_index(slot_idx)) {
    return;
  }

  SerialSlotState& slot = s.slots[slot_idx];
  if (!slot.mounted || s.rx_cb == NULL) {
    return;
  }

  SerialDevice dev(this, slot_idx, slot.generation);
  s.rx_cb(dev, tuh_cdc_read_available(idx));
#else
  (void) idx;
#endif
}

void SerialHostAPI::handleTxComplete(uint8_t idx) {
#if defined(CFG_TUH_ENABLED) && CFG_TUH_ENABLED && defined(CFG_TUH_CDC) && CFG_TUH_CDC
  SerialHostState& s = st();
  uint8_t slot_idx = slot_from_idx(idx);
  if (!slot_valid_index(slot_idx)) {
    return;
  }

  SerialSlotState& slot = s.slots[slot_idx];
  if (!slot.mounted || s.tx_complete_cb == NULL) {
    return;
  }

  SerialDevice dev(this, slot_idx, slot.generation);
  s.tx_complete_cb(dev);
#else
  (void) idx;
#endif
}

bool SerialHostAPI::slotAttached(uint8_t slot, uint16_t generation) const {
#if defined(CFG_TUH_ENABLED) && CFG_TUH_ENABLED && defined(CFG_TUH_CDC) && CFG_TUH_CDC
  SerialHostState const& s = st();
  if (!s.enabled || !slot_valid_index(slot)) {
    return false;
  }

  SerialSlotState const& cur = s.slots[slot];
  return cur.mounted && cur.generation == generation;
#else
  (void) slot;
  (void) generation;
  return false;
#endif
}

bool SerialHostAPI::slotInfo(uint8_t slot, uint16_t generation,
                             SerialHostInterfaceInfo& out) const {
#if defined(CFG_TUH_ENABLED) && CFG_TUH_ENABLED && defined(CFG_TUH_CDC) && CFG_TUH_CDC
  if (!slotAttached(slot, generation)) {
    return false;
  }
  out = st().slots[slot].info;
  return true;
#else
  (void) slot;
  (void) generation;
  (void) out;
  return false;
#endif
}

bool SerialHostAPI::slotConnected(uint8_t slot, uint16_t generation) const {
#if defined(CFG_TUH_ENABLED) && CFG_TUH_ENABLED && defined(CFG_TUH_CDC) && CFG_TUH_CDC
  if (!slotAttached(slot, generation)) {
    return false;
  }
  return tuh_cdc_connected(st().slots[slot].info.idx);
#else
  (void) slot;
  (void) generation;
  return false;
#endif
}

bool SerialHostAPI::slotGetLineCoding(uint8_t slot, uint16_t generation,
                                      SerialHostLineCoding& out) const {
#if defined(CFG_TUH_ENABLED) && CFG_TUH_ENABLED && defined(CFG_TUH_CDC) && CFG_TUH_CDC
  if (!slotAttached(slot, generation)) {
    return false;
  }

  cdc_line_coding_t lc;
  if (!tuh_cdc_get_local_line_coding(st().slots[slot].info.idx, &lc)) {
    return false;
  }

  out.bit_rate = lc.bit_rate;
  out.stop_bits = lc.stop_bits;
  out.parity = lc.parity;
  out.data_bits = lc.data_bits;
  return true;
#else
  (void) slot;
  (void) generation;
  (void) out;
  return false;
#endif
}

bool SerialHostAPI::slotSetLineCoding(uint8_t slot, uint16_t generation,
                                      SerialHostLineCoding const& coding) {
#if defined(CFG_TUH_ENABLED) && CFG_TUH_ENABLED && defined(CFG_TUH_CDC) && CFG_TUH_CDC
  if (!slotAttached(slot, generation)) {
    return false;
  }

  cdc_line_coding_t lc;
  lc.bit_rate = coding.bit_rate;
  lc.stop_bits = coding.stop_bits;
  lc.parity = coding.parity;
  lc.data_bits = coding.data_bits;

  return tuh_cdc_set_line_coding(st().slots[slot].info.idx, &lc, NULL, 0);
#else
  (void) slot;
  (void) generation;
  (void) coding;
  return false;
#endif
}

bool SerialHostAPI::slotSetBaudrate(uint8_t slot, uint16_t generation,
                                    uint32_t baudrate) {
#if defined(CFG_TUH_ENABLED) && CFG_TUH_ENABLED && defined(CFG_TUH_CDC) && CFG_TUH_CDC
  if (!slotAttached(slot, generation)) {
    return false;
  }
  return tuh_cdc_set_baudrate(st().slots[slot].info.idx, baudrate, NULL, 0);
#else
  (void) slot;
  (void) generation;
  (void) baudrate;
  return false;
#endif
}

bool SerialHostAPI::slotSetDtrRts(uint8_t slot, uint16_t generation, bool dtr,
                                  bool rts) {
#if defined(CFG_TUH_ENABLED) && CFG_TUH_ENABLED && defined(CFG_TUH_CDC) && CFG_TUH_CDC
  if (!slotAttached(slot, generation)) {
    return false;
  }

  uint16_t line_state = 0;
  if (dtr) {
    line_state |= CDC_CONTROL_LINE_STATE_DTR;
  }
  if (rts) {
    line_state |= CDC_CONTROL_LINE_STATE_RTS;
  }

  return tuh_cdc_set_control_line_state(st().slots[slot].info.idx, line_state,
                                        NULL, 0);
#else
  (void) slot;
  (void) generation;
  (void) dtr;
  (void) rts;
  return false;
#endif
}

bool SerialHostAPI::slotConnect(uint8_t slot, uint16_t generation) {
#if defined(CFG_TUH_ENABLED) && CFG_TUH_ENABLED && defined(CFG_TUH_CDC) && CFG_TUH_CDC
  if (!slotAttached(slot, generation)) {
    return false;
  }
  return tuh_cdc_connect(st().slots[slot].info.idx, NULL, 0);
#else
  (void) slot;
  (void) generation;
  return false;
#endif
}

bool SerialHostAPI::slotDisconnect(uint8_t slot, uint16_t generation) {
#if defined(CFG_TUH_ENABLED) && CFG_TUH_ENABLED && defined(CFG_TUH_CDC) && CFG_TUH_CDC
  if (!slotAttached(slot, generation)) {
    return false;
  }
  return tuh_cdc_disconnect(st().slots[slot].info.idx, NULL, 0);
#else
  (void) slot;
  (void) generation;
  return false;
#endif
}

int SerialHostAPI::slotAvailable(uint8_t slot, uint16_t generation) const {
#if defined(CFG_TUH_ENABLED) && CFG_TUH_ENABLED && defined(CFG_TUH_CDC) && CFG_TUH_CDC
  if (!slotAttached(slot, generation)) {
    return 0;
  }
  return (int) tuh_cdc_read_available(st().slots[slot].info.idx);
#else
  (void) slot;
  (void) generation;
  return 0;
#endif
}

int SerialHostAPI::slotPeek(uint8_t slot, uint16_t generation) const {
#if defined(CFG_TUH_ENABLED) && CFG_TUH_ENABLED && defined(CFG_TUH_CDC) && CFG_TUH_CDC
  if (!slotAttached(slot, generation)) {
    return -1;
  }

  uint8_t ch;
  return tuh_cdc_peek(st().slots[slot].info.idx, &ch) ? (int) ch : -1;
#else
  (void) slot;
  (void) generation;
  return -1;
#endif
}

int SerialHostAPI::slotRead(uint8_t slot, uint16_t generation) {
#if defined(CFG_TUH_ENABLED) && CFG_TUH_ENABLED && defined(CFG_TUH_CDC) && CFG_TUH_CDC
  if (!slotAttached(slot, generation)) {
    return -1;
  }

  uint8_t ch;
  return (tuh_cdc_read(st().slots[slot].info.idx, &ch, 1) == 1) ? (int) ch : -1;
#else
  (void) slot;
  (void) generation;
  return -1;
#endif
}

size_t SerialHostAPI::slotRead(uint8_t slot, uint16_t generation,
                               uint8_t* buffer, size_t size) {
#if defined(CFG_TUH_ENABLED) && CFG_TUH_ENABLED && defined(CFG_TUH_CDC) && CFG_TUH_CDC
  if (!slotAttached(slot, generation) || buffer == NULL || size == 0) {
    return 0;
  }
  return tuh_cdc_read(st().slots[slot].info.idx, buffer, size);
#else
  (void) slot;
  (void) generation;
  (void) buffer;
  (void) size;
  return 0;
#endif
}

void SerialHostAPI::slotFlush(uint8_t slot, uint16_t generation) {
#if defined(CFG_TUH_ENABLED) && CFG_TUH_ENABLED && defined(CFG_TUH_CDC) && CFG_TUH_CDC
  if (!slotAttached(slot, generation)) {
    return;
  }
  (void) tuh_cdc_write_flush(st().slots[slot].info.idx);
#else
  (void) slot;
  (void) generation;
#endif
}

size_t SerialHostAPI::slotWrite(uint8_t slot, uint16_t generation, uint8_t ch) {
  return slotWrite(slot, generation, &ch, 1);
}

size_t SerialHostAPI::slotWrite(uint8_t slot, uint16_t generation,
                                uint8_t const* buffer, size_t size) {
#if defined(CFG_TUH_ENABLED) && CFG_TUH_ENABLED && defined(CFG_TUH_CDC) && CFG_TUH_CDC
  if (!slotAttached(slot, generation) || buffer == NULL || size == 0) {
    return 0;
  }
  return tuh_cdc_write(st().slots[slot].info.idx, buffer, size);
#else
  (void) slot;
  (void) generation;
  (void) buffer;
  (void) size;
  return 0;
#endif
}

int SerialHostAPI::slotAvailableForWrite(uint8_t slot, uint16_t generation) const {
#if defined(CFG_TUH_ENABLED) && CFG_TUH_ENABLED && defined(CFG_TUH_CDC) && CFG_TUH_CDC
  if (!slotAttached(slot, generation)) {
    return 0;
  }
  return (int) tuh_cdc_write_available(st().slots[slot].info.idx);
#else
  (void) slot;
  (void) generation;
  return 0;
#endif
}

bool SerialHostAPI::slotClearRx(uint8_t slot, uint16_t generation) {
#if defined(CFG_TUH_ENABLED) && CFG_TUH_ENABLED && defined(CFG_TUH_CDC) && CFG_TUH_CDC
  if (!slotAttached(slot, generation)) {
    return false;
  }
  return tuh_cdc_read_clear(st().slots[slot].info.idx);
#else
  (void) slot;
  (void) generation;
  return false;
#endif
}

bool SerialHostAPI::slotClearTx(uint8_t slot, uint16_t generation) {
#if defined(CFG_TUH_ENABLED) && CFG_TUH_ENABLED && defined(CFG_TUH_CDC) && CFG_TUH_CDC
  if (!slotAttached(slot, generation)) {
    return false;
  }
  return tuh_cdc_write_clear(st().slots[slot].info.idx);
#else
  (void) slot;
  (void) generation;
  return false;
#endif
}

#if defined(CFG_TUH_ENABLED) && CFG_TUH_ENABLED && defined(CFG_TUH_CDC) && CFG_TUH_CDC
extern "C" {

void tuh_cdc_mount_cb(uint8_t idx) { SerialHost.handleMount(idx); }

void tuh_cdc_umount_cb(uint8_t idx) { SerialHost.handleUmount(idx); }

void tuh_cdc_rx_cb(uint8_t idx) { SerialHost.handleRx(idx); }

void tuh_cdc_tx_complete_cb(uint8_t idx) { SerialHost.handleTxComplete(idx); }

}
#endif

SerialHostAPI SerialHost;
