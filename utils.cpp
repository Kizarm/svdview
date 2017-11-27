#include <QDebug>
#include <stdio.h>
#include "device.h"
#include "utils.h"

const char * ref_width (unsigned long w) {
  if (w == 8)  return "uint8_t ";
  if (w == 16) return "uint16_t";
  if (w == 32) return "uint32_t";
  return "unsigned";
}

QString & l_just (QString & str, const int w) {
  int len = str.length();
  if (len >= w) return str;
  str += indent(w - len);
  return str;
}
QString indent (const unsigned no) {
  QString str;
  for (unsigned n=0; n<no; n++) str += " ";
  return str;
}

ACCESS ref_access (const QString & s) {
  if (s == "read-only")  return READ_ONLY;
  if (s == "read-write") return READ_WRITE;
  if (s == "write-only") return WRITE_ONLY;
  return READ_WRITE;
}
QString & strip (QString & str) {
  str = str.remove("\n");
  str = str.remove("\r");
  return str.replace(QRegExp("[ \\t]+"), " ");
}
void set_bits (const char * range, unsigned long & offset, unsigned long & size) {
  unsigned from, to;
  int n = sscanf (range, "[%d:%d]", &to, &from);
  if (n==2) {
    offset = from;
    size   = to - from + 1;
  } else {
    qDebug("set_bits error");
  }
}

