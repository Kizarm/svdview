#include <QDebug>
#include <stdlib.h>
#include <stdio.h>
#include "device.h"
#include "utils.h"

extern bool NO_UNIONS;
extern bool CLASICDEF;

static const char * val_str = "R";
static const char * bit_str = "B";
static const char * handler_prefix = "Handler";

// pokud je to mozne, tento atribut neni dobre pouzivat, potreba je napr. pro Fujitse a Spansion
// 32. bit procesory by mely mit registry zarovnany, takze to indikuje nejaky problem.
// static const char * pack = "__attribute__((packed)) ";
   static const char * pack = " ";

void f_enum::dump (QString & out, const f_enums * , const unsigned id) const {
  QString scm(comment);
  out += indent(id)   + strip(scm) + "\n";
  out += indent(id+2) + name + " = " + value + "\n";
}

void f_enums::dump (QString & out, const field * parent, const unsigned id) const {
  QString strf;
  strf += indent(id) + "/** Enumerated values " + name + " for " + parent->name + "\n";
  for (int n=0; n<enums.size(); n++) enums[n].dump (strf, this, id + 2);
  strf += indent(id) + "*/\n";
  
  out += strf;
}

void interrupt::dump (QString & out, const unsigned id) const {
  QString fs, sirq = name + "n", cmn(comment);
  SWRAP wn(l_just(sirq, 24)), wc(strip(cmn));
  fs.sprintf("%s = %ld, /*!< %s */\n", (char const *) wn, no, (char const *) wc);
  out += indent(id) + fs;
}
void interrupt::defs (QString & out, const unsigned id) const {
  QString fs, sirq = "void " + name + handler_prefix;
  SWRAP wn(l_just(sirq, 24));
  fs.sprintf("%s (void) ALIAS(Default_Handler);\n", (char const *) wn);
  out += indent(id) + fs;
}
void interrupt::func (QString& out, const unsigned int id) const {
  QString sirq = name + handler_prefix + ",\n";
  out += indent(id) + sirq;
}

void field::dump (QString & out, const IOreg * parent, const unsigned int id) const {
  if (!values.name.isEmpty()) values.dump (out, this, id);
  QString str, styp, cname = name;
  styp.sprintf("%s %s ", bitsmodifics[access], ref_width(parent->size));
  if (access != UNMODIFY) {
    str.sprintf (" : %2ld; /*!< Ofs=%ld, w=%ld ", size, offset, size);
    str += comment;
    out += indent(id) + styp + l_just(cname, name_width) + str + " */\n";
  } else {
    str.sprintf (" : %2ld;", size);
    out += indent(id) + styp + l_just(cname, name_width) + str + "\n";
  }
}
void field::defs (QString & out, const IOreg * parent, const QString & prefix) const {
  if (access == UNMODIFY) return;
  QString sf,sn = prefix + "_" + parent->name + "_" + name;
  unsigned long mask = ((1ul << size) - 1ul) << offset;
  if (size > 1) {
    QString sm = sn + "_MASK";
    QString sb = sn + "_BITS";
    SWRAP wp(parent->name), wn(sm), wb(sb), wx(sn);
    if (offset) {
      sf.sprintf("/*! w=%ld use: %s = ((X << %s) & %s) */\n", size, (char const *) wp, (char const *) wb, (char const *) wn);
      out += sf;
      if (CLASICDEF) {
        sf.sprintf("#define %s ((%s)%ldu) /*!< LeftShift=%ld */\n",
                  (char const *) wb, ref_width(parent->size), offset, offset);
      } else {
        sf.sprintf("static const %s %s = %ldu; /*!< LeftShift=%ld */\n",
                  ref_width(parent->size), (char const *) wb, offset, offset);
      }
      out += sf;
      sn   = sm;
    }
    else {
      sf.sprintf("/*! w=%ld use: %s = (X & %s) */\n", size, (char const *) wp, (char const *) wx);
      out += sf;
    }
  }
  SWRAP nm(l_just(sn, 32)), cm(comment);
  if (CLASICDEF) {
    sf.sprintf("#define %s ((%s)0x%08lXu) /*!< Ofs=%ld %s */\n",
              (char const *) nm, ref_width(parent->size), mask, offset, (char const *) cm);
  } else {
    sf.sprintf("static const %s %s = 0x%08lXu; /*!< Ofs=%ld %s */\n",
              ref_width(parent->size), (char const *) nm, mask, offset, (char const *) cm);
  }
  out += sf;
}
static unsigned min_width_union (const QVector<IOreg> & regs) {
  unsigned size = ~1u;
  for (int n=0; n<regs.size(); n++) {
    const IOreg & r = regs[n];
    unsigned w = r.size * r.array;
    if (w < size) size = w;
  }
  return size;
}

void IOreg::dump (QString& out, const group * parent, const unsigned un, const unsigned int id) const {
  if (!alternatives.isEmpty()) {
    unsigned mu = min_width_union(alternatives);
    out += indent(id) + "union {\n";    // aby to slo vypsat najednou
    for (int n=0; n<alternatives.size(); n++) alternatives[n].dump(out, parent, mu, id+2);
    out += indent(id) + "}" + pack + ";\n";
    return;
  }
  SWRAP dm;
  if (NO_UNIONS || (fields.isEmpty())) {
    QString sdef = ref_width(size);
    dm  = sdef;
  } else {
    QString sdef = "struct " + parent->name + "_" + name + struct_suffix;
    dm  = l_just(sdef, 24);
  }
  QString strreg, rn;
  if (array > 1) {
    long unsigned asize = array;
    if (un) {
      asize = un / size;
    }
    rn.sprintf("[%ld]", asize);
  }
  rn = name + rn + ";";
  if (access != UNMODIFY) {
    rn = l_just(rn, 16);
  }
  SWRAP nm(rn), cm(comment);
  if (access != UNMODIFY) {
    strreg.sprintf("%s %s %s /*!< Ofs=%3ld, W=%ld %s */\n", modificators[access], (char const *) dm,
                  (char const *) nm, offset, (size * array) >> 3, (char const *) cm);
  } else {
    strreg.sprintf("%s %s %s\n", modificators[access], ref_width(size), (char const *) nm);
  }
  out += indent(id) + strreg;
}
void IOreg::defs (QString& out, const group * parent, const unsigned int id) const {
  if (access == UNMODIFY) return;
  if (fields.isEmpty())   return;
  
  if (!alternatives.isEmpty()) {
    for (int n=0; n<alternatives.size(); n++) alternatives[n].defs(out, parent, id);
    return;
  }
  QString strreg;
  if (NO_UNIONS) {
    strreg += "\n";
    for (int n=0; n<fields.size(); n++) fields[n].defs(out, this, parent->name);
  } else {
    QString rn, sdef = parent->name + "_" + name + struct_suffix;
    rn.sprintf("%s %s %s;\n", bitsmodifics[access], ref_width(size), val_str);
    strreg += indent(id)   + "struct " + sdef + " { /*!< " + comment + " */\n";
    strreg += indent(id+2) + "union {\n";
    strreg += indent(id+6) + rn;
    strreg += indent(id+4) + "struct {\n";
    out += strreg;
    for (int n=0; n<fields.size(); n++) fields[n].dump(out, this, id+6);
    strreg  = indent(id+4) + "} "/* + pack*/ + bit_str + ";\n";
    strreg += indent(id+2) + "} "/* + pack*/ + ";\n";
    meth (strreg, parent, id + 2);
    strreg += indent(id)   + "}" + pack + ";\n";
  }
  out += strreg;
}
/* metody napr.:
  USART_ISR_s(__RO USART_ISR_s & o): R(o.R) {};
  RCC_CFGR_s(): VAL(0) {};
  void modify (uint32_t (*f) (RCC_CFGR_s & r)) volatile {
    RCC_CFGR_s t; t.VAL = VAL;
    VAL = f (t);
  }
  void setbits (uint32_t (*f) (RCC_CFGR_s & r)) volatile {
    RCC_CFGR_s t;
    VAL = f (t);
  }
 * konstruktor je potreba, volatile taky, funguje stejne */
void IOreg::meth (QString & out, const group * parent, const unsigned int id) const {
  if (access == UNMODIFY ) return;
  QString mn,strreg, sdef = parent->name + "_" + name + struct_suffix;
  SWRAP wdef(sdef);
  strreg += "#ifdef __cplusplus\n";
  if (access == READ_ONLY) {    // kopirovaci konstruktor
    mn.sprintf("%s(%s %s & o) : %s(o.%s) {};\n", (char const *) wdef, modificators[access], (char const *) wdef, val_str, val_str);
    strreg += indent(id) + mn;
  } else {                      // konstruktor - nastavi hodnotu na tu po resetu
    // if (reset) qDebug ("reset=%ld", reset);
    mn.sprintf("%s(): %s(0x%08lXu) {};\n", (char const *) wdef, val_str, reset);
    strreg += indent(id) + mn;
    if (access == READ_WRITE) {
      mn.sprintf("void modify  (%s (*f) (%s & r)) volatile {\n", ref_width(size), (char const *) wdef);
      strreg += indent(id) + mn;
      mn.sprintf("%s t; t.%s = %s;\n", (char const *) wdef, val_str, val_str);
      strreg += indent(id+2) + mn;
      mn.sprintf("%s = f (t);\n", val_str);
      strreg += indent(id+2) + mn;
      strreg += indent(id) + "}\n";
    }
    if (access & WRITE_ONLY) {
      mn.sprintf("void setbits (%s (*f) (%s & r)) volatile {\n", ref_width(size), (char const *) wdef);
      strreg += indent(id) + mn;
      mn.sprintf("%s t;\n", (char const *) wdef);
      strreg += indent(id+2) + mn;
      mn.sprintf("%s = f (t);\n", val_str);
      strreg += indent(id+2) + mn;
      strreg += indent(id) + "}\n";
    }
  }
  strreg += "#endif // __cplusplus\n";
  
  out += strreg;
}

void group::dump (QString& out, const unsigned int id) const {
  if (system) return;
  int n;
  QString end;
  end.sprintf("; /* W=%ld=0x%lX */\n", size, size);
  out += indent(id) + "typedef struct " + name + struct_suffix + " { /*!< peripheral group " + name + " */\n";
  for (n=0; n<registers.size(); n++) registers[n].dump(out, this, 0u, id+2);
  out += indent(id) + "} " + pack + name + typedef_suffix + end;
  
}
void group::defs (QString& out, const unsigned int id) const {
  if (system) return;
  for (int n=0; n<registers.size(); n++) registers[n].defs(out, this, id);
}

void peripheral::dump (QString& out, const bool plus, const unsigned int id) const {
  if (registers.system) return;
  const unsigned w = 22;
  QString strper, tname = registers.name + typedef_suffix, cname = name;
  SWRAP tn (l_just(tname, w)), pn(l_just(cname, w)), cm(comment);
  if (plus) {
    strper.sprintf("static %s & %s = * reinterpret_cast<%s * const> (0x%08lXu); /*!< %s */\n",       // C++ odkaz
                  (char const *) tn, (char const *) pn, (char const *) tn, address, (char const *) cm);
  } else {
    if (CLASICDEF) {
      strper.sprintf("#define %s ((%s * const) 0x%08lXu) /*!< %s */\n",                     // C   ukazatel
                    (char const *) pn, (char const *) tn, address, (char const *) cm);
    } else {
      strper.sprintf("static %s * const %s = (%s * const) 0x%08lXu; /*!< %s */\n",                     // C   ukazatel
                    (char const *) tn, (char const *) pn, (char const *) tn, address, (char const *) cm);
    }
  }
  out += indent(id) + strper;
}
void property::dump (QString& out, const unsigned int id) const {
  out += indent(id) + name + " = \"" + value + "\"\n";
}
void processor::dump (QString& out, const unsigned int id) const {
  int n;
  out += indent(id) + "cpu:\n";
  for (n=0; n<properties.size(); n++)  properties[n].dump(out, id+2);
}

void device::defs (QString& out, const unsigned int id) const {
  out += "#if 0  // koliduje s CMSIS a chybi potrebna systemova preruseni\n";
  out += "//! Pouzijte COPY / PASTE do prislusnych souboru\n";
  out += "typedef enum IRQn {\n";
  for (int n=0; n<interrupts.size();  n++)  interrupts[n].dump(out, id+2);
  out += "} IRQn_Type;\n//! Vector table declarations (device part)\n";
  for (int n=0; n<interrupts.size();  n++)  interrupts[n].defs(out, id+2);
  out += "//! Vector table definitions (device part)\n";
  const int m = interrupts.last().no + 1;
  for (int n=0; n<m;  n++) {
    int k = -1;
    for (int i=0; i<interrupts.size(); i++) {
      const interrupt & it = interrupts[i];
      if (it.no == n) {
        k = i;
        break;
      }
    }
    if (k >= 0) {
      interrupts[k].func(out, id+2);
    } else {
      out += indent(id+2) + "0,\n";
    }
  }
  out += "\n";
  out += "#endif // 0\n";
}
void device::dump (QString& out, const unsigned int id) const {
  int n;
  out += "/*\n";
  for (n=0; n<properties. size(); n++) properties [n].dump(out, id);
  cpu.dump(out, id);
  out += "*/\n";
  out += "\n";
  for (n=0; n<regsdef    .size(); n++) regsdef    [n].defs(out, id);
  out += "\n";
  for (n=0; n<regsdef    .size(); n++) regsdef    [n].dump(out, id);
  out += "\n";
  out += "#ifdef __cplusplus\n";  
  for (n=0; n<peripherals.size(); n++) peripherals[n].dump(out, true,  id);
  out += "#else // __cplusplus\n";  
  for (n=0; n<peripherals.size(); n++) peripherals[n].dump(out, false, id);
  out += "#endif // __cplusplus\n";
  out += "\n";
  defs (out, id); // interrupts
}
