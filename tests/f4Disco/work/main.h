#ifndef MAIN_H
#define MAIN_H

#include "gpio.h"
#include "midiplayer.h"
#include "efect.h"

/** 
 * 
 * @author  Mrazik
 * @version V1.0.b
 * @date    26-11-2017
 
   @section Preface Orchestrion
   
   Přepis <a href="http://mcu.cz/news3615.html.3">staršího projektu</a> pomocí
   generované hlavičky mcu. Je to jen pro F4 Discovery, hlavička je generována
   z tohoto <a href="https://github.com/posborne/cmsis-svd/blob/master/data/STMicro/STM32F40x.svd">souboru</a>.
   
   Z kódu v adresáři ./src je zřejmé, že tento generovaný předpis je s mírnými úpravami použitelný
   i pro rozsáhlejší projekty, pokud se vrstva pracující přímo s hardware dobře oddělí od ostatního.
   Experimentálně jsou použity i některé novější konstrukce v C++ (lambda, for). Prostě fungují.
 
 * @file main.h
 * @brief Include pro main.cpp
 * */

#if defined (__cplusplus)
extern "C" {
#endif
  /// Tabulka vektorů je v čistém C
  extern  void SysTick_Handler (void);
  /// A volání main ostatně taky
  extern  int  main  (void);
#if defined (__cplusplus)
}
#endif


#endif // MAIN_H