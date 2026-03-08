/********************************** (C) COPYRIGHT *******************************
 * File Name          : ch32v20x_it.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2021/06/06
 * Description        : Main Interrupt Service Routines.
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/
#include "ch32v20x_it.h"

// void NMI_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
// void HardFault_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
// void Ecall_U_Mode_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
// void Ecall_M_Mode_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

// /*********************************************************************
//  * @fn      NMI_Handler
//  *
//  * @brief   This function handles NMI exception.
//  *
//  * @return  none
//  */
// void NMI_Handler(void)
// {
// }

// /*********************************************************************
//  * @fn      HardFault_Handler
//  *
//  * @brief   This function handles Hard Fault exception.
//  *
//  * @return  none
//  */
// void HardFault_Handler(void)
// {
//   while (1)
//   {
    
//   }
// }


// /*********************************************************************
//  * @fn      Ecall_U_Mode_Handler
//  *
//  * @brief   This function handles user mode ecall exception.
//  *
//  * @return  none
//  */
// void Ecall_U_Mode_Handler(void)
// {
//   while(1)
//   {
    
//   }
// }

// /*********************************************************************
//  * @fn      Ecall_M_Mode_Handler
//  *
//  * @brief   This function handles mashine mode ecall exception.
//  *
//  * @return  none
//  */
// void Ecall_M_Mode_Handler(void)
// {
//   while(1)
//   {
    
//   }
// }


// Default interrupt handler definition

void while1_handler(void) {
  while(1) {

  }
};


// Weak symbol for interrupts

__attribute__((weak)) void NMI_Handler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void HardFault_Handler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void Ecall_M_Mode_Handler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void Ecall_U_Mode_Handler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void Break_Point_Handler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void SysTick_Handler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void SW_Handler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void WWDG_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void PVD_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void TAMPER_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void RTC_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void FLASH_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void RCC_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void EXTI0_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void EXTI1_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void EXTI2_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void EXTI3_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void EXTI4_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void DMA1_Channel1_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void DMA1_Channel2_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void DMA1_Channel3_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void DMA1_Channel4_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void DMA1_Channel5_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void DMA1_Channel6_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void DMA1_Channel7_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void ADC1_2_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void USB_HP_CAN1_TX_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void USB_LP_CAN1_RX0_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void CAN1_RX1_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void CAN1_SCE_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void EXTI9_5_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void TIM1_BRK_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void TIM1_UP_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void TIM1_TRG_COM_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void TIM1_CC_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void TIM2_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void TIM3_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void TIM4_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void I2C1_EV_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void I2C1_ER_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void I2C2_EV_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void I2C2_ER_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void SPI1_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void SPI2_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void USART1_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void USART2_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void USART3_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void EXTI15_10_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void RTCAlarm_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void USBWakeUp_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void USBHD_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void USBHDWakeUp_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void UART4_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
__attribute__((weak)) void DMA1_Channel8_IRQHandler(void) __attribute__ ((weak, interrupt("WCH-Interrupt-fast"), alias ("while1_handler")));
