#include <stdio.h>
#include <math.h>
#include "params.h"

static const int maxTone = (1L<<15) - 1;

int limit (double tone) {
  int k = (int) round (tone);
  if (k > maxTone) k = 0;
  return k;
}
int normalize (double val, double scale) {
  return (int) round (val * scale);
}
int main (void) {
  double base, dint;
  int i,j;
  
  base  = 8.1757989156;       // C5 v Hz (http://www.tonalsoft.com/pub/news/pitch-bend.aspx)
#if 0
  base *= (double)(1UL << 16) / 23437.5; // ARM: fs = 48MHz / 2048
#else
  base *= (double)(1UL << 16) / AudioSampleRate;
#endif
  dint  = pow(2.0, 1.0 / 12.0);
  
  FILE* out = fopen ("../miditone.c","w");
  // Tabulka inkrementů pro midi tóny
  fprintf (out, "const unsigned short midiTones[] = {\n");
  for (i=0,j=0; i<127; i++) {
    fprintf (out, "%6d, ", limit (base));
    if (++j >= 12) {
      j = 0;
      fprintf (out, "\n");
    }
    base *= dint;
  }
  fprintf (out, "%6d };\n\n", limit (base));
  // Vzorky pro jednu periodu tónu včetně barvy
  double samples [256], max = 0.0, val;
  base = M_PI / 128.0;
  for (i=0; i<256; i++) {
    val  = 0.0;
    val += 1.0 * sin (1.0 * base * (double) i);
    // Je dobré přidat nějaké harmonické, jinak je tón chudý
    val += 0.3 * sin (2.0 * base * (double) i);
    val += 0.1 * sin (3.0 * base * (double) i);
    // 7. harmonická je nepříjemná, zkuste si to.
    // val += 0.1 * sin (7.0 * base * (double) i);
    if (val > +max) max = +val;
    if (val < -max) max = -val;
    samples [i] = val;
  }
  max = (double)(0x7FF) / max;  // normála -2047 až +2047 (do 12. bitů)
  // mormalizace a výpis
  fprintf (out, "const short onePeriod[] = {\n");
  for (i=0,j=0; i<255; i++) {
    fprintf (out, "%6d, ", normalize (samples[i], max));
    if (++j >= 8) {
      j = 0;
      fprintf (out, "\n");
    }
    base *= dint;
  }
  fprintf (out, "%6d };\n\n", normalize (samples[i], max));
  
  unsigned Attack = maxAmplt;
  fprintf (out, "const unsigned attackTable[] = {\n");
  for (i=0,j=0; i<127; i++) {
    fprintf (out, "0x%08X, ", Attack);
    if (++j >= 8) {
      j = 0;
      fprintf (out, "\n");
    }
    Attack -= Attack / 20;
  }
  fprintf (out, "0x%08X };\n\n", Attack);
  
  
  fclose  (out);
}
