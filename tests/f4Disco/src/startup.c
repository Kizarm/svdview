#include <stdint.h>
#include "system.h"

#if defined (__cplusplus)
extern "C" {
#endif

/// V linker skriptu
  extern void (*__init_array_start) ();
  extern void (*__init_array_end) ();
/// Inicializace statických konstruktorů - použít explicitně po celkové inicializaci systému
  static inline void static_init() {
    void (**p) ();
    // Tohle je sice konstrukce značně nepřehledná (nalezená na webu), leč funguje.
    for (p = &__init_array_start; p < &__init_array_end; p++) (*p) ();
  }


#define WEAK     __attribute__ ((weak))
#define ALIAS(f) __attribute__ ((weak, alias (#f)))

  extern unsigned int _estack;
  /* start address for the initialization values of the .data section.
  defined in linker script */
  extern unsigned int _sidata;
  /* start address for the .data section. defined in linker script */
  extern unsigned int _sdata;
  /* end address for the .data section. defined in linker script */
  extern unsigned int _edata;
  /* start address for the .bss section. defined in linker script */
  extern unsigned int _sbss;
  /* end address for the .bss section. defined in linker script */
  extern unsigned int _ebss;

  WEAK void Reset_Handler (void);
  WEAK void DefaultHandler (void);


  void  NMI_Handler             (void) ALIAS (Default_Handler);
  void  HardFault_Handler       (void) ALIAS (Default_Handler);
  void  MemManage_Handler       (void) ALIAS (Default_Handler);
  void  BusFault_Handler        (void) ALIAS (Default_Handler);
  void  UsageFault_Handler      (void) ALIAS (Default_Handler);
  void  SVC_Handler             (void) ALIAS (Default_Handler);
  void  DebugMon_Handler        (void) ALIAS (Default_Handler);
  void  PendSV_Handler          (void) ALIAS (Default_Handler);
  void  SysTick_Handler         (void) ALIAS (Default_Handler);

  void     WWDG_IRQHandler (void) ALIAS (Default_Handler);
  void     PVD_IRQHandler (void) ALIAS (Default_Handler);
  void     TAMP_STAMP_IRQHandler (void) ALIAS (Default_Handler);
  void     RTC_WKUP_IRQHandler (void) ALIAS (Default_Handler);
  void     FLASH_IRQHandler (void) ALIAS (Default_Handler);
  void     RCC_IRQHandler (void) ALIAS (Default_Handler);
  void     EXTI0_IRQHandler (void) ALIAS (Default_Handler);
  void     EXTI1_IRQHandler (void) ALIAS (Default_Handler);
  void     EXTI2_IRQHandler (void) ALIAS (Default_Handler);
  void     EXTI3_IRQHandler (void) ALIAS (Default_Handler);
  void     EXTI4_IRQHandler (void) ALIAS (Default_Handler);
  void     DMA1_Stream0_IRQHandler (void) ALIAS (Default_Handler);
  void     DMA1_Stream1_IRQHandler (void) ALIAS (Default_Handler);
  void     DMA1_Stream2_IRQHandler (void) ALIAS (Default_Handler);
  void     DMA1_Stream3_IRQHandler (void) ALIAS (Default_Handler);
  void     DMA1_Stream4_IRQHandler (void) ALIAS (Default_Handler);
  void     DMA1_Stream5_IRQHandler (void) ALIAS (Default_Handler);
  void     DMA1_Stream6_IRQHandler (void) ALIAS (Default_Handler);
  void     ADC_IRQHandler (void) ALIAS (Default_Handler);
  void     CAN1_TX_IRQHandler (void) ALIAS (Default_Handler);
  void     CAN1_RX0_IRQHandler (void) ALIAS (Default_Handler);
  void     CAN1_RX1_IRQHandler (void) ALIAS (Default_Handler);
  void     CAN1_SCE_IRQHandler (void) ALIAS (Default_Handler);
  void     EXTI9_5_IRQHandler (void) ALIAS (Default_Handler);
  void     TIM1_BRK_TIM9_IRQHandler (void) ALIAS (Default_Handler);
  void     TIM1_UP_TIM10_IRQHandler (void) ALIAS (Default_Handler);
  void     TIM1_TRG_COM_TIM11_IRQHandler (void) ALIAS (Default_Handler);
  void     TIM1_CC_IRQHandler (void) ALIAS (Default_Handler);
  void     TIM2_IRQHandler (void) ALIAS (Default_Handler);
  void     TIM3_IRQHandler (void) ALIAS (Default_Handler);
  void     TIM4_IRQHandler (void) ALIAS (Default_Handler);
  void     I2C1_EV_IRQHandler (void) ALIAS (Default_Handler);
  void     I2C1_ER_IRQHandler (void) ALIAS (Default_Handler);
  void     I2C2_EV_IRQHandler (void) ALIAS (Default_Handler);
  void     I2C2_ER_IRQHandler (void) ALIAS (Default_Handler);
  void     SPI1_IRQHandler (void) ALIAS (Default_Handler);
  void     SPI2_IRQHandler (void) ALIAS (Default_Handler);
  void     USART1_IRQHandler (void) ALIAS (Default_Handler);
  void     USART2_IRQHandler (void) ALIAS (Default_Handler);
  void     USART3_IRQHandler (void) ALIAS (Default_Handler);
  void     EXTI15_10_IRQHandler (void) ALIAS (Default_Handler);
  void     RTC_Alarm_IRQHandler (void) ALIAS (Default_Handler);
  void     OTG_FS_WKUP_IRQHandler (void) ALIAS (Default_Handler);
  void     TIM8_BRK_TIM12_IRQHandler (void) ALIAS (Default_Handler);
  void     TIM8_UP_TIM13_IRQHandler (void) ALIAS (Default_Handler);
  void     TIM8_TRG_COM_TIM14_IRQHandler (void) ALIAS (Default_Handler);
  void     TIM8_CC_IRQHandler (void) ALIAS (Default_Handler);
  void     DMA1_Stream7_IRQHandler (void) ALIAS (Default_Handler);
  void     FSMC_IRQHandler (void) ALIAS (Default_Handler);
  void     SDIO_IRQHandler (void) ALIAS (Default_Handler);
  void     TIM5_IRQHandler (void) ALIAS (Default_Handler);
  void     SPI3_IRQHandler (void) ALIAS (Default_Handler);
  void     UART4_IRQHandler (void) ALIAS (Default_Handler);
  void     UART5_IRQHandler (void) ALIAS (Default_Handler);
  void     TIM6_DAC_IRQHandler (void) ALIAS (Default_Handler);
  void     TIM7_IRQHandler (void) ALIAS (Default_Handler);
  void     DMA2_Stream0_IRQHandler (void) ALIAS (Default_Handler);
  void     DMA2_Stream1_IRQHandler (void) ALIAS (Default_Handler);
  void     DMA2_Stream2_IRQHandler (void) ALIAS (Default_Handler);
  void     DMA2_Stream3_IRQHandler (void) ALIAS (Default_Handler);
  void     DMA2_Stream4_IRQHandler (void) ALIAS (Default_Handler);
  void     ETH_IRQHandler (void) ALIAS (Default_Handler);
  void     ETH_WKUP_IRQHandler (void) ALIAS (Default_Handler);
  void     CAN2_TX_IRQHandler (void) ALIAS (Default_Handler);
  void     CAN2_RX0_IRQHandler (void) ALIAS (Default_Handler);
  void     CAN2_RX1_IRQHandler (void) ALIAS (Default_Handler);
  void     CAN2_SCE_IRQHandler (void) ALIAS (Default_Handler);
  void     OTG_FS_IRQHandler (void) ALIAS (Default_Handler);
  void     DMA2_Stream5_IRQHandler (void) ALIAS (Default_Handler);
  void     DMA2_Stream6_IRQHandler (void) ALIAS (Default_Handler);
  void     DMA2_Stream7_IRQHandler (void) ALIAS (Default_Handler);
  void     USART6_IRQHandler (void) ALIAS (Default_Handler);
  void     I2C3_EV_IRQHandler (void) ALIAS (Default_Handler);
  void     I2C3_ER_IRQHandler (void) ALIAS (Default_Handler);
  void     OTG_HS_EP1_OUT_IRQHandler (void) ALIAS (Default_Handler);
  void     OTG_HS_EP1_IN_IRQHandler (void) ALIAS (Default_Handler);
  void     OTG_HS_WKUP_IRQHandler (void) ALIAS (Default_Handler);
  void     OTG_HS_IRQHandler (void) ALIAS (Default_Handler);
  void     DCMI_IRQHandler (void) ALIAS (Default_Handler);
  void     CRYP_IRQHandler (void) ALIAS (Default_Handler);
  void     HASH_RNG_IRQHandler (void) ALIAS (Default_Handler);
  void     FPU_IRQHandler (void) ALIAS (Default_Handler);


  extern int main (void);
  extern void SystemInit (void);

#if defined (__cplusplus)
} // extern "C"
#endif

__attribute__ ( (section (".isr_vector")))
void (* const Vectors[]) (void) = {
  (void ( *) (void)) &_estack,
  Reset_Handler,                    // The reset handler

  NMI_Handler,
  HardFault_Handler,
  MemManage_Handler,
  BusFault_Handler,
  UsageFault_Handler,
  0,
  0,
  0,
  0,
  SVC_Handler,
  DebugMon_Handler,
  0,
  PendSV_Handler,
  SysTick_Handler,

  /* External Interrupts */
  WWDG_IRQHandler,                   /* Window WatchDog              */
  PVD_IRQHandler,                    /* PVD through EXTI Line detection */
  TAMP_STAMP_IRQHandler,             /* Tamper and TimeStamps through the EXTI line */
  RTC_WKUP_IRQHandler,               /* RTC Wakeup through the EXTI line */
  FLASH_IRQHandler,                  /* FLASH                        */
  RCC_IRQHandler,                    /* RCC                          */
  EXTI0_IRQHandler,                  /* EXTI Line0                   */
  EXTI1_IRQHandler,                  /* EXTI Line1                   */
  EXTI2_IRQHandler,                  /* EXTI Line2                   */
  EXTI3_IRQHandler,                  /* EXTI Line3                   */
  EXTI4_IRQHandler,                  /* EXTI Line4                   */
  DMA1_Stream0_IRQHandler,           /* DMA1 Stream 0                */
  DMA1_Stream1_IRQHandler,           /* DMA1 Stream 1                */
  DMA1_Stream2_IRQHandler,           /* DMA1 Stream 2                */
  DMA1_Stream3_IRQHandler,           /* DMA1 Stream 3                */
  DMA1_Stream4_IRQHandler,           /* DMA1 Stream 4                */
  DMA1_Stream5_IRQHandler,           /* DMA1 Stream 5                */
  DMA1_Stream6_IRQHandler,           /* DMA1 Stream 6                */
  ADC_IRQHandler,                    /* ADC1, ADC2 and ADC3s         */
  CAN1_TX_IRQHandler,                /* CAN1 TX                      */
  CAN1_RX0_IRQHandler,               /* CAN1 RX0                     */
  CAN1_RX1_IRQHandler,               /* CAN1 RX1                     */
  CAN1_SCE_IRQHandler,               /* CAN1 SCE                     */
  EXTI9_5_IRQHandler,                /* External Line[9:5]s          */
  TIM1_BRK_TIM9_IRQHandler,          /* TIM1 Break and TIM9          */
  TIM1_UP_TIM10_IRQHandler,          /* TIM1 Update and TIM10        */
  TIM1_TRG_COM_TIM11_IRQHandler,     /* TIM1 Trigger and Commutation and TIM11 */
  TIM1_CC_IRQHandler,                /* TIM1 Capture Compare         */
  TIM2_IRQHandler,                   /* TIM2                         */
  TIM3_IRQHandler,                   /* TIM3                         */
  TIM4_IRQHandler,                   /* TIM4                         */
  I2C1_EV_IRQHandler,                /* I2C1 Event                   */
  I2C1_ER_IRQHandler,                /* I2C1 Error                   */
  I2C2_EV_IRQHandler,                /* I2C2 Event                   */
  I2C2_ER_IRQHandler,                /* I2C2 Error                   */
  SPI1_IRQHandler,                   /* SPI1                         */
  SPI2_IRQHandler,                   /* SPI2                         */
  USART1_IRQHandler,                 /* USART1                       */
  USART2_IRQHandler,                 /* USART2                       */
  USART3_IRQHandler,                 /* USART3                       */
  EXTI15_10_IRQHandler,              /* External Line[15:10]s        */
  RTC_Alarm_IRQHandler,              /* RTC Alarm (A and B) through EXTI Line */
  OTG_FS_WKUP_IRQHandler,            /* USB OTG FS Wakeup through EXTI line */
  TIM8_BRK_TIM12_IRQHandler,         /* TIM8 Break and TIM12         */
  TIM8_UP_TIM13_IRQHandler,          /* TIM8 Update and TIM13        */
  TIM8_TRG_COM_TIM14_IRQHandler,     /* TIM8 Trigger and Commutation and TIM14 */
  TIM8_CC_IRQHandler,                /* TIM8 Capture Compare         */
  DMA1_Stream7_IRQHandler,           /* DMA1 Stream7                 */
  FSMC_IRQHandler,                   /* FSMC                         */
  SDIO_IRQHandler,                   /* SDIO                         */
  TIM5_IRQHandler,                   /* TIM5                         */
  SPI3_IRQHandler,                   /* SPI3                         */
  UART4_IRQHandler,                  /* UART4                        */
  UART5_IRQHandler,                  /* UART5                        */
  TIM6_DAC_IRQHandler,               /* TIM6 and DAC1&2 underrun errors */
  TIM7_IRQHandler,                   /* TIM7                         */
  DMA2_Stream0_IRQHandler,           /* DMA2 Stream 0                */
  DMA2_Stream1_IRQHandler,           /* DMA2 Stream 1                */
  DMA2_Stream2_IRQHandler,           /* DMA2 Stream 2                */
  DMA2_Stream3_IRQHandler,           /* DMA2 Stream 3                */
  DMA2_Stream4_IRQHandler,           /* DMA2 Stream 4                */
  ETH_IRQHandler,                    /* Ethernet                     */
  ETH_WKUP_IRQHandler,               /* Ethernet Wakeup through EXTI line */
  CAN2_TX_IRQHandler,                /* CAN2 TX                      */
  CAN2_RX0_IRQHandler,               /* CAN2 RX0                     */
  CAN2_RX1_IRQHandler,               /* CAN2 RX1                     */
  CAN2_SCE_IRQHandler,               /* CAN2 SCE                     */
  OTG_FS_IRQHandler,                 /* USB OTG FS                   */
  DMA2_Stream5_IRQHandler,           /* DMA2 Stream 5                */
  DMA2_Stream6_IRQHandler,           /* DMA2 Stream 6                */
  DMA2_Stream7_IRQHandler,           /* DMA2 Stream 7                */
  USART6_IRQHandler,                 /* USART6                       */
  I2C3_EV_IRQHandler,                /* I2C3 event                   */
  I2C3_ER_IRQHandler,                /* I2C3 error                   */
  OTG_HS_EP1_OUT_IRQHandler,         /* USB OTG HS End Point 1 Out   */
  OTG_HS_EP1_IN_IRQHandler,          /* USB OTG HS End Point 1 In    */
  OTG_HS_WKUP_IRQHandler,            /* USB OTG HS Wakeup through EXTI */
  OTG_HS_IRQHandler,                 /* USB OTG HS                   */
  DCMI_IRQHandler,                   /* DCMI                         */
  CRYP_IRQHandler,                   /* CRYP crypto                  */
  HASH_RNG_IRQHandler,               /* Hash and Rng                 */
  FPU_IRQHandler                     /* FPU                          */

};

void Reset_Handler (void) {
  register unsigned int *src, *dst, *end;

  /* Zero fill the bss section */
  dst = &_sbss;
  end = &_ebss;
  while (dst < end)  *dst++ = 0U;

  /* Copy data section from flash to RAM */
  src = &_sidata;
  dst = &_sdata;
  end = &_edata;
  while (dst < end) *dst++ = *src++;

  SystemInit();
  SystemCoreClockUpdate();          // Potřebné pro USART
  static_init();                    // Zde zavolám globální konstruktory

  main();

  for (;;);
}


void Default_Handler (void) {
  for (;;);
  //asm volatile ("bkpt 0");
}
