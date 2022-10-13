#ifndef DECOMPRESS_H
#define DECOMPRESS_H

#include <string>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
extern const char  filedata [];
extern const int   filesize;
extern int xwrite (int fd, char * buf, int len);
#ifdef __cplusplus
};
#endif // __cplusplus

class Decompress {
  std::string out;
  public:
    explicit Decompress() noexcept;
    std::string & get ();
};

#endif // DECOMPRESS_H
