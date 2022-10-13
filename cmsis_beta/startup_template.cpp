#include <fstream>
#include <algorithm>
#include "utils.h"
#include "PrinterHpp.h"
using namespace std;

static const char * template1 = R"---(#include <stdint.h>

#if defined (__cplusplus)
extern "C" {
#endif
extern void (*__init_array_start)();
extern void (*__init_array_end)  ();
void static_init() {
  void (**p)();
  for (p = &__init_array_start; p < &__init_array_end; p++) (*p)();
}
#define WEAK     __attribute__ ((weak))
#define ALIAS(f) __attribute__ ((weak, alias (#f)))

extern unsigned int _estack;
extern unsigned int _sidata;
extern unsigned int _sdata;
extern unsigned int _edata;
extern unsigned int _sbss;
extern unsigned int _ebss;

WEAK void Reset_Handler     (void);
WEAK void DefaultHandler    (void);
)---";
static const char * template3 = R"---(
extern int  main (void);
extern void SystemInit (void);
extern void SystemCoreClockUpdate (void);

#if defined (__cplusplus)
}; // extern "C"
#endif
typedef void (*handler) (void);
__attribute__ ((section(".isr_vector")))
handler Vectors[] = {
  (handler) &_estack,
)---";
static const char * template5 = R"---(
};
void Reset_Handler(void) {
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
  asm volatile ("bkpt 1");
}
)---";

void PrinterHpp::StartupTemplate(string & name) {
  string out;
  out += template1;
  size_t maxlen = 0lu;
  for (auto & e: interrupts) {
    if (e.name.size() > maxlen) maxlen = e.name.size();
  }
  for (auto & e: interrupts) {
    if (e.value < -14) continue;
    int fill = maxlen - e.name.size();
    if (e.value == 0l) out += '\n';
    if (e.value <  0l) out += cprintf ("void %s_Handler %*s   (void) ALIAS(Default_Handler);\n", e.name.c_str(), fill, "");
    else               out += cprintf ("void %s_IRQHandler %*s(void) ALIAS(Default_Handler);\n", e.name.c_str(), fill, "");
  }
  out += template3;
  long count = interrupts[0].value;
  for (auto & e: interrupts) {
    while (e.value > count) {
      out += "  0,\n";
      count += 1l;
    }
    if (e.value == 0l) out += '\n';
    if (e.value <  0l) out += cprintf("  %s_Handler,\n", e.name.c_str());
    else               out += cprintf("  %s_IRQHandler,\n", e.name.c_str());
    count += 1l;
  }
  out += template5;
  const string filename = name + "_startup.c";
  printf ("Create %s\n", filename.c_str());
  ofstream file (filename);
  file << out;
}


