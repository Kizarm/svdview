#include "common.h"

/**
 * rmw_x - read / modify / write na volatilnim registru
 * wo_x  - pouze zapis vice bitu (OR) do volatilniho registru
 * */

extern void rmw_c_1   ();
extern void rmw_c_2   ();
extern void xrmw_c_3  ();
extern void rmw_cpp_1 ();

extern void wo_c_1    ();
extern void wo_c_2    ();
extern void wo_cpp_1  ();

extern void xtest     ();
// Zde jsou ukazatele na funkce - predejde se GC (odstaneni sekci)
const fnc test[] = {    // je to podobne jako tabulka vektoru
  rmw_c_1, rmw_cpp_1, rmw_c_2, xrmw_c_3,
  wo_c_1,  wo_c_2,    wo_cpp_1, xtest,
};
