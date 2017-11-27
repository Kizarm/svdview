#ifndef ONEWAY_H
#define ONEWAY_H
#include <stdint.h>
#include <stdlib.h>

union Sample {
  uint32_t u;
  struct {
    int16_t l;
    int16_t r;
  } ss;
  struct {
    uint16_t l;
    uint16_t r;
  } us;
};

class OneWay {
  public:
    OneWay() { pUp = NULL; };
    virtual uint32_t    Up   (Sample * data, uint32_t len) {
      if (pUp) return pUp->Up (data, len);
      return 0;
    };
    virtual OneWay & operator += (OneWay& bl) {
      bl.setUp (this);  // ta spodní bude volat při Up tuto třídu
      return *this;
    };
  protected:
    void setUp   (OneWay * p) { pUp   = p; };
  private:
    OneWay * pUp;
};

#endif // ONEWAY_H
