#include "../STM32F0x1.h"
#include "system.h"

#if !defined  (HSE_VALUE)     
#define HSE_VALUE        ((uint32_t)8000000)           /*!< Value of the External oscillator in Hz */
#endif /* HSE_VALUE */
#if !defined  (HSI_VALUE) 
#define HSI_VALUE        ((uint32_t)8000000)           /*!< Value of the Internal High Speed oscillator in Hz. */
#endif /* HSI_VALUE */

#define HSE_STARTUP_TIMEOUT   ((uint16_t)0x5000)       /*!< Time out for HSE start up */

typedef enum {
  USEHSI = 0, USEHSE, USEPLL
} SW_EN;
typedef enum {
  RCC_CFGR_PLLMUL2 = 0,
  RCC_CFGR_PLLMUL3, 
  RCC_CFGR_PLLMUL4, 
  RCC_CFGR_PLLMUL5, 
  RCC_CFGR_PLLMUL6, 
  RCC_CFGR_PLLMUL7, 
  RCC_CFGR_PLLMUL8, 
  RCC_CFGR_PLLMUL9, 
  RCC_CFGR_PLLMUL10,
  RCC_CFGR_PLLMUL11,
  RCC_CFGR_PLLMUL12,
  RCC_CFGR_PLLMUL13,
  RCC_CFGR_PLLMUL14,
  RCC_CFGR_PLLMUL15,
  RCC_CFGR_PLLMUL16,
} PLLML_EN;

/* Select the PLL clock source */
  #define PLL_SOURCE_HSI        // HSI (~8MHz) used to clock the PLL, and the PLL is used as system clock source
//#define PLL_SOURCE_HSE        // HSE (8MHz) used to clock the PLL, and the PLL is used as system clock source
//#define PLL_SOURCE_HSE_BYPASS // HSE bypassed with an external clock (8MHz, coming from ST-Link) used to clock
// the PLL, and the PLL is used as system clock source

uint32_t SystemCoreClock    = 48000000;
const uint8_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};


static void SetSysClock (void);

/**
  * @brief  Setup the microcontroller system.
  *         Initialize the Embedded Flash Interface, the PLL and update the
  *         SystemCoreClock variable.
  * @param  None
  * @retval None
  */
extern "C"
void SystemInit (void) {
  /* Set HSION bit */
  RCC.CR.R    |= (uint32_t) 0x00000001;
  /* Reset SW[1:0], HPRE[3:0], PPRE[2:0], ADCPRE and MCOSEL[2:0] bits */
  RCC.CFGR.R  &= (uint32_t) 0xF8FFB80C;
  /* Reset HSEON, CSSON and PLLON bits */
  RCC.CR.R    &= (uint32_t) 0xFEF6FFFF;
  /* Reset HSEBYP bit */
  RCC.CR.R    &= (uint32_t) 0xFFFBFFFF;
  /* Reset PLLSRC, PLLXTPRE and PLLMUL[3:0] bits */
  RCC.CFGR.R  &= (uint32_t) 0xFFC0FFFF;
  /* Reset PREDIV1[3:0] bits */
  RCC.CFGR2.R &= (uint32_t) 0xFFFFFFF0;
  /* Reset USARTSW[1:0], I2CSW, CECSW and ADCSW bits */
  RCC.CFGR3.R &= (uint32_t) 0xFFFFFEAC;
  /* Reset HSI14 bit */
  RCC.CR2.R   &= (uint32_t) 0xFFFFFFFE;
  /* Disable all interrupts */
  RCC.CIR.R = 0x00000000u;
  /* Configure the System clock frequency, AHB/APBx prescalers and Flash settings */
  SetSysClock();
}

/**
  * @brief  Update SystemCoreClock according to Clock Register Values
  *         The SystemCoreClock variable contains the core clock (HCLK), it can
  *         be used by the user application to setup the SysTick timer or configure
  *         other parameters.
  *
  * @note   Each time the core clock (HCLK) changes, this function must be called
  *         to update SystemCoreClock variable value. Otherwise, any configuration
  *         based on this variable will be incorrect.
  *
  * @note   - The system frequency computed by this function is not the real
  *           frequency in the chip. It is calculated based on the predefined
  *           constant and the selected clock source:
  *
  *           - If SYSCLK source is HSI, SystemCoreClock will contain the HSI_VALUE(*)
  *
  *           - If SYSCLK source is HSE, SystemCoreClock will contain the HSE_VALUE(**)
  *
  *           - If SYSCLK source is PLL, SystemCoreClock will contain the HSE_VALUE(**)
  *             or HSI_VALUE(*) multiplied/divided by the PLL factors.
  *
  *         (*) HSI_VALUE is a constant defined in stm32f0xx.h file (default value
  *             8 MHz) but the real value may vary depending on the variations
  *             in voltage and temperature.
  *
  *         (**) HSE_VALUE is a constant defined in stm32f0xx.h file (default value
  *              8 MHz), user has to ensure that HSE_VALUE is same as the real
  *              frequency of the crystal used. Otherwise, this function may
  *              have wrong result.
  *
  *         - The result of this function could be not correct when using fractional
  *           value for HSE crystal.
  * @param  None
  * @retval None
  */
extern "C"
void SystemCoreClockUpdate (void) {
  uint32_t prediv1factor, pllmull;

  switch (RCC.CFGR.B.SWS) {
    case USEHSI:  /* HSI used as system clock */
      SystemCoreClock = HSI_VALUE;
      break;
    case USEHSE:  /* HSE used as system clock */
      SystemCoreClock = HSE_VALUE;
      break;
    case USEPLL:  /* PLL used as system clock */
      /* Get PLL clock source and multiplication factor */
      pllmull = (uint32_t) RCC.CFGR.B.PLLMUL + 2u;

      if (RCC.CFGR.B.PLLSRC == RESET) {
        /* HSI oscillator clock divided by 2 selected as PLL clock entry */
        SystemCoreClock = (HSI_VALUE >> 1) * pllmull;
      } else {
        prediv1factor = RCC.CFGR2.B.PREDIV + 1;
        /* HSE oscillator clock selected as PREDIV1 clock entry */
        SystemCoreClock = (HSE_VALUE / prediv1factor) * pllmull;
      }
      break;
    default: /* HSI used as system clock */
      SystemCoreClock = HSI_VALUE;
      break;
  }
  /* Compute HCLK clock frequency */
  /* Get HCLK prescaler */
  pllmull = AHBPrescTable[RCC.CFGR.B.HPRE];
  /* HCLK clock frequency */
  SystemCoreClock >>= pllmull;
}

/**
  * @brief  Configures the System clock frequency, AHB/APBx prescalers and Flash
  *         settings.
  * @note   This function should be called only once the RCC clock configuration
  *         is reset to the default reset state (done in SystemInit() function).
  * @param  None
  * @retval None
  */

static void SetSysClock (void) {
  /* SYSCLK, HCLK, PCLK configuration */
#if defined (PLL_SOURCE_HSI)
  /* At this stage the HSI is already enabled */
  /* Enable Prefetch Buffer and set Flash Latency */
  Flash.ACR.setbit([] (auto & r) -> auto { // C++14
    r.B.PRFTBE  = SET;
    r.B.LATENCY = SET;
    return r.R;
  });
  RCC.CFGR.modify([] (auto & r) -> auto {
    r.B.HPRE     = 0;
    r.B.PPRE     = 0;
    r.B.PLLSRC   = RESET;
    r.B.PLLXTPRE = RESET;
    r.B.PLLMUL   = RCC_CFGR_PLLMUL12;
    return r.R;
  });
  /* Enable PLL */
  RCC.CR.B.PLLON = SET;
  /* Wait till PLL is ready */
  while ((RCC.CR.B.PLLRDY) == RESET);
  /* Select PLL as system clock source */
  RCC.CFGR.B.SW = USEPLL;
  /* Wait till PLL is used as system clock source */
  while (RCC.CFGR.B.SWS != USEPLL);
#else
#if defined (PLL_SOURCE_HSE)
  /* Enable HSE */
  RCC.CR.B.HSEON = SET;
#elif defined (PLL_SOURCE_HSE_BYPASS)
  /* HSE oscillator bypassed with external clock */
  RCC.CR.B.HSEON  = SET;
  RCC.CR.B.HSEBYP = SET;
#endif /* PLL_SOURCE_HSE */
  __IO uint32_t StartUpCounter = 0;
  __IO uint32_t HSEStatus;
  /* Wait till HSE is ready and if Time out is reached exit */
  do {
    HSEStatus = RCC.CR.B.HSERDY;
    StartUpCounter++;
  } while ((HSEStatus == RESET) && (StartUpCounter != HSE_STARTUP_TIMEOUT));

  HSEStatus = RCC.CR.B.HSERDY;

  if (HSEStatus == SET) {
    /* Enable Prefetch Buffer and set Flash Latency */
    Flash.ACR.setbit([] (auto & r) -> uint32_t {
      r.B.PRFTBE  = SET;
      r.B.LATENCY = SET;
      return r.R;
    });
    RCC.CFGR.modify([] (auto & r) -> uint32_t {
      r.B.HPRE     = 0;
      r.B.PPRE     = 0;
      r.B.PLLSRC   = SET;
      r.B.PLLXTPRE = RESET;
      r.B.PLLMUL   = RCC_CFGR_PLLMUL12;
      return r.R;
    });
    /* Enable PLL */
    RCC.CR.B.PLLON = SET;
    /* Wait till PLL is ready */
    while ((RCC.CR.B.PLLRDY) == RESET);
    /* Select PLL as system clock source */
    RCC.CFGR.B.SW = USEPLL;
    /* Wait till PLL is used as system clock source */
    while (RCC.CFGR.B.SWS != USEPLL);
  } else {
    /* If HSE fails to start-up, the application will have wrong clock
         configuration. User can add here some code to deal with this error */
  }
#endif /* PLL_SOURCE_HSI */
}
