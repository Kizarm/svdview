#include "usart.h"
#include "gpio.h"
#include "STM32F0x1.h"
#include "CortexM0.h"   // SysTick

extern "C" uint32_t SystemCoreClock;      // deklarace pomocných proměnných
extern "C" void SysTick_Handler ();       // roztroušených různě po zdrojácích
// statické objekty
static Usart     ser (19200);             // sériový port, výstup přes přerušení
static GpioClass led (GpioPortA, 0u);     // dáme si tam ledku
static volatile uint32_t counter = 0u;    // mělo by být atomic, ale zde volatile stačí
// V přerušení od SysTick vlastně celkem nic být nemusí, stačí inkrementovat čítač.
void SysTick_Handler () {
  counter += 1u;
}
// čekání pomocí SysTick
static void wait (const uint32_t dly = 10u) {
  counter = 0u;
  // Při čekání lze uspávat jádro.
  while (counter <= dly) asm volatile ("wfi");
  ~led;                                   // na konci změň stav ledky
}
//////////////////////////////////////////////
int main() {
  SysTick.Config (SystemCoreClock / 100u);  // zapneme systémový časovač (100Hz)
  ser.SetRS485   (false);                   // hardware používá RS485, polarita = 0
  for (;;) {
    ser.puts ("Hello world\r\n");
    wait (20u);
  }
  return 0;
}
