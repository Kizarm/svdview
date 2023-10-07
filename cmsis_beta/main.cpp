#include <fstream>
#include <iostream>
#include <filesystem>
#include "cmsis.h"

int main (int argv, char * argc[]) {
  if (argv < 2) {
    std::cout << "\tusage " << argc[0] << " filename.svd [old | new]\n";
    return 0;
  }
  char * filename = argc [1];
  PRINT_ENUMS pe = DECLARE_ONLY;
  if (argv > 2) {
    std::string par (argc[2]);
    if (par.find_first_of("oO") != std::string::npos) pe = OLD_STYLE;
    if (par.find_first_of("nN") != std::string::npos) pe = NEW_STYLE;
  }
  return create_from_svd (filename, pe);
}
