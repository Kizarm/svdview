#ifndef USART_H
#define USART_H
#include <stdint.h>
#include "fifo.h"
/** @class Usart
 * @brief Sériový port
 */
class Usart {
  FIFO<char, 64> tx_ring;
  public:
    explicit Usart (const uint32_t baud = 9600) noexcept;
    void irq            (void);   // nepoužitelné, ale musí být public
    void SetHalfDuplex  (const bool on) const;
    void SetRS485       (const bool polarity) const;
    void puts           (const char * buf);
  protected:
    uint32_t Down  (const char * data, const uint32_t len);
};

#endif // USART_H
