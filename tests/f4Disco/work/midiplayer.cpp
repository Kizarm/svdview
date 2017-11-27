#include "midiplayer.h"
#include "gpio.h"

/**
 * @file
 * @brief Jednoduchý přehrávač midi souborů.
 * 
 * Kompletní midi obsahuje zvukové fonty, které jsou obrovské. Tohle je velice zjednodušené,
 * takže docela dobře přehrává skladby typu ragtime, orchestrální midi jsou skoro nepoužitelné.
 * Přesto se to pro jednoduché zvuky může hodit, protože je to poměrně nenáročné na systémové
 * prostředky. Může to fungovat dokonce i na 8-bitovém uP.
 * */
//static Tone  gens [maxGens];

/// Konstruktor
MidiPlayer::MidiPlayer() : OneWay(), audio() {
  pause = 0;
  for (auto & t : gens) t.init();
  audio.SetVolume (245);
  func = 0;
  *this += audio;
}
/// Počítá další vzorek
int16_t MidiPlayer::nextSample (void) {
  if (pause) pause -= 1;  // Časování tónu
  else ToneChange();      // Nový tón
    
  int res = 0;
  for (auto & g : gens) res += g.step();
  // Pro jistotu omezíme - předejdeme chrastění
  if (res > maxValue) res = maxValue;
  if (res < minValue) res = minValue;
  return (int16_t) res;
}
/// Předává data do nižší vrstvy - audio pomocí DMA, tedy bloky.
uint32_t MidiPlayer::Up (Sample * data, uint32_t len) {
  for (unsigned n=0; n<len; n++) {
    Sample  s;
    int16_t t = nextSample();
    s.ss.l = t;
    s.ss.r = t;
    data [n] = s;
  }
  return len;
}

/// Nastavení callbacku pro ukončení playeru
void MidiPlayer::setEnd (callBackFunc fnc) {
  func = fnc;
}
/// Spušťění přehrávání
void MidiPlayer::start (unsigned char const* score) {
  melody = score;
  audio.Turn(true);   // audio spustit až zde !!!
}
/// Voláno fakticky pomocí soft.Intr()
void MidiPlayer::ToneChange (void) {
  unsigned char cmd, midt;
  unsigned int  geno;
  
  for (;;) {              // Pro všechny tóny před pauzou
    cmd = *melody++;
    if (cmd & 0x80) {     // event
      geno  = cmd & 0x0F;
      cmd >>= 4;
      switch (cmd) {
        case 0x8:         // off
          gens[geno].setMidiOff();
          break;
        case 0x9:         // on
          midt = *melody++;
          gens[geno].setMidiOn (midt); 
          break;
        default:
          audio.Turn(false);
          if (func) func ();
          return;         // melodie končí eventem 0xf0
      }
    } else {              // pause
      midt   = *melody++;
      pause  = ((unsigned int) cmd << 8) + midt;  // v ms
      pause *= AudioMidiDelay;        // ale máme vzorkování cca 44 kHz 
      return;
    }
  }
}
