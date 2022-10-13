#ifndef UTILS_H
#define UTILS_H
#include <string>
#include <vector>
#include <regex>

#ifdef __EMSCRIPTEN__
#include <sstream>
extern std::ostringstream CERR;
#else
#define CERR cerr
#endif // __EMSCRIPTEN__
struct StringEnumerator {
  const char * const name;
  const unsigned     value;
};
extern std::string cprintf  (const char *__restrict fmt, ...)__attribute__((__format__(__printf__, 1, 2)));
extern void     check_pattern    (const std::string & str, const char * const pattern);
extern unsigned check_enumeration(const std::string & str, StringEnumerator const * enumeration, const size_t len);
extern const unsigned long cmsis_toUlong (const std::string & str);
// vyhází všechny bílé znaky
extern std::string strip_wc (const std::string & str);
// ve výsledku jsou i prázdné stringy - hlídá počet oddělovačů
extern std::vector<std::string> split_string (const std::string & str, const char delim);

#endif // UTILS_H
