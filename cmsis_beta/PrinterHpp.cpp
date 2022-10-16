#include <fstream>
#include <algorithm>
#include "utils.h"
#include "PrinterHpp.h"
/** NEWSTYLE
 * Při překladu bitových polí, které obsahují enum nastávají problémy.
 * Pro arm-none-eabi-g++ verze menší než 9.3 nejde použít u enum typ,
 * vyplyve to spoustu varování. Pokud ho nepoužijeme, zůstane ale zachována
 * dělka struktur.
 * Na druhou stranu u novějších překladačů je nutno typ použít, jinak se
 * délky rozlezou. Jde to obejít -fshort-union, ale může to kolidovat
 * s ostatními částmi programu. Je tu tedy na výběr.
 * */
using namespace std;

static const char * const preamble = R"+-+(
/* IO definitions (access restrictions to peripheral registers) */
/** defines 'read only'    permissions */
#define __I volatile
/** defines 'write only'   permissions */
#define __O volatile
/** defines 'read / write' permissions */
#define __IO volatile
#include <stdint.h>
#define MERGE union
)+-+";
static const char * const accessStrings[] = {
  "      ", // unspecified
  "  __I ",
  "  __O ",
  "  __IO",
  "  __O ", // writeOnce
  "  __IO", // read-writeOnce
};
static const char * typeNames [] = {
 "BYTE ",  "uint8_t  ", "uint16_t ", "uint24_t ", "uint32_t ",
};
/*
static const char * test_text = R"---(#include "%s"
int main () {
  return 0;
}
)---";*/
static string insert_spaces (const int spaces) {
  string result;
  if (spaces <= 0) return result;
  for (int n=0; n<spaces; n++) result += " ";
  return result;
}

void PrinterHpp::save(const char * fname) {
  validate();   // validace pro C++ hlavičku
  string out;
  print (out);
  if (fname) {
    ofstream file (fname);
    file << out;
  } else {
    ofstream file (headerName);
    file << out; 
  }
  StartupTemplate (name);
}
void PrinterHpp::print(string & out) {
  out += cprintf("#ifndef %s_HDEF\n", name.c_str());
  out += cprintf("#define %s_HDEF\n", name.c_str());
  out += cprintf("/** @brief %s */\n", name.c_str());
  out += cprintf("/* %s */\n", comment.c_str());
  out += preamble;
  printCpu (out);
  switch (m_eprt) {
    case DECLARE_ONLY:
    case OLD_STYLE:    out += "enum ONE_BIT { RESET = 0, SET = 1 };\n";            break; 
    case NEW_STYLE:    out += "enum ONE_BIT : uint32_t { RESET = 0, SET = 1 };\n"; break;
  }
  size_t maxl = 0u;
  for (auto & p: peripherals) {
    const size_t l = p.name.size();
    if (l > maxl) maxl = l;
  }
  for (auto & p: peripherals) {
    if (p.baseName.size()) continue;
    printPerpheralDef(p, out, maxl);
    // print registers
    printRegisters (p, out);
    out += cprintf("}; /* total size = 0x%04lx, struct size = 0x%04lX */\n", p.size, p.struct_len);
  }
  // seřaď podle adresy
  sort (peripherals.begin(), peripherals.end(), [] (PeripheralPart & a, PeripheralPart & b) {
    return a.address < b.address;
    
  });
  for (auto & p: peripherals) {
    printPerpheralAddress (p, out, maxl);
  }
  printStaticAsserts (out, 0);
  printInterrupts    (out, 0);
  out += "#endif\n";
}
void PrinterHpp::printCpu(string & out) {
  string cmnt;
  if (!cpu.name.empty() or !cpu.revision.empty() or !cpu.endian.empty()) {
    cmnt += "/*\n";
    if (!cpu.name.empty())     cmnt += cprintf("  cpuName  = %s\n", cpu.name.c_str());
    if (!cpu.revision.empty()) cmnt += cprintf("  revision = %s\n", cpu.revision.c_str());
    if (!cpu.endian.empty())   cmnt += cprintf("  endian   = %s\n", cpu.endian.c_str());
    cmnt += "*/\n";
  }
  out += cmnt;
  out += cprintf("#define __MPU_PRESENT             %s\n", cpu.mpuPresent ? "1" : "0");
  out += cprintf("#define __NVIC_PRIO_BITS          %d\n", cpu.nvicPrioBits);
  out += cprintf("#define __Vendor_SysTickConfig    %s\n", cpu.vendorSystickConfig ? "1" : "0");
  out += cprintf("#define __FPU_PRESENT             %s\n", cpu.fpuPresent ? "1" : "0");
}

void PrinterHpp::printInterrupts(string & out, const int indent) {
  size_t maxlen = 0u;
  for (auto & i: interrupts) {  // determine maximum lenght of names
    if (i.name.size() > maxlen) maxlen = i.name.size();
  }
  out += "\n/* Interrupts : conflicts cmsis part - use copy / paste */\n";
  out += "enum IRQn {\n";
  for (auto & i: interrupts) {
    if (i.value < -14l) continue;
    if (i.value ==  0l) out += '\n';
    unsigned fill = maxlen - i.name.size();
    out += cprintf("  %s_IRQn%*s = %3ld, //!< %s\n", i.name.c_str(), fill, "", i.value, i.comment.c_str());
  }
  out += "};\n";
}

void PrinterHpp::printStaticAsserts(string & out, const int indent) {
  out += '\n';
  for (auto & p: peripherals) {
    if (!p.struct_len) continue;
    string def;
    if (p.baseName.empty()) def = p.name;
    else                    def = p.baseName;
    out += cprintf ("static_assert (sizeof(struct %s_DEF) == %ld, \"size error %s\");\n", def.c_str(), p.struct_len, p.name.c_str());
  }
}

void PrinterHpp::printPerpheralDef(PeripheralPart & p, string & out, const int indent) {
  string def (p.name);
  out += cprintf("\n// ////////////////////+++ %s +-+//////////////////// //\n", def.c_str());
  const int spaces = indent - p.name.size();
  def += "_DEF";
  def += insert_spaces (spaces);
  const char * dn = def.c_str();
  out += cprintf("struct %s { /*!< %s */\n", dn, p.comment.c_str());
}

void PrinterHpp::printPerpheralAddress(PeripheralPart & p, string & out, const int indent) {
  string sn (p.name);
  sn += insert_spaces (indent - sn.size());
  const char * nm = sn.c_str();
  string def;
  if (p.baseName.empty()) def = p.name;
  else                    def = p.baseName;
  const int spaces = indent - def.size();
  def += "_DEF";
  def += insert_spaces (spaces);
  const char * dn = def.c_str();
  out += cprintf("static %s & %s = * reinterpret_cast<%s *> (0x%08lx);\n", dn, nm, dn, p.address);
}
void PrinterHpp::printRegisters(PeripheralPart & p, string & out) {
  size_t maxlen = 0u;
  for (auto & r: p.registers) {
    const size_t len = r.baseName.size();
    if (len > maxlen) maxlen = len;
  }
  for (auto & r: p.registers) {
    printRegDef  (r, out, maxlen);
  }
  string def;
  if (p.baseName.empty()) def = p.name;
  else                    def = p.baseName;
  out += cprintf("  // PERIPHERAL %s REGISTERS INSTANCES\n", def.c_str());
  for (auto & r: p.registers) {
    printRegInst (r, out, maxlen);
  }
}
void PrinterHpp::printRegDef(RegisterPart & r, string & out, const int indent) {
  if (r.unused)         return;
  if (r.fields.empty()) return;
  const string reg = r.baseName;
  const string regdef = reg + "_DEF";
  const int fill = indent - reg.size ();
  out += cprintf("  union %s%*s {  //!< %s\n", regdef.c_str(), fill, "", r.comment.c_str());
  // vypiš enumerations, pokud existují
  printEnumerations (r, out);
  out += cprintf("    struct {\n");
  printFields (r, out);
  out += cprintf("    } B;\n");
  const char * type_name = typeNames [r.width];
  out += cprintf("  %s %s R;\n", accessStrings[r.access], type_name);
  if (r.width == TYPE_32BIT) printMethods (regdef, r.access, r.resetValue, out);
  out += cprintf("  };\n");
  if (r.reg_union.empty()) return;
  for (auto & e: r.reg_union) printRegDef (e, out, indent);
}
void PrinterHpp::printRegInst(RegisterPart & r, string & out, const int indent) {
  if (r.reg_union.empty()) {
    printRegSimple (r, out, indent);
    return;
  }
  out += "  MERGE {\n";
  printRegSimple (r, out, indent);
  for (auto & e: r.reg_union) printRegSimple (e, out, indent);
  out += "  };\n";
}
void PrinterHpp::printRegSimple(RegisterPart & r, string & out, const int indent) {
  string regdef = r.baseName + "_DEF";
  string   fs, reg = r.name;
  if (r.fields.empty())  regdef = typeNames[r.width];
  if (reg.empty())       reg    = r.baseName;           // TODO asi nekonzistence
  if (r.size > 1ul) {
    reg = r.baseName; // pole
    fs = cprintf("[%ld]", r.size);
  }
  const int fill = indent - reg.size ();
  out += cprintf("%s %s%*s %s %s;  //!< [%04lx](%02lx)[0x%08lX]\n", accessStrings[r.access],
                 regdef.c_str(), fill, "", reg.c_str(), fs.c_str(),r.address, r.width * r.size, r.resetValue);
}
void PrinterHpp::printEnumerations(RegisterPart & r, string & out) {
  for (auto & f: r.fields) {
    if (f.eenum.values.empty()) continue;
    switch (m_eprt) {
      case DECLARE_ONLY:
      case OLD_STYLE: out += cprintf("    enum %s /* %s*/ {\n", f.eenum.name.c_str(), typeNames[f.width]); break;
      case NEW_STYLE: out += cprintf("    enum %s :  %s   {\n", f.eenum.name.c_str(), typeNames[f.width]); break;
    }
    size_t maxlen = 0u;
    for (auto & e: f.eenum.values) {    // oprava - e.value musí být číslo - vyskytují se všechny typy cmsis (0x, #)
      string s = cprintf ("%s = %ld", e.name.c_str(), cmsis_toUlong(e.value));
      if (s.size() > maxlen) maxlen = s.size();
    }
    for (auto & e: f.eenum.values) {
      string s = cprintf ("%s = %ld", e.name.c_str(), cmsis_toUlong(e.value));
      int fill = maxlen - s.size();
      out += cprintf ("      %s,%*s  //!< %s\n", s.c_str(), fill, "", e.comment.c_str());
    }
    out += "    };\n";
  }
}
string PrinterHpp::determine_type(FieldPart& f) {
  string result;
  if (m_eprt == DECLARE_ONLY) {
    result = string (typeNames[f.width]);
    return result;
  }
  if (f.eenum.values.empty()) {
    if ((f.size == 1ul) and (f.width == TYPE_32BIT)) result = string ("ONE_BIT");
    else                                             result = string (typeNames[f.width]);
    return result;
  }
  result = f.eenum.name;
  return result;
}
size_t PrinterHpp::determine_type_len (RegisterPart & r) {
  size_t maxlen = 0u;
  for (auto & f: r.fields) {
    const string type_str = determine_type (f);
    if (type_str.size() > maxlen) maxlen = type_str.size();
  }
  return maxlen;
}
void PrinterHpp::printFields(RegisterPart & r, string & out) {
  string res;
  size_t maxlen = 0ul;
  for (auto & f: r.fields) {
    if (f.name.size() > maxlen) maxlen = f.name.size();
  }
  for (auto & f: r.fields) {
    const int fill0 = maxlen - f.name.size();
    const string type_str = determine_type (f);
    const int fill1 = determine_type_len (r) - type_str.size();
    // printf("%d,%d,%d - %s - %s\n", fill0, fill1, f.access, r.name.c_str(), f.name.c_str());
    res += cprintf("    %s %s %*s %s %*s: %2ld; //!<[%02ld] %s\n", accessStrings[f.access], type_str.c_str(), fill1, "",
                   f.name.c_str(), fill0, "", f.size, f.address, f.comment.c_str());
  }
  out += res;
}
static const char * const fmt_wo = R"---(
    explicit %s () noexcept { R = 0x%08lxu; }
    template<typename F> void setbit (F f) volatile {
      %s r;
      R = f (r);
    }
)---";
static const char * const fmt_rw = R"---(
    template<typename F> void modify (F f) volatile {
      %s r; r.R = R;
      R = f (r);
    }
)---";
void PrinterHpp::printMethods(const string & regdef, const unsigned int access, const unsigned long resetValue, string & out) {
  const char * defptr = regdef.c_str();
  if (access == 1u) {   // read only - copy constructor 
    out += cprintf("\n    explicit %s (volatile %s & o) noexcept { R = o.R; };\n", defptr, defptr);
    return;
  }
  if (access == 2u) {   // write only
    out += cprintf(fmt_wo, defptr, resetValue, defptr);
    return;
  }
  if (access == 3u) {   // read / write
    out += cprintf(fmt_wo, defptr, resetValue, defptr);
    out += cprintf(fmt_rw, defptr);
    return;
  }
}


