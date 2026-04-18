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

#ifndef EEPROM_h
#define EEPROM_h
#include <Arduino.h>


class EEPROMClass {
  public:
    EEPROMClass(void);
    ~EEPROMClass(void);

    void begin(void);

    uint8_t * getDataPtr();
    uint8_t const * getConstDataPtr() const;

    uint8_t read(int const idx);
    void write(int const idx, uint8_t const val);     // requires commit() to make data stick
    void update(int const idx, uint8_t const val) { write(idx, val); }
    void erase(void);     // requires commit() to make data stick

    bool commit(void);
    bool end(void);

    template<typename T> 
    T &get(int const address, T &t) {
      if (address < 0 || address + sizeof(T) > _size)
        return t;
      memcpy((uint8_t*) &t, _data + address, sizeof(T));
      return t;
    }

    template<typename T> 
    const T &put(int const address, const T &t) {
      if (address < 0 || address + sizeof(T) > _size)
        return t;
      if (memcmp(_data + address, (const uint8_t*)&t, sizeof(T)) != 0) {
        _dirty = true;
        memcpy(_data + address, (const uint8_t*)&t, sizeof(T));
      }
      return t;
    }

    size_t length() {return _size;}

    uint8_t& operator[](int const address) {return getDataPtr()[address];}
    uint8_t const & operator[](int const address) const {return getConstDataPtr()[address];}

  protected:
    uint8_t* _data = nullptr;
    bool _dirty = false;
    size_t _size = 0;
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_EEPROM)
extern EEPROMClass EEPROM;
#endif


#endif
