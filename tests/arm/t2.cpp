#include "test.h"
// Tohle sice neni validni nastaveni USART, ale ukazuje pouziti lambda funkce.
// Krome toho je zrejme, ze pouziti odkazu misto ukazatele nezmeni vysledny kod.
extern "C" void rmw_cpp_1 () {
  USART0.CFG.modify([](USART_CFG_s & r) -> auto {    // load volatile (v headeru)
    r.B.DATALEN   = 1u;                       // modify ...
    r.B.PARITYSEL = 3u;  // ODD
    r.B.ENABLE    = 1u;
    return r.R;                               // store volatile (v headeru)
  });
}

// Jako auto lze pouzit i nazev struktury, odvodi se z prototypu,
// ale moje IDE ma zatim potize automaticky doplnovat nazvy bitu.
extern "C" void wo_cpp_1 () {
  USART0.CFG.setbits([](auto & r) -> auto {   // vstupuje vynulovany registr
    r.B.DATALEN   = 1u;                       // modify ...
    r.B.PARITYSEL = 3u;  // ODD
    r.B.ENABLE    = 1u;
    return r.R;                               // store volatile (v headeru)
  });
}

// Vysledky jsou _uplne_ stejne jako v cistem C. Viz listing.

/** Poznámky k použití typu enum.
 * 1. Soubory *.svd pokud vůbec obsahují enum hodnoty, mají názvy dost divné, takže by se to muselo ručně editovat.
 * 2. Protože není zaručeno, že se názvy nebudou opakovat, bylo by nutné použít enum class, tedy dlouhé názvy.
 * 3. V C++11 a více je možné použít i typ proměnné pro enum (většinou uint32_t), ale protože se to používá
 *    v bitových polích zkrácené, g++ nadává, i když to přeloží správně. Pokud typ nepoužijeme, stejně je pak
 *    problém s short-enums vs. no-short-enums v knihovnách.
 * Zatím je tedy lépe se na to vykašlat, dát to do komentářů a používat čísla. Principiálně by to fungovalo asi
 * takto:
 * */
#ifdef __clang__

#ifdef __cplusplus
      enum struct TEST_CFG_DATALEN : uint32_t {
          B7_BIT_DATA_LENGTH = 0x0u,
          B8_BIT_DATA_LENGTH = 0x1u,
          B9_BIT_DATA_LENGTH = 0x2u,
          RESERVED = 0x3u,
      };
      // ... další enums
#endif // __cplusplus
struct TEST_CFG_s {
  union {
      __RWB uint32_t R;
    struct {
      __RWB uint32_t ENABLE           :  1;
            uint32_t unused0          :  1;
#ifdef __cplusplus
      __RWB TEST_CFG_DATALEN DATALEN  :  2;
#else
      __RWB uint32_t DATALEN          :  2;
#endif // __cplusplus
       // ...
            uint32_t unused1          :  28;
    };
  };
#ifdef __cplusplus
  TEST_CFG_s(): R(0x00000000u) {};
  void setbits (uint32_t (*f) (TEST_CFG_s & r)) volatile {
    TEST_CFG_s t;
    R = f (t);
  }
#endif // __cplusplus
};

extern "C" void xtest (void) {
  __RW TEST_CFG_s t;
  t.setbits([](auto & r) -> auto {
    r.DATALEN = TEST_CFG_DATALEN::B8_BIT_DATA_LENGTH;
    return r.R;
  });
}

#else // !__clang__
extern "C" void xtest (void) {
}
#endif // __clang__