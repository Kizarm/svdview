#ifndef GPIO_H
#define GPIO_H

#include "stm32f40x.h"
#include "system.h"
#include "common.h"
#include "core_cm4.h"

/// Asociace port Adress a RCC clock
struct GpioAssocPort {
  GPIO_Type * portAdr;
  uint32_t    clkMask;
};

/** 
  * @brief  GPIO Configuration Mode enumeration 
  */   
typedef enum { 
  GPIO_Mode_IN   = 0x00, /*!< GPIO Input Mode */
  GPIO_Mode_OUT  = 0x01, /*!< GPIO Output Mode */
  GPIO_Mode_AF   = 0x02, /*!< GPIO Alternate function Mode */
  GPIO_Mode_AN   = 0x03  /*!< GPIO Analog Mode */
}GPIOMode_TypeDef;

/** 
  * @brief  GPIO Output type enumeration 
  */  
typedef enum { 
  GPIO_OType_PP = 0x00,
  GPIO_OType_OD = 0x01
}GPIOOType_TypeDef;
/** 
  * @brief  GPIO Output Maximum frequency enumeration 
  */  
typedef enum { 
  GPIO_Speed_2MHz   = 0x00, /*!< Low speed */
  GPIO_Speed_25MHz  = 0x01, /*!< Medium speed */
  GPIO_Speed_50MHz  = 0x02, /*!< Fast speed */
  GPIO_Speed_100MHz = 0x03  /*!< High speed on 30 pF (80 MHz Output max speed on 15 pF) */
}GPIOSpeed_TypeDef;
/** 
  * @brief  GPIO Configuration PullUp PullDown enumeration 
  */ 
typedef enum { 
  GPIO_PuPd_NOPULL = 0x00,
  GPIO_PuPd_UP     = 0x01,
  GPIO_PuPd_DOWN   = 0x02
}GPIOPuPd_TypeDef;
typedef enum {
  GPIO_Dir_Mode_IN   = 0x00, /*!< GPIO Input Mode              */
  GPIO_Dir_Mode_OUT  = 0x01, /*!< GPIO Output Mode             */
} GPIODir_TypeDef;

/// Enum pro PortNumber
typedef enum {
  GpioPortA,
  GpioPortB,
  GpioPortC,
  GpioPortD,
  GpioPortE,
  GpioPortF,
  GpioPortG,
  GpioPortH,
  GpioPortI
} GpioPortNum;
/** @file
  * @brief Obecný GPIO pin.
  * 
  * @class GpioClass
  * @brief Obecný GPIO pin.
  * 
  *  Ukázka přetížení operátorů. Návratové hodnoty jsou v tomto případě celkem zbytečné,
  * ale umožňují řetězení, takže je možné napsat např.
    @code
    +-+-+-led;
    @endcode
  * a máme na led 3 pulsy. Je to sice blbost, ale funguje.
  * Všechny metody jsou konstantní, protože nemění data uvnitř třídy.
  * Vlastně ani nemohou, protože data jsou konstantní.
*/
class GpioClass {
  public:
    /** Konstruktor
    @param port GpioPortA | GpioPortB | GpioPortC | GpioPortD | GpioPortF
    @param no   číslo pinu na portu
    @param type IN, OUT, AF, AN default OUT 
    */
    GpioClass (GpioPortNum const port, const uint32_t no, const GPIOMode_TypeDef type = GPIO_Mode_OUT);
    /// Nastav pin @param b na tuto hodnotu
    const GpioClass& operator<< (const bool b) const {
      if (b) io->BSRRL = pos;
      else   io->BSRRH = pos;
      return *this;
    }
    /// Nastav pin na log. H
    const GpioClass& operator+ (void) const {
      io->BSRRL = pos;
      return *this;
    }
    /// Nastav pin na log. L
    const GpioClass& operator- (void) const {
      io->BSRRH = pos;
      return *this;
    }
    /// Změň hodnotu pinu
    const GpioClass& operator~ (void) const {
      io->ODR ^= pos;
      return *this;
    };
    /// Načti logickou hodnotu na pinu
    const bool get (void) const {
      if (io->IDR & pos) return true;
      else              return false;
    };
    /// A to samé jako operátor
    const GpioClass& operator>> (bool& b) const {
      b = get();
      return *this;
    }
    /// Různá nastavení
    void setMode (GPIOMode_TypeDef p) {
      uint32_t dno = num * 2;
      io->MODER   &= ~(3UL << dno);
      io->MODER   |=  (p   << dno);
    }
    void setOType (GPIOOType_TypeDef p) {
      io->OTYPER  &= ~(1UL << num);
      io->OTYPER  |=  (p   << num);
    }
    void setSpeed (GPIOSpeed_TypeDef p) {
      uint32_t dno = num * 2;
      io->OSPEEDR &= ~(3UL << dno);
      io->OSPEEDR |=  (p   << dno);
    }
    void setPuPd (GPIOPuPd_TypeDef p) {
      uint32_t dno = num * 2;
      io->PUPDR   &= ~(3UL << dno);
      io->PUPDR   |=  (p   << dno);
    }
    void setAF (unsigned af) {
      register unsigned int pd,pn = num;
      pd = (pn & 7) << 2; pn >>= 3;
      io->AFR[pn] &= ~(0xFU << pd);
      io->AFR[pn] |=  (  af << pd);
    }
    void setDir (GPIODir_TypeDef p) {
      uint32_t dno = num * 2;
      if (p) io->MODER   |=  (1UL << dno);
      else   io->MODER   &= ~(3UL << dno);
    }
  private:
    /// Port.
    GPIO_Type * const io; // může být klidně i odkaz, ale je to stejné, necháme to tak jak bylo
    /// A pozice pinu na něm, stačí 16.bit
    const uint16_t pos;
    /// pro funkce setXXX necháme i číslo pinu
    const uint16_t num;
  
};
class GpioGroup {
  public:
    GpioGroup (GpioPortNum const port, const uint8_t * const list, const uint32_t num, const GPIOMode_TypeDef type = GPIO_Mode_AF);
    void setMode (GPIOMode_TypeDef p) const {
      unsigned i,m;
      for (i=0,m=0; i<size; i++) m |= (3 << 2*pins[i]);
      io->MODER   &= ~ m;
      for (i=0,m=0; i<size; i++) m |= (p << 2*pins[i]);
      io->MODER   |=   m;
    }
    void setOType (GPIOOType_TypeDef p) const {
      unsigned i,m;
      for (i=0,m=0; i<size; i++) m |= (1 <<   pins[i]);
      io->OTYPER  &= ~ m;
      for (i=0,m=0; i<size; i++) m |= (p <<   pins[i]);
      io->OTYPER  |=   m;
    }
    void setSpeed (GPIOSpeed_TypeDef p) const {
      unsigned i,m;
      for (i=0,m=0; i<size; i++) m |= (3 << 2*pins[i]);
      io->OSPEEDR &= ~ m;
      for (i=0,m=0; i<size; i++) m |= (p << 2*pins[i]);
      io->OSPEEDR |=   m;
    }
    void setPuPd (GPIOPuPd_TypeDef p) const {
      unsigned i,m;
      for (i=0,m=0; i<size; i++) m |= (3 << 2*pins[i]);
      io->PUPDR   &= ~ m;
      for (i=0,m=0; i<size; i++) m |= (p << 2*pins[i]);
      io->PUPDR   |=   m;
    }
    void setAF (unsigned af) const {
      struct {
        uint32_t  c,s;
      }           m[2];                 // masky pro clear a set
      union {
        struct {
          uint8_t d : 3;                // spodní 3  bity 0..7
          uint8_t c : 5;                // 0 nebo 1
        };
        uint8_t   v;                    // pin má hodnotu 0..15
      }           up;                   // union pro pin
      unsigned    n;                    // index

      for (n=0; n<2; n++) {             // nulování masek
        m[n].c = 0;
        m[n].s = 0;
      }
      for (n=0; n<size; n++) {          // pro všechny piny
        up.v = pins [n];                // nastav union
        const unsigned sh = up.d << 2;  // shift je 4x větší než spodní 3 bity pinu
        m[up.c].c |= 0xFU << sh;        // maska nulování
        m[up.c].s |=   af << sh;        // maska nastavení
      }
      for (n=0; n<2; n++) {             // nastav najednou
        if (! m[n].c) continue;         // jen co je potřeba
        io->AFR[n] &= ~ m[n].c;         // napřed nuluj co tam bylo
        io->AFR[n] |=   m[n].s;         // a pak nastav potřebné
      }
      
    }
    /// manipulace
    void    set (const uint8_t v) const {
      unsigned i,m;
      for (i=0,m=0; i<size; i++) {
        if (v & (1U << i)) m |= 0x00001 << pins[i];
        else               m |= 0x10000 << pins[i];
        // V hlavičce F4 je BSRR popsán blbě - rozdělen na spodní a horní 16.bit část
        //volatile uint32_t * bsrr = (volatile uint32_t *) & io->BSRRL;
        io->BSRR = m;
      }
    }
    uint8_t get (void) const {
      uint8_t result = 0;
      uint32_t v = io->IDR;
      for (unsigned i=0; i<size; i++) {
        if (v & (1U << pins[i])) result |= (1U << i);
      }
      return result;
    }
  private:
    /// Port.
    GPIO_Type     * const io;
    const uint8_t * const pins;
    const uint32_t        size;
};

#endif // GPIO_H
