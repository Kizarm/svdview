#ifndef AUDIODMA_H
#define AUDIODMA_H

#define Audio8000HzSettings 256,5,12,1
#define Audio16000HzSettings 213,2,13,0
#define Audio32000HzSettings 213,2,6,1
#define Audio48000HzSettings 258,3,3,1
#define Audio96000HzSettings 344,2,3,1
#define Audio22050HzSettings 429,4,9,1
#define Audio44100HzSettings 271,2,6,0
#define AudioVGAHSyncSettings 419,2,13,0 // 31475.3606. Actual VGA timer is 31472.4616.

#include <stdint.h>
#include "oneway.h"
// Délka buferu zde není příliš důležitá.
static const unsigned DmaBufLen = 0x100;

class AudioDma : public OneWay {

  public:
    AudioDma       ();
    void SetVolume (int volume);
    void Turn      (bool on);
    void irq       (void);
  protected:
    void WriteRegister (uint8_t address, uint8_t value);
  private:
    Sample   buf [2 * DmaBufLen];
    Sample * pbuf0;
    Sample * pbuf1;
};

#endif // AUDIODMA_H
