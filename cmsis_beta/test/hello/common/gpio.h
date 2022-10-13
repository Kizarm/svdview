#ifndef GPIO_H
#define GPIO_H

#include "../STM32F0x1.h"

/** 
  * @brief General Purpose IO
  */

typedef enum {
  GPIO_Mode_IN   = 0x00, /*!< GPIO Input Mode              */
  GPIO_Mode_OUT  = 0x01, /*!< GPIO Output Mode             */
  GPIO_Mode_AF   = 0x02, /*!< GPIO Alternate function Mode */
  GPIO_Mode_AN   = 0x03  /*!< GPIO Analog In/Out Mode      */
} GPIOMode_TypeDef;

typedef enum {
  GPIO_OType_PP = 0x00,
  GPIO_OType_OD = 0x01
} GPIOOType_TypeDef;

typedef enum {
  GPIO_Speed_Level_1  = 0x01, /*!< Medium Speed */
  GPIO_Speed_Level_2  = 0x02, /*!< Fast Speed   */
  GPIO_Speed_Level_3  = 0x03  /*!< High Speed   */
} GPIOSpeed_TypeDef;

typedef enum {
  GPIO_PuPd_NOPULL = 0x00,
  GPIO_PuPd_UP     = 0x01,
  GPIO_PuPd_DOWN   = 0x02
} GPIOPuPd_TypeDef;
/// Enum pro PortNumber
typedef enum {
  GpioPortA,
  GpioPortB,
  GpioPortC,
  GpioPortD,
  GpioPortF
} GpioPortNum;
/// Asociace port Adress a RCC clock
struct GpioAssocPort {
  GPIOF_DEF * const portAdr;
  const uint32_t    clkMask;
};
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
    explicit GpioClass (GpioPortNum const port, const uint32_t no, const GPIOMode_TypeDef type = GPIO_Mode_OUT) noexcept;
    /// Nastav pin @param b na tuto hodnotu
    const GpioClass& operator<< (const bool b) const {
      if (b) io->BSRR.R = pos;
      else   io->BRR.R  = pos;
      return *this;
    }
//![Gpio example]
    /// Nastav pin na log. H
    const GpioClass& operator+ (void) const {
      io->BSRR.R = (uint32_t) pos;
      return *this;
    }
    /// Nastav pin na log. L
    const GpioClass& operator- (void) const {
      io->BRR.R  = pos;
      return *this;
    }
    /// Změň hodnotu pinu
    const GpioClass& operator~ (void) const {
      io->ODR.R ^= pos;
      return *this;
    };
    /// Načti logickou hodnotu na pinu
    const bool get (void) const {
      if (io->IDR.R & pos) return true;
      else               return false;
    };
    /// A to samé jako operátor
    const GpioClass& operator>> (bool& b) const {
      b = get();
      return *this;
    }
//![Gpio example]
    void setMode (GPIOMode_TypeDef p) {
      uint32_t dno = num * 2;
      io->MODER.R   &= ~(3UL << dno);
      io->MODER.R   |=  (p   << dno);
    }
    void setOType (GPIOOType_TypeDef p) {
      io->OTYPER.R  &= (uint16_t)~(1UL << num);
      io->OTYPER.R  |= (uint16_t) (p   << num);
    }
    void setSpeed (GPIOSpeed_TypeDef p) {
      uint32_t dno = num * 2;
      io->OSPEEDR.R &= ~(3UL << dno);
      io->OSPEEDR.R |=  (p   << dno);
    }
    void setPuPd (GPIOPuPd_TypeDef p) {
      uint32_t dno = num * 2;
      io->PUPDR.R   &= ~(3UL << dno);
      io->PUPDR.R   |=  (p   << dno);
    }
    void setAF (unsigned af) {
      unsigned int pd,pn = num;
      pd = (pn & 7) << 2; pn >>= 3;
      if (pn) {
        io->AFRH.R &= ~(0xFU << pd);
        io->AFRH.R |=  (  af << pd);
      } else {
        io->AFRL.R &= ~(0xFU << pd);
        io->AFRL.R |=  (  af << pd);
      }
    }
  private:
    /// Port.
    GPIOF_DEF * const io;
    /// A pozice pinu na něm, stačí 16.bit
    const uint16_t   pos;
    /// pro funkce setXXX necháme i číslo pinu
    const uint16_t   num;
  
};

#endif // GPIO_H
