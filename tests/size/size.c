#define SIZE_TEST
#include "test.h"
#include <stdio.h>

/*
  Program nedělá nic užitečného,
  pouze zjistí, zda vygenerovaná hlavička jde přeložit C i C++ (gcc)
  a vloženým testem zkontroluje délky struktur, popisující registry.
  Mělo by to sedět i na 64.bit architektuře i když není použito
  pakování struktur. Měly by být poskládány tak, aby to nebylo potřeba.
*/

int main (void) {
  int n = size_test();
  if (n) {
    printf ("Errors in size %d\n", n);
    return -1;
  }
  printf ("OK.\n");
  return 0;
}
