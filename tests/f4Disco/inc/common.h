#ifndef COMMON_F4_H
#define COMMON_F4_H
/**
 * @brief Configuration of the Cortex-M4 Processor and Core Peripherals 
 */
#define __CM4_REV                 0x0001  /*!< Core revision r0p1                            */
#define __MPU_PRESENT             1       /*!< STM32F4XX provides an MPU                     */
#define __NVIC_PRIO_BITS          4       /*!< STM32F4XX uses 4 Bits for the Priority Levels */
#define __Vendor_SysTickConfig    0       /*!< Set to 1 if different SysTick Config is used  */

#if !defined  (__FPU_PRESENT) 
  #define __FPU_PRESENT           1       /*!< FPU present                                   */
#endif /* __FPU_PRESENT */

#if !defined  (HSE_VALUE) 
  #define HSE_VALUE    ((uint32_t) 8000000) /*!< Value of the External oscillator in Hz */
#endif /* HSE_VALUE */

/**
 * @brief In the following line adjust the External High Speed oscillator (HSE) Startup 
   Timeout value 
   */
#if !defined  (HSE_STARTUP_TIMEOUT) 
  #define HSE_STARTUP_TIMEOUT    ((uint16_t)0x0500)   /*!< Time out for HSE start up */
#endif /* HSE_STARTUP_TIMEOUT */   

#if !defined  (HSI_VALUE)   
  #define HSI_VALUE    ((uint32_t)16000000) /*!< Value of the Internal oscillator in Hz*/
#endif /* HSI_VALUE */   

#endif // COMMON_F4_H
