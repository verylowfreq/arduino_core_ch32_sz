/********************************** (C) COPYRIGHT *******************************
 * File Name          : ch32v20x_it.h
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2021/06/06
 * Description        : This file contains the headers of the interrupt handlers.
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/
#ifndef __CH32V20x_IT_H
#define __CH32V20x_IT_H

#ifdef __cplusplus
 extern "C" {
#endif 

// #include "debug.h"
#include <stddef.h>

void while1_handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void NMI_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void HardFault_Handler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void Ecall_M_Mode_Handler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void Ecall_U_Mode_Handler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void Break_Point_Handler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void SysTick_Handler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void SW_Handler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void WWDG_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void PVD_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void TAMPER_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void RTC_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void FLASH_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void RCC_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI0_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI1_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI2_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI3_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI4_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void DMA1_Channel1_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void DMA1_Channel2_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void DMA1_Channel3_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void DMA1_Channel4_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void DMA1_Channel5_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void DMA1_Channel6_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void DMA1_Channel7_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void ADC1_2_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void USB_HP_CAN1_TX_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void USB_LP_CAN1_RX0_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void CAN1_RX1_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void CAN1_SCE_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI9_5_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM1_BRK_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM1_UP_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM1_TRG_COM_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM1_CC_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM2_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM3_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM4_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void I2C1_EV_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void I2C1_ER_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void I2C2_EV_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void I2C2_ER_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void SPI1_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void SPI2_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void USART1_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void USART2_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void USART3_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI15_10_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void RTCAlarm_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void USBWakeUp_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void USBHD_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void USBHDWakeUp_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void UART4_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void DMA1_Channel8_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));


#ifdef __cplusplus
}
#endif


#endif /* __CH32V20x_IT_H */


