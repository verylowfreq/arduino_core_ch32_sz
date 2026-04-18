/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 Ha Thach (tinyusb.org) for Adafruit Industries
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef MSH_USBH_BLOCKDEVICE_H_
#define MSH_USBH_BLOCKDEVICE_H_

#include "tusb.h"

#include "SdFat.h"

class MSH_USBH_BlockDevice : public FsBlockDeviceInterface {
public:
  MSH_USBH_BlockDevice();

  bool begin(uint8_t dev_addr);
  void end(void);

  bool mounted(void);

  bool setActiveLUN(uint8_t lun);
  void setWriteCompleteCallback(tuh_msc_complete_cb_t cb);

  //------------- SdFat v2 FsBlockDeviceInterface API -------------//
  virtual bool isBusy();
  virtual uint32_t sectorCount();
  virtual bool syncDevice();

  virtual bool readSector(uint32_t block, uint8_t* dst);
  virtual bool readSectors(uint32_t block, uint8_t* dst, size_t ns);
  virtual bool writeSector(uint32_t block, uint8_t const* src);
  virtual bool writeSectors(uint32_t block, uint8_t const* src, size_t ns);

  //------------- Internal APIs -------------//
  bool _io_complete_cb(uint8_t dev_addr, tuh_msc_complete_data_t const* cb_data);

private:
  uint8_t _daddr;
  uint8_t _lun;

  volatile bool _busy;
  tuh_msc_complete_cb_t _wr_cb;

  bool wait_for_io(void);
};

#endif
