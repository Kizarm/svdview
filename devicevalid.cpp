#include <QDebug>
#include <stdio.h>
#include "device.h"
#include "utils.h"

/** Některé produktové řady např. Kinetis (Freescale) mají udělaný popis tak divně,
 *  že nemá cenu se tím dál zabývat. Překrytí 32/16/8. bitového přístupu by se asi dalo,
 *  ale muselo by se to přestrukturovat, ale v některých *.svd souborech jsou registry,
 *  jejichž popis mi nějak nedává smysl (pole XXX%sYYY s divnýma délkama).
 * */

/***********************************************************************************************/
static bool LessThanReg (const IOreg & r1, const IOreg & r2) {
  return r1.offset < r2.offset;
}
static bool LessThanField (const field & f1, const field & f2) {
  return f1.offset < f2.offset;
}
static bool LessThanPeriph (const peripheral & p1, const peripheral & p2) {
  return p1.address < p2.address;
}
static bool LessThanIqr (const interrupt & i1, const interrupt & i2) {
  return i1.no < i2.no;
}
/***********************************************************************************************/
static const char * irq_suffix = "_IRQ";

void interrupt::validate () {
  if (name.contains("irq", Qt::CaseInsensitive)) return;
  if (name.contains("fiq", Qt::CaseInsensitive)) return;
  name += irq_suffix;
}
void field::validate(const IOreg * r) {
  if (size > r->size) {
    qDebug() << "field size error" << r->name << name;
    size = r->size;
  }
}
void IOreg::validate() {
  if (!alternatives.isEmpty()) {
    for (int n=0; n<alternatives.size(); n++) alternatives[n].validate();
    return;
  }
  if (fields.isEmpty()) return;
  if (fields.size() == 1) {             // vyhazej pole, pokud je jen jedno s pocatkem v nule
    const field f = fields.at(0);       // zbytecne to prodluzuje hlavicku
    if (f.offset == 0) {
      fields.clear();
      return;
    }
  }
  qSort (fields.begin(), fields.end(), LessThanField);

  field nf;
  nf.access = UNMODIFY;
  QVector<field> nv;
  unsigned o=0, nn=0;
  for (int n=0; n<fields.size(); n++) {
    field f = fields[n];
    if (f.offset > o) {   // vyplnit mezery
      nf.name.sprintf("unused%d", nn++);
      nf.offset = o;
      nf.size   = f.offset - o;
      o += nf.size;
      nv.push_back(nf);
    }
    o += f.size;
  }
  if (o < size) {         // vyplnit do konce
    nf.name.sprintf("unused%d", nn++);
    nf.offset = o;
    nf.size   = size - o;
    nv.push_back(nf);
  }
  fields += nv;
  qSort (fields.begin(), fields.end(), LessThanField);
  for (int n=0; n<fields.size(); n++) fields[n].validate(this);
}
bool IOreg::eq (IOreg& r, group * parent) {
  if (name   != r.name) return false;
  if (offset != r.offset) {
    char c = 'A';
    c += parent->rename;
    parent->rename += 1;
    r.name += "_";
    r.name += c;      // zkusime to trochu spravit, ale je to divne (asi funkcni)
    qDebug() << "register group " << parent->name << " error (rename) : " << name << " => " << r.name;
    return false;
  }
  return true;
}
static void make_unions (QVector<IOreg> & regs) {
  qSort (regs.begin(), regs.end(), LessThanReg);
  unsigned long ofs = ~0ul;
  IOreg         brr;
  for (int n=0; n<regs.size(); n++) {
    IOreg & r = regs[n];
    if (r.offset == ofs) {
      brr.alternatives.push_back(r);///
      regs.remove(n--);
    } else {
      if (n>0) regs[n-1] = brr;
      ofs = r.offset;
      brr = r;
    }
  }
  for (int n=0; n<regs.size(); n++) {     // vlastni registr dame jako 1. alternativu
    IOreg & brr = regs[n];
    if (!brr.alternatives.isEmpty()) {
      IOreg r = brr;
      r.alternatives.clear();
      brr.alternatives.push_front(r);
    }
  }
}

bool is_short_in (const IOreg & t, const IOreg & s) {
  if (t.size < s.size) {
    unsigned long sb = s.offset;
    unsigned long se = s.offset + s.size;
    unsigned long tb = t.offset;
    unsigned long te = t.offset + t.size;
    if ((sb <= tb) && (se >= te)) return true;
  }
  return false;
}
static void remove_short (QVector<IOreg> & regs) {
  qSort (regs.begin(), regs.end(), LessThanReg);
  for (int n=0; n<regs.size(); n++) {
    const IOreg & rt = regs[n];     // testovany
    bool result = false;
    for (int k=0; k<regs.size(); k++) {
      const IOreg & rr = regs[k];   // srovnavaci
      if (is_short_in(rt, rr)) {
        result = true;
        break;
      }
    }
    if (result) {
      // qDebug() << "remove" << rt.name << rt.size;
      regs.remove(n--);
    }
  }
}

void group::validate() {
//qDebug() << "validate group " << name;
  remove_short (registers);   // pro freescale, ale stejne je to naprd.
  make_unions  (registers);
  qSort (registers.begin(), registers.end(), LessThanReg);
  IOreg nr;
  nr.access = UNMODIFY;
  QVector<IOreg> nv;
  nv.clear();
  unsigned long o=0ul, nn=0ul;
  for (int n=0; n<registers.size(); n++) {
    IOreg r = registers[n];
    if (r.offset > o) {
      unsigned long total = r.offset - o; // v bytech
      nr.name.sprintf("UNUSED%ld", nn++);
      nr.offset = o;
      unsigned long bytew = 4ul;
      if        (total & 1ul) { // bytes
        bytew = 1ul;
      } else if (total & 2ul) { // half-words
        bytew = 2ul;
      } else {                  // words
        bytew = 4ul;
      }
      nr.size   = bytew << 3;             // v bitech
      nr.array  = total / bytew;
      if (!nr.array)  nr.array = 1;       // pro jistotu
      o += (nr.size * nr.array) >> 3;
      nv.push_back(nr);///
    } else if (r.offset < o) {
      qDebug() << "address error group" << name << "register" << r.name;
    }
    o += (r.size * r.array) >> 3;
  }
  size = o;
  registers += nv;
  qSort (registers.begin(), registers.end(), LessThanReg);
  // kontrola delky a setrideni
  IOreg last = registers.last();
  unsigned long ctrl = last.offset + ((last.size * last.array) >> 3);
  if (size != ctrl) {
    qDebug("size error at %s %ld != %ld", name.toLatin1().constData(), size, ctrl);
    size = ctrl;
  }
  // TODO : finalni kontrola na stejna jmena (kvuli polim - stejna jmena, ruzne ofsety)
  for (int n=0; n<registers.size(); n++) registers[n].validate();
}
bool group::eq (group & g) {
  if (name != g.name) return false;
  for (int i=0; i<g.registers.size(); i++) {
    IOreg & r = g.registers[i];
    bool pushing = true;
    for (int j=0; j<registers.size(); j++) {
      IOreg & rr = registers[j];
      if (rr.eq (r, this)) {
        pushing = false;
        break;
      }
    }
    if (pushing) registers.push_back(r);///
  }
  return true;
}
void group::append (IOreg & r) {
  for (int n=0; n<registers.size(); n++) {
    IOreg & rr = registers[n];
    if (r.name == rr.name) {
      rr.name += "A";
      r. name += "B";
      // mozna by stalo za to dodelat to stejne jako u registru, ale stejne je to problem
      qDebug() << "append group " << name << " error: " << rr.name << "<=>" << r.name;
    }
  }
  registers.push_back(r);
}
static QString like_name (const QString & s1, const QString & s2) {
  QString result;
  SWRAP w1(s1), w2(s2);
  const char * p1 = w1;
  const char * p2 = w2;
  int n1 = strlen (p1);
  int n2 = strlen (p2);
  int n = n1;
  if (n > n2) n = n2;   // n je minimum
  int i = 0;
  for (i=0; i<n; i++) {
    if (p1[i] != p2[i]) break;
  }
  if (i < (n-1)) return result;
  char * s = strdup(p1);
  s[i] = '\0';
  result = s;
  free (s);
  return result;
}
static peripheral * like_to_this (peripheral & op, QVector<peripheral> & per) {
  group & og = op.registers;
  for (int n=0; n<per.size(); n++) {
    peripheral & pp = per[n];
    group      & gg = pp.registers;
    if (!gg.name.isEmpty()) {
      if (op.name.contains(gg.name)) {            // napr. GPIOA obsahuje skupinu GPIO
        op.groupname = gg.name;
        og.name      = gg.name;
        op.comment   = pp.comment;
        return & pp;
      }
      QString s = like_name(gg.name, op.name);    // nekde se to lisi o posledni pismenko
      if (!s.isEmpty()) {
        // qDebug() << "like " << gg.name << " groupname" << s;
        op.groupname = gg.name;
        og.name      = gg.name;
        op.comment   = pp.comment;
        return & pp;
      }      
    } else {
      /* On si to každý výrobce dělá jinak, kašlu na to.
       * Fakt něco nejde odvodit automaticky, i ručně je to jen odhad.
       * Prostě nutno jít do manuálu.
      */
    }
  }
  return 0;
}
void device::validate (QVector<peripheral> & per) {
  for (int n=0; n<per.size(); n++) {                        // doplnit groupname podle predchozi - STM, LPC
    peripheral & p = per[n];
    if (!p.interrupts.isEmpty()) {
      interrupts += p.interrupts;
    }
    group & g = p.registers;
    g.name = p.groupname;
    if (p.groupname.isEmpty()) {
      peripheral * po = like_to_this (p, per);
      if (!po) {
        g.name      = p.name;                               // tohle bude vadit, musi se rucne opravit
        p.groupname = p.name;                               // preklad size_test() zjisti, kde je problem
      }
    } else {
      if (g.registers.isEmpty()) {
        // qDebug() << "groupname no registers" << p.groupname;
      }
    }
  }
}

void device::validate() {
  validate (peripherals);
  qSort (peripherals.begin(), peripherals.end(), LessThanPeriph);
  for (int n=0; n<peripherals.size(); n++) {                        // vytvorit a validovat periferni skupiny
    peripheral & p = peripherals[n];
    group      & g = p.registers;
    if (p.address < 0xC0000000) g.system = false;
    if (!g.name.isEmpty() && !g.registers.isEmpty()) {
      bool pushing = true;
      for (int i=0; i<regsdef.size(); i++) {
        group & gg = regsdef[i];
        if (gg.eq(g)) {
          pushing = false;
          break;
        }
      }
      if (pushing) regsdef.push_back(g);
    }
  }
  for (int n=0; n<regsdef.size(); n++) regsdef[n].validate();
  qSort (interrupts.begin(), interrupts.end(), LessThanIqr);
  long irq = -1000000; // nesmyslne cislo
  for (int n=0; n<interrupts.size(); n++) {   // vyjmount pripadne duplicity
    interrupt & i = interrupts[n];
    i.validate();
    if (i.no == irq) {
      interrupts.remove(n--);
    } else {
      irq = i.no;
    }
  }
}

