#include <string>
#include <vector>
#include "utils.h"
#include "PrinterHTML.h"
using namespace std;
static const char * classes [] = {" class=\"un\""," class=\"ro\""," class=\"wo\""," class=\"rw\""," class=\"wo\""," class=\"rw\""};

void PrinterHTML::save(const char * name) {
}
void PrinterHTML::fillFieldGaps() {
  for (auto & pp: peripherals) {
    //pp.checkNames();
    for (auto & rr: pp.registers) {
      //rr.checkNames();
      rr.fillGaps ();
    }
    sort (pp.registers.begin(), pp.registers.end(), [] (RegisterPart & a, RegisterPart & b) {
      return a.address < b.address;                 // podle adresy (ofsetu)
    });
  }
  sort (peripherals.begin(), peripherals.end(), [] (PeripheralPart & a, PeripheralPart & b) {
    return a.name < b.name;                         // tady lépe podle jména
  });
}

void PrinterHTML::dumpPeripherals(string & out) {                 // left table
  fillFieldGaps();  // tady není dobré udělat plnou validaci, mezery mezi registry nejsou potřeba
                    // není dobré dělat uniony a jména fields UNUSED budou prázdná
  finalIndexing();
  out += cprintf("<p><b>%s</b></p>\n", name.c_str());
  out += "<table>\n";
  out += "  <tr><td>Peripheral</td></tr>\n";
  for (auto & p: peripherals) {
    out += cprintf ("  <tr><td class=\"gr\" onclick=\"Register(%d);\">%s</td></tr>\n", p.part_id, p.name.c_str()); 
  }
  out += "</table>\n";
}
void PrinterHTML::dumpPeripheral(string & out, const int id) {    // right table
  if (peripherals.size() <= (size_t) id) return;
  string sder, info;
  const PeripheralPart & pp = peripherals [id];
  if (pp.baseName.size()) sder = cprintf(" , derived from <b>%s</b>", pp.baseName.c_str());
  if (pp.comment.size())  info = cprintf("<i>%s</i>", pp.comment.c_str());
  out += "<div align=\"left\">\n";
  out += cprintf("<p>Peripheral: <b>%s</b> Address = 0x%08lx %s</p><p>%s</p>\n", pp.name.c_str(), pp.address, sder.c_str(), info.c_str());
  out += "</div>\n";
  
  if (pp.registers.empty()) return;
  out += cprintf("<table class=\"regs\">\n");
  
  string header, s;
  for (unsigned i=0; i<32; i++) {
    s += cprintf("<td>%02d</td>", i);
  }
  header += cprintf(" <tr><td>name</td><td>offset</td>%s</tr>\n", s.c_str());
  unsigned n = 0u;
  for (auto & r: pp.registers) {
    if ((n & 0x0Fu) == 0u) out += header; 
    dumpRegister (r, out, pp.part_id);
    n += 1;
  }
  out += cprintf("</table>\n");
}
void PrinterHTML::dumpRegister(const RegisterPart & r, string & out, const int per) {
  out += cprintf(" <tr><td%s onclick=\"RegisterDesc(%d,%d);\">%s</td><td>%04lX</td>\n", classes[r.access],
                 per, r.part_id, r.baseName.c_str(), r.address);
  for (const auto & f: r.fields) {
    dumpField (f, out, per, r.part_id);
  }
  out += cprintf("</tr>\n");
}
void PrinterHTML::dumpField(const FieldPart & f, string & out, const int per, const int reg) {
  string s, nm;
  if (!f.unused) nm = f.name;
  if (f.size > 1) s = cprintf(" colspan=\"%lu\"", f.size);
  out += cprintf("<td%s%s onclick=\"Field(%d, %d, %d);\">%s</td>\n", s.c_str(), classes[f.access],
                 per, reg, f.part_id, nm.c_str());
}
void PrinterHTML::dumpDescription(string & out, const int per, const int reg) { // Register
  const PeripheralPart & pp = peripherals  [per];
  const RegisterPart   & rr = pp.registers [reg];
  
  out += "<div align=\"left\">\n";
  out += cprintf("<p>Register: <b>%s</b>&nbsp; Offset: 0x%04lx bytes, size = %ld of elements width %d bits</p>\n",
                 rr.baseName.c_str(), rr.address, rr.size, rr.width << 3);
  if (!rr.name.empty()) {
    string cp;
    for (const char c: rr.name) {
      cp += c;
      if (c == ',') cp += ' ';      // nejsou tam mezery, doplň je
    }
    out += cprintf("<p> eval : %s</p>\n", cp.c_str());
  }
  out += cprintf("<p><i>%s</i></p>\n", rr.comment.c_str());
  out += "</div>\n";
}

void PrinterHTML::dumpDescription(string & out, const int per, const int reg, const int fie) { // Field
  const PeripheralPart & pp = peripherals  [per];
  const RegisterPart   & rr = pp.registers [reg];
  const FieldPart      & ff = rr.fields    [fie];
  
  out += "<div align=\"left\">\n";
  out += cprintf("<p>Field: <b>%s</b>&nbsp; Offset: %ld bits, size = %ld bits</p>\n",
                 ff.name.c_str(), ff.address, ff.size);
  out += cprintf("<p><i>%s</i></p>\n", ff.comment.c_str());
  out += "</div>\n";
  if (ff.eenum.values.empty()) return;
  dumpEnums (out, ff.eenum.values);
}
void PrinterHTML::dumpEnums(string & out, const vector<EnumValuesPart> & values) {
  out += "<table>\n";
  out += "<tr><td> Name </td><td> Value </td><td> Comment </td></tr>\n";
  for (auto & part: values) {
    out += cprintf("<tr><td> %s </td><td> %s </td><td><i> %s </i></td></tr>\n",
                   part.name.c_str(), part.value.c_str(), part.comment.c_str());
  }
  out += "</table>\n";
}
