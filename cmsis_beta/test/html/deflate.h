#ifndef PREFIX_H
#define PREFIX_H
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
typedef struct {
  int          clen;
  const char * data;
} cFile;
extern void restore_file (const cFile * ptr);

#ifdef __cplusplus
};
#endif // __cplusplus
#endif // PREFIX_H

