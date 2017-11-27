#ifndef DACPLAYER_H
#define DACPLAYER_H
#include "params.h"
#include "audiodma.h"
#include "oneway.h"
#include "tone.h"

bool PlatformMainLoop (void);
/// Prototyp pro funkci ukončení melodie.
typedef void (*callBackFunc) (void);
/// Třída, která hraje čistě na pozadí.
class MidiPlayer : public OneWay {
  // Veřejné metody
  public:
    /// Konstruktor
    MidiPlayer ();
    uint32_t    Up   (Sample * data, uint32_t len);
    /// Start melodie v poli score
    void start  (unsigned char const* score);
    /// Pokud je potřeba vědět, kdy je konec
    void setEnd (callBackFunc fnc);
  // Chráněné metody
  protected:
    void ToneChange    (void);
    /// Obsluha vzorku
    int16_t nextSample (void);
  private:
    /// Výstup
    AudioDma       audio;
    /// Generátory tónů
    Tone           gens [maxGens];
    /// vnitřní ukazatel na tóny
    unsigned char const * melody;
    volatile int   pause;    //!< Časování
    callBackFunc   func;     //!< Uživatelská funkce na konci melodie
};

#endif // DACPLAYER_H
