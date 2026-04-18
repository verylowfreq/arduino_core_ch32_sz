#ifndef SERIALHOST_SERIALHOST_H_
#define SERIALHOST_SERIALHOST_H_

#include <stddef.h>
#include <stdint.h>

#include <Arduino.h>
#include <Adafruit_TinyUSB.h>

struct SerialHostInterfaceInfo {
  uint8_t idx;
  uint8_t dev_addr;
  uint8_t itf_num;
  uint16_t vid;
  uint16_t pid;
  uint8_t subclass;
  uint8_t protocol;
};

struct SerialHostLineCoding {
  uint32_t bit_rate;
  uint8_t stop_bits;
  uint8_t parity;
  uint8_t data_bits;
};

class SerialHostAPI;

class SerialDevice {
public:
  SerialDevice();

  explicit operator bool() const;
  bool attached() const;

  bool operator==(SerialDevice const& rhs) const;
  bool operator!=(SerialDevice const& rhs) const;

  uint8_t index() const;
  bool info(SerialHostInterfaceInfo& out) const;

  bool connected() const;
  bool getLineCoding(SerialHostLineCoding& out) const;
  bool setLineCoding(SerialHostLineCoding const& coding) const;
  bool setBaudrate(uint32_t baudrate) const;
  bool setDtrRts(bool dtr, bool rts) const;
  bool connect() const;
  bool disconnect() const;

  int available() const;
  int peek() const;
  int read() const;
  size_t read(uint8_t* buffer, size_t size) const;
  void flush() const;
  size_t write(uint8_t ch) const;
  size_t write(uint8_t const* buffer, size_t size) const;
  int availableForWrite() const;
  bool clearRx() const;
  bool clearTx() const;

private:
  friend class SerialHostAPI;
  SerialDevice(SerialHostAPI* host, uint8_t slot, uint16_t generation);

  SerialHostAPI* _host;
  uint8_t _slot;
  uint16_t _generation;
};

class SerialHostAPI {
public:
  typedef void (*MountCallback)(SerialDevice const& dev,
                                SerialHostInterfaceInfo const& info);
  typedef void (*UmountCallback)(SerialDevice const& dev,
                                 SerialHostInterfaceInfo const& last_info);
  typedef void (*RxCallback)(SerialDevice const& dev, uint32_t available);
  typedef void (*TxCompleteCallback)(SerialDevice const& dev);
  typedef void (*ErrorCallback)(SerialDevice const& dev, int32_t code);

  SerialHostAPI();

  bool begin();
  void end();
  void task();

  SerialDevice findByAddress(uint8_t dev_addr, uint8_t itf_num) const;

  void onMount(MountCallback cb);
  void onUmount(UmountCallback cb);
  void onRx(RxCallback cb);
  void onTxComplete(TxCompleteCallback cb);
  void onError(ErrorCallback cb);

  void setAutoConnect(bool enabled);

  void handleMount(uint8_t idx);
  void handleUmount(uint8_t idx);
  void handleRx(uint8_t idx);
  void handleTxComplete(uint8_t idx);

private:
  friend class SerialDevice;
  friend void tuh_cdc_mount_cb(uint8_t idx);
  friend void tuh_cdc_umount_cb(uint8_t idx);
  friend void tuh_cdc_rx_cb(uint8_t idx);
  friend void tuh_cdc_tx_complete_cb(uint8_t idx);

  bool slotAttached(uint8_t slot, uint16_t generation) const;
  bool slotInfo(uint8_t slot, uint16_t generation, SerialHostInterfaceInfo& out) const;

  bool slotConnected(uint8_t slot, uint16_t generation) const;
  bool slotGetLineCoding(uint8_t slot, uint16_t generation, SerialHostLineCoding& out) const;
  bool slotSetLineCoding(uint8_t slot, uint16_t generation,
                         SerialHostLineCoding const& coding);
  bool slotSetBaudrate(uint8_t slot, uint16_t generation, uint32_t baudrate);
  bool slotSetDtrRts(uint8_t slot, uint16_t generation, bool dtr, bool rts);
  bool slotConnect(uint8_t slot, uint16_t generation);
  bool slotDisconnect(uint8_t slot, uint16_t generation);

  int slotAvailable(uint8_t slot, uint16_t generation) const;
  int slotPeek(uint8_t slot, uint16_t generation) const;
  int slotRead(uint8_t slot, uint16_t generation);
  size_t slotRead(uint8_t slot, uint16_t generation, uint8_t* buffer, size_t size);
  void slotFlush(uint8_t slot, uint16_t generation);
  size_t slotWrite(uint8_t slot, uint16_t generation, uint8_t ch);
  size_t slotWrite(uint8_t slot, uint16_t generation, uint8_t const* buffer,
                   size_t size);
  int slotAvailableForWrite(uint8_t slot, uint16_t generation) const;
  bool slotClearRx(uint8_t slot, uint16_t generation);
  bool slotClearTx(uint8_t slot, uint16_t generation);
};

extern SerialHostAPI SerialHost;

#endif // SERIALHOST_SERIALHOST_H_
