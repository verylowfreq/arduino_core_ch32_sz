/*
  EEPROM - Enables reading and writing to non-volatile storage in the processor.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#if defined(ARDUINO_ARCH_CH32) && defined(CH32V20x)

#include <EEPROM.h>

#include <ch32v20x_flash.h>

static const uint32_t FLASH_PAGE_SIZE = 256;
static const uint32_t EEPROM_SIZE = 512;
static const uint32_t EEPROM_AREA_ADDRESS = 0x08000000 + 224 * 1024 - EEPROM_SIZE;

alignas(4) static uint8_t buffer[EEPROM_SIZE];


EEPROMClass::EEPROMClass(void) {
}

EEPROMClass::~EEPROMClass() {
  end();
}


void EEPROMClass::begin(void)
{
  _size = EEPROM_SIZE;
  _dirty = false;

  // Allocate data buffer and copy the current content from storage
  _data = &buffer[0];
  memcpy(_data, (void*)EEPROM_AREA_ADDRESS, _size);
}


uint8_t * EEPROMClass::getDataPtr() {
  _dirty = true;
  return &_data[0];
}

uint8_t const * EEPROMClass::getConstDataPtr() const {
  return &_data[0];
}

uint8_t EEPROMClass::read(int const idx) {
  if(_data && idx>=0 && (size_t)idx<_size)
    return(_data[idx]);
  return(0);
}

void EEPROMClass::write(int const idx, uint8_t  const val) {
  if(_data && idx>=0 && (size_t)idx<_size) {
    _dirty = true;
    _data[idx]=val;
  }
}

void EEPROMClass::erase(void) {
  _dirty = true;
  for (size_t i=0;i<_size; i++)
    _data[i]=0xFF;
}

static void loopdelay(uint32_t count) {
  count /= 3;
  for (volatile uint32_t i = 0; i < count; i++) {
    __asm__ volatile ("nop");
  }
}

bool EEPROMClass::commit()
{
  if(!_dirty)
    return(true);

  __disable_irq();

  FLASH_Unlock_Fast();
  FLASH_Enhance_Mode(DISABLE);
  // delay(1);
  loopdelay(SystemCoreClock / 1000);

  for (int i = 0; i < EEPROM_SIZE / FLASH_PAGE_SIZE; i++) {
    uint32_t offset = i * FLASH_PAGE_SIZE;
    uint32_t flash_addr = EEPROM_AREA_ADDRESS + offset;
    uint32_t const* dataptr = (uint32_t*)&_data[offset];
    FLASH_ErasePage_Fast(flash_addr);
    FLASH_ProgramPage_Fast(flash_addr, dataptr);
  }

  FLASH_Enhance_Mode(ENABLE);
  // delay(1);
  loopdelay(SystemCoreClock / 1000);
  FLASH_Lock_Fast();
  FLASH_Lock();

  __enable_irq();

  bool matched = memcmp(_data, (void*)EEPROM_AREA_ADDRESS, EEPROM_SIZE) == 0;
  if (matched) {
    _dirty = false;
  }
  return matched;
}

bool EEPROMClass::end() {
  bool retval;

  retval = commit();
  _data = nullptr;
  _size = 0;
  return(retval);
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_EEPROM)
EEPROMClass EEPROM;
#endif

#endif
