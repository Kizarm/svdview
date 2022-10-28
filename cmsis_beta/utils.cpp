#include <iostream>
#include <cstdarg>
#include "utils.h"

using namespace std;

string cprintf (const char *__restrict fmt, ...) {
  const int len = 0x1000;
  char buffer [len];
  va_list arg;
  va_start (arg, fmt);
  vsnprintf(buffer, len, fmt, arg);
  va_end (arg);
  return string (buffer);
}
void check_pattern(const string & str, const char * const pattern) {
  const regex str_expr (pattern);
  const bool result = regex_match (str, str_expr);
  if (!result) CERR << "\"" << str << "\" pattern error " << pattern << '\n';
}
unsigned check_enumeration(const string & str, StringEnumerator const * enumeration, const size_t len) {
  unsigned result = 0;
  for (unsigned n=0; n<len; n++) {
    const char * p = enumeration[n].name;
    if (str == p) {
      result = enumeration[n].value;
      break;
    }
  }
  if (!result) CERR << "enumeration error \"" << str << "\" not in list\n";
  return result;
}
const unsigned long cmsis_toUlong (const string & str) {
  int nb = 10, n = 0;
  char copy [256]; 
  for (auto c: str) {
    if (c == 'X') {
      nb = 16;
    } else if (c == 'x') {
      nb = 16;
    } else if (c == '#') {
      nb = 2;
    } else {
      copy [n++] = c;
    }
  }
  copy [n] = '\0';
  char * endptr = nullptr;
  long long result = strtoll (copy, &endptr, nb);
  string end (endptr);
  const string x ("kmgt");
  if (end.size() > 0u) {
    const char m = endptr[0] | ' ';
    int index = 0;
    for (const char c: x) {
      if (c == m) break;
      index += 1;
    }
    if (index < (int) x.size()) {
      result <<= (index + 1) * 10;
    }
  }
  return result;
}
string strip_wc(const string & str) {
  string result;
  int count = 0;
  for (char c: str) {
    if (c == '\n') continue;
    if (c == '\r') continue;
    if (c ==  ' ' or c == '\t') {
      count += 1;  continue;
    }
    if (count) {
      count = 0;
      result += ' ';
    }
    result += c;
  }
  return result;
}
vector<string> split_string (const string & str, const char delim) {
  vector<string> result;
  string s;
  for (auto c: str) {
    if (c == delim) {
      result.push_back (s);
      s.clear();
    } else {
      s += c;
    }
  }
  result.push_back (s);
  return result;
}

