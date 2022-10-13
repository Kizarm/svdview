#ifndef __SYSTEM_STM32F0XX_H
#define __SYSTEM_STM32F0XX_H

#ifdef __cplusplus
 extern "C" {
#endif 

extern uint32_t SystemCoreClock;          /*!< System Clock Frequency (Core Clock) */
extern void SystemInit(void);
extern void SystemCoreClockUpdate(void);

#ifdef __cplusplus
}
#endif

#endif /*__SYSTEM_STM32F0XX_H */

