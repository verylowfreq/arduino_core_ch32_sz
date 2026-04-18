#if defined(USE_TINYUSB)
// Keep Adafruit's CH32 port implementation, but provide a core-local
// TinyUSB_Port_EnterDFU() override below.
#define TinyUSB_Port_EnterDFU TinyUSB_Port_EnterDFU_AdafruitDefault

#include "../../../libraries/Adafruit_TinyUSB_Arduino/src/arduino/ports/ch32/Adafruit_TinyUSB_ch32.cpp"

#undef TinyUSB_Port_EnterDFU

void TinyUSB_Port_EnterDFU(void) {
  // Give USB control transfer time to complete before reset.
  delay(10);

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
  PWR_BackupAccessCmd(ENABLE);
  // Bootloader entry marker checked after reset.
  BKP_WriteBackupRegister(BKP_DR10, 0x624c);

  // Reboot into bootloader path.
  NVIC_SystemReset();
}
#endif
