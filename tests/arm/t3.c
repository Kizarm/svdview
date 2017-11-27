  #include "stat.h"   // hlavicka se statickymi konstantami
//#include "defs.h"   // klasicka hlavicka
// Klasicky zapis
// Pro CM3,4 clang i gcc je vysledny kod stejny jako predchozi, u CM0 a gcc ku podivu o neco delsi.
void rmw_c_2 () {
  uint32_t r = USART0->CFG;         // load volatile
  r &= ~USART_CFG_DATALEN_MASK      // vynulovat pouzita pole
    &  ~USART_CFG_PARITYSEL_MASK    // zbytecne (nastavuji pak plnou sirku), ale nejaky bic tam stejne je,
    &  ~USART_CFG_ENABLE;           // take zbytecne, ale aby byl stejny kod
  r |= (1u << USART_CFG_DATALEN_BITS)               // modify -> set bits
    |  (3u << USART_CFG_PARITYSEL_BITS)
    |  USART_CFG_ENABLE;
  USART0->CFG = r;                  // store volatile
}
// Klasicky zapis po RESETu
void wo_c_2 () {
  USART0->CFG  = (1u << USART_CFG_DATALEN_BITS)     // set bits
              |  (3u << USART_CFG_PARITYSEL_BITS)   // zde jsou ty masky zbytecne
              |  USART_CFG_ENABLE;
}
