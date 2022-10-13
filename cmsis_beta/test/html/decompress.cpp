#include "decompress.h"
#include "deflate.h"

static std::string * output_string = nullptr;

int xwrite (int fd, char * buf, int len) {
  if (output_string) output_string->append (buf, len);
  return len;
}

Decompress::Decompress() noexcept : out() {
  output_string = & out;
}
std::string & Decompress::get() {
  out.clear();
  cFile ptr = {
    .clen  = filesize,
    .data  = filedata,
  };
  restore_file (&ptr);
  return out;
}
