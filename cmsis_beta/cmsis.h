#ifndef _CMSIS_H_
#define _CMSIS_H_

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus
  enum PRINT_ENUMS {
  DECLARE_ONLY = 0,
  OLD_STYLE, NEW_STYLE,
  };

  extern int create_from_svd (const char * filename, const enum PRINT_ENUMS pe);
#ifdef __cplusplus
};
#endif //__cplusplus

#endif // _CMSIS_H_

