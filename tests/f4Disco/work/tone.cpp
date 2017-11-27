#include "tone.h"

/**
 * Přidán attack - zmizí rušivé lupání, prodlouží se obsluha tónu.
 * */

extern "C" const          short onePeriod[];
extern "C" const unsigned short midiTones[];
extern "C" const unsigned int attackTable[];

static const unsigned defFall   = 8;
static const unsigned maxAttack = 127;

Tone::Tone() {
  init();
}
void Tone::init() {
  ampl = 0; freq = 0; base = 0; atck = 0;
  fall = defFall;
}

void Tone::setAmpl (unsigned int a) {
  ampl = a;
}

void Tone::setFreq (unsigned int f) {
  freq = f;
}

void Tone::setMidiOn (unsigned int m) {
  freq = midiTones [m & 0x7F];
  if (freq) atck = maxAttack;
  fall = 1;
}

void Tone::setMidiOff (void) {
  fall = defFall;
}

void Tone::setFall (unsigned int f) {
  fall = f;
}

int Tone::step (void) {
  unsigned int k,x,t;
  int y;
  // Spočteme index x pro přístup do tabulky
  x  = (base >> 8) & 0xFF;
  y  = onePeriod [x];     // vzorek vezmeme z tabulky
  // k je horní půlka amplitudy
  k  = ampl >> 16;
  y  *= k;                // vzorek násobíme amplitudou (tedy tím vrškem)
  y >>= 12;               // a vezmeme jen to, co potřebuje DAC
  k  *= fall;             // Konstanta fall určuje rychlost poklesu amplitudy,
  // čím více, tím je rychlejší. Pokud by bylo 1, pokles je 2^16 vzorků, což už je pomalé.
  base += freq;           // časová základna pro další vzorek

  if (atck) {                   // přidán attack = náběh amplitudy
    t = attackTable [atck];     // z tabulky
    if (t > ampl) ampl = t;     // prevence lupání - nemí být skok amplitudy
    atck -= 1;                  // dočasovat k nule
  } else
    ampl -= k;           // exponenciální pokles amplitudy
  // a je to
  return y;
}

