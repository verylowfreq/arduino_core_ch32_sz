/**
 *******************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * This software component is licensed by WCH under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 *******************************************************************************
 */

#include "pins_arduino.h"
#include "hw_config.h"




// Digital PinName array
const PinName digitalPin[] = {
  // D0-D15 Digital Pins
  PA_10,
  PA_9,
  PA_8,
  PB_12,
  PA_15,
  PB_3,
  PB_4,
  PB_5,
  PB_11,
  PB_10,
  PA_4,
  PA_7,
  PA_6,
  PA_5,
  PB_9,
  PB_8,

  // D16-21 Analog Pins
  PA_0,
  PA_1,
  PA_2,
  PA_3,
  PB_0,
  PB_1,

  // D22-D24 Additional Pins
  PC_13,
  PC_14,
  PC_15
};


// Analog (Ax) pin number array
const uint32_t analogInputPin[] = {
  16,  // A0 = PA0
  17,  // A1 = PA1
  18,  // A2 = PA2
  19,  // A3 = PA3
  20,  // A4 = PB0
  21,  // A5 = PB1
  10,  // A6 = PA4
  13,  // A7 = PA5
  12,  // A8 = PA6
  11,  // A9 = PA7
};

extern "C" {

void pre_init(void) {
  // Enable Flash enhance read mode for full 224KB
  FLASH->KEYR = 0x45670123; // FLASH_Unlock_Fast();
  FLASH->KEYR = 0xCDEF89AB;

  FLASH->CTLR |= (1 << 24); // Enhanced Read Mode

  FLASH->CTLR |= (1 << 15); // FLASH_Lock_Fast();

  hw_config_init();
}

}