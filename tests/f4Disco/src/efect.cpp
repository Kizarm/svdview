#include "stm32f40x.h"
#include "gpio.h"
#include "efect.h"

static Efect * pEfectInstance = 0;

Efect::Efect() {
  cyc = 0; sta = 0;
  pEfectInstance = this;
  RCC.APB1ENR.B.TIM4EN = 1u;
  const uint8_t pins [4] = {12,13,14,15};
  GpioGroup     grpi (GpioPortD, pins, 4, GPIO_Mode_AF);
  grpi.setAF (2);
  
  TIM4.PSC      = 512;
  TIM4.ARR.R    = 256;
  TIM4.RCR      = 0;
  TIM4.EGR.B.UG = 1;
  
  TIM4.CCMR1_Output.modify([](TIM_CCMR1_Output_s & r) -> auto {
    r.B.OC1PE = 1u;
    r.B.OC1M  = 6u;
    r.B.OC2PE = 1u;
    r.B.OC2M  = 6u;
    return r.R;
  });
  TIM4.CCMR2_Output.modify([](TIM_CCMR2_Output_s & r) -> auto {
    r.B.OC3PE = 1u;
    r.B.OC3M  = 6u;
    r.B.OC4PE = 1u;
    r.B.OC4M  = 6u;
    return r.R;
  });
  // povol piny
  TIM4.CCER.modify([](TIM_CCER_s & r) -> auto {
    r.B.CC1E = 1u;
    r.B.CC2E = 1u;
    r.B.CC3E = 1u;
    r.B.CC4E = 1u;
    return r.R;
  });
  // pocatecni pwm
  for (unsigned i=0; i<NoOfChannels; i++) set (i, 0x7F);
  // interrupt
  NVIC_EnableIRQ (TIM4_IRQn);
  // + start(true)
}
void Efect::set (uint8_t channel, uint8_t value) {
  channel &= 3;
  volatile uint32_t * const ccr = & (TIM4.CCR1.R);
  ccr [channel] = value;
}
void Efect::irq (void) {
  uint8_t dop = 0xFF - cyc;
  switch (sta) {
    case 0: set (0,cyc); set (1,0);   set (2,0);   set (3,dop); break;
    case 1: set (0,dop); set (1,cyc); set (2,0);   set (3,0);   break;
    case 2: set (0,0);   set (1,dop); set (2,cyc); set (3,0);   break;
    case 3: set (0,0);   set (1,0);   set (2,dop); set (3,cyc); break;
  }
  if (++cyc > 0xFF) {
    cyc  = 0;
    sta += 1;
    sta &= 3;
  }
}
void Efect::start (bool on) {
  if (on) {
    // nakonec cely timer - autoreload + enable
    TIM4.DIER.B.UIE = 1;
    TIM4.CR1.modify([](TIM_CR1_s & r) -> auto {
      r.B.ARPE = 1u;
      r.B.CEN  = 1u;
      return r.R;
    });
  } else {
    TIM4.CNT.R = 0;
    TIM4.DIER.B.UIE = 0u;
    set (0,0); set (1,0); set (2,0); set (3,0);
  }
}

extern "C" void TIM4_IRQHandler (void) {
  TIM4.SR.B.UIF = ~ TIM4.SR.B.UIF;
  if (pEfectInstance) pEfectInstance->irq();
}
