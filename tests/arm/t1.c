#include "test.h"               // hlavicka je pouzitelna jak pro std. C, tak C++11
// Stejne jako v C++ lambda funkce. Jak pro gcc tak pro clang.
void rmw_c_1 () {
  struct USART_CFG_s r;
  r.R = USART0->CFG.R;          // load volatile
  r.B.DATALEN   = 1u;           // modify ...
  r.B.PARITYSEL = 3u;           // ODD
  r.B.ENABLE    = 1u;
  USART0->CFG.R = r.R;          // store volatile
}
// Modifikace po polozkach bude take fungovat, ale
// po kazde modifikaci se provede zapis do registru,
// coz je zbytecne a prodluzuje kod.
void xrmw_c_3 () {
  USART0->CFG.B.DATALEN   = 1u;
  USART0->CFG.B.PARITYSEL = 3u;  // ODD
  USART0->CFG.B.ENABLE    = 1u;
}

void wo_c_1 () {
  struct USART_CFG_s r;
  r.R = 0u;                     // je otazka, co je tam default po resetu
  r.B.DATALEN   = 1u;           // modify ...
  r.B.PARITYSEL = 3u;           // ODD
  r.B.ENABLE    = 1u;
  USART0->CFG.R = r.R;          // store volatile
}
