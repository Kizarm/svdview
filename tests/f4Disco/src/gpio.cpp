#include "stm32f40x.h"
#include "gpio.h"
#if 0
// Tohle je nutné vytáhnout ze staticky generované hlavičky.
static const uint32_t RCC_AHB1ENR_GPIOAEN = 0x00000001u; /*!< Ofs=0 IO port A clock enable */
static const uint32_t RCC_AHB1ENR_GPIOBEN = 0x00000002u; /*!< Ofs=1 IO port B clock enable */
static const uint32_t RCC_AHB1ENR_GPIOCEN = 0x00000004u; /*!< Ofs=2 IO port C clock enable */
static const uint32_t RCC_AHB1ENR_GPIODEN = 0x00000008u; /*!< Ofs=3 IO port D clock enable */
static const uint32_t RCC_AHB1ENR_GPIOEEN = 0x00000010u; /*!< Ofs=4 IO port E clock enable */
static const uint32_t RCC_AHB1ENR_GPIOFEN = 0x00000020u; /*!< Ofs=5 IO port F clock enable */
static const uint32_t RCC_AHB1ENR_GPIOGEN = 0x00000040u; /*!< Ofs=6 IO port G clock enable */
static const uint32_t RCC_AHB1ENR_GPIOHEN = 0x00000080u; /*!< Ofs=7 IO port H clock enable */
static const uint32_t RCC_AHB1ENR_GPIOIEN = 0x00000100u; /*!< Ofs=8 IO port I clock enable */
// V C++ nejsou tyhle ukazatele definovány, je nutné si je přetáhnout a přejmenovat.
static GPIO_Type * const pGPIOA = (GPIO_Type * const) 0x40020000u; /*!< General-purpose I/Os */
static GPIO_Type * const pGPIOB = (GPIO_Type * const) 0x40020400u; /*!< General-purpose I/Os */
static GPIO_Type * const pGPIOC = (GPIO_Type * const) 0x40020800u; /*!< General-purpose I/Os */
static GPIO_Type * const pGPIOD = (GPIO_Type * const) 0x40020C00u; /*!< General-purpose I/Os */
static GPIO_Type * const pGPIOE = (GPIO_Type * const) 0x40021000u; /*!< General-purpose I/Os */
static GPIO_Type * const pGPIOF = (GPIO_Type * const) 0x40021400u; /*!< General-purpose I/Os */
static GPIO_Type * const pGPIOG = (GPIO_Type * const) 0x40021800u; /*!< General-purpose I/Os */
static GPIO_Type * const pGPIOH = (GPIO_Type * const) 0x40021C00u; /*!< General-purpose I/Os */
static GPIO_Type * const pGPIOI = (GPIO_Type * const) 0x40022000u; /*!< General-purpose I/Os */
#endif // 0

#define RCC_AHB1ENR_GPIOAEN ((uint32_t)0x00000001u) /*!< Ofs=0 IO port A clock enable */
#define RCC_AHB1ENR_GPIOBEN ((uint32_t)0x00000002u) /*!< Ofs=1 IO port B clock enable */
#define RCC_AHB1ENR_GPIOCEN ((uint32_t)0x00000004u) /*!< Ofs=2 IO port C clock enable */
#define RCC_AHB1ENR_GPIODEN ((uint32_t)0x00000008u) /*!< Ofs=3 IO port D clock enable */
#define RCC_AHB1ENR_GPIOEEN ((uint32_t)0x00000010u) /*!< Ofs=4 IO port E clock enable */
#define RCC_AHB1ENR_GPIOFEN ((uint32_t)0x00000020u) /*!< Ofs=5 IO port F clock enable */
#define RCC_AHB1ENR_GPIOGEN ((uint32_t)0x00000040u) /*!< Ofs=6 IO port G clock enable */
#define RCC_AHB1ENR_GPIOHEN ((uint32_t)0x00000080u) /*!< Ofs=7 IO port H clock enable */
#define RCC_AHB1ENR_GPIOIEN ((uint32_t)0x00000100u) /*!< Ofs=8 IO port I clock enable */

#define pGPIOA ((GPIO_Type * const) 0x40020000u) /*!< General-purpose I/Os */
#define pGPIOB ((GPIO_Type * const) 0x40020400u) /*!< General-purpose I/Os */
#define pGPIOC ((GPIO_Type * const) 0x40020800u) /*!< General-purpose I/Os */
#define pGPIOD ((GPIO_Type * const) 0x40020C00u) /*!< General-purpose I/Os */
#define pGPIOE ((GPIO_Type * const) 0x40021000u) /*!< General-purpose I/Os */
#define pGPIOF ((GPIO_Type * const) 0x40021400u) /*!< General-purpose I/Os */
#define pGPIOG ((GPIO_Type * const) 0x40021800u) /*!< General-purpose I/Os */
#define pGPIOH ((GPIO_Type * const) 0x40021C00u) /*!< General-purpose I/Os */
#define pGPIOI ((GPIO_Type * const) 0x40022000u) /*!< General-purpose I/Os */
// Tabulka 72 bytu ve flash, kód to zkrátí i zrychlí. Aby byla tabulka ve flash, nelze do ní
// umístit odkazy, musí to být ukazatele (jsou konstantní). V C to takhle inicializovat nejde,
// musely by se použít define, C++ (gcc) to spolkne i statické konstanty.
// Ukázalo se, že clang sice statické konstanty spolkne, ale umístí tabulku do .bss (vytvoří
// pro ní statický kostruktor). To je sice nepříjemné, ale horší je, že to nefunguje.
// Pokud zůstaneme u klasických define, funguje to správně, takže to tak necháme.
static const struct GpioAssocPort cPortTab[] = {
  {pGPIOA, RCC_AHB1ENR_GPIOAEN},
  {pGPIOB, RCC_AHB1ENR_GPIOBEN},
  {pGPIOC, RCC_AHB1ENR_GPIOCEN},
  {pGPIOD, RCC_AHB1ENR_GPIODEN},
  {pGPIOE, RCC_AHB1ENR_GPIOEEN},
  {pGPIOF, RCC_AHB1ENR_GPIOFEN},
  {pGPIOG, RCC_AHB1ENR_GPIOGEN},
  {pGPIOH, RCC_AHB1ENR_GPIOHEN},
  {pGPIOI, RCC_AHB1ENR_GPIOIEN},
};

GpioClass::GpioClass (GpioPortNum const port, const uint32_t no, const GPIOMode_TypeDef type) :
  io (cPortTab[port].portAdr), pos(1UL << no), num(no) {
  // Povol hodiny
  RCC.AHB1ENR.R |= cPortTab[port].clkMask;
  // A nastav pin (pořadí dle ST knihovny).
  setSpeed (GPIO_Speed_100MHz);
  setOType (GPIO_OType_PP);
  setMode  (type);
  setPuPd  (GPIO_PuPd_NOPULL);
}
GpioGroup::GpioGroup (const GpioPortNum port, const uint8_t * const list, const uint32_t num, const GPIOMode_TypeDef type) :
  io(cPortTab[port].portAdr), pins (list), size (num){
  // Povol hodiny
  RCC.AHB1ENR.R |= cPortTab[port].clkMask;
  // A nastav pin (pořadí dle ST knihovny).
  setSpeed (GPIO_Speed_100MHz);
//setOType (GPIO_OType_PP);
  setMode  (type);
//setPuPd  (GPIO_PuPd_NOPULL);
}

