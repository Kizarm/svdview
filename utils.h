#ifndef DEV_UTILS_H
#define DEV_UTILS_H
#include "device.h"

extern void set_bits (const char * range, unsigned long & offset, unsigned long & size);
extern QString & strip (QString & str);
extern ACCESS ref_access (const QString & s);
extern QString indent (const unsigned no);
extern QString & l_just (QString & str, const int w);
extern const char * ref_width (unsigned long w);

#endif // DEV_UTILS_H

