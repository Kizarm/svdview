/**
  * @file    main.cpp 
  * @author  Mrazik
  * @version V1.0.b
  * @date    26-11-2017
  * @brief   Main program body
  
  */ 
#include "main.h"
/**
  * @brief  Main program.
  * @retval 0 (none)
  */
extern "C" const unsigned char score [];

// Efekt je na F4 kolečko led s pwm, jinak rozsviť led
static Efect      blink;
static MidiPlayer player;

// Když dohrajeme, skončí efekt
void EndPlaying (void) {
  blink.start (false);
}

int main(void) {
  EnableDebugOnSleep();
  
  player.setEnd (EndPlaying);
  blink .start  (true);
  player.start  (score);    // Hrajeme
  // smycka je platforme zavisla, v mcu muze byt prazdna
  for (;;) {
    asm volatile ("wfi");
  }
  return 0;
}

