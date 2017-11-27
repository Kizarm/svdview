#ifndef EFFECT_H
#define EFFECT_H
#include <stdint.h>

/** Používá ledky na F4 Discovery v PWM módu.
 * Navázáno na TIMER4.
 * */
static const uint32_t NoOfChannels = 4;

class Efect {
  public:
    Efect();
    void    set   (uint8_t channel, uint8_t value);
    void    irq   (void);
    void    start (bool on);
  private:
    uint32_t cyc;
    uint32_t sta;
};

#endif // EFFECT_H
