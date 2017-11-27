#ifndef TONE_H
#define TONE_H

class Tone {
  public:
    Tone ();
    void init ();
    void setMidiOn  (unsigned int m);
    void setMidiOff (void);
    void setFreq    (unsigned int f);
    void setAmpl    (unsigned int a);
    void setFall    (unsigned int f);
    int  step       (void);
  private:
    /// Amplituda tónu, interní proměnná
    unsigned int ampl;
    /// Exponenciální doběh - čím víc, tím rychlejší (0 = stálý)
    unsigned int fall;
    /// Frekvence (normalizovaná)
    unsigned int freq;
    /// Přetékající index do tabulky vzorků
    unsigned int base;
    /// Attack = index do tabulky attackTable
    unsigned int atck;
};

#endif // TONE_H
