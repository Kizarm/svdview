#ifndef _TESTCOMMON_H_
#define _TESTCOMMON_H_
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

typedef void (*fnc) (void);
extern const fnc test[];

#ifdef __cplusplus
};
#endif //__cplusplus
#endif // _TESTCOMMON_H_
