#include <iostream>
#include <algorithm>
#include "cmsistree.h"
#include "devicetree.h"

using namespace std;

static void evalStrings (string & name, string & baseName, const dimElementGroup * group) {
  size_t n = name.find_first_of ("%[");
  baseName = name.substr (0, n);
  if (name.find_first_of ("[]") != string::npos) {  // Atmel - v závorkách to nemá být, vytvoříme jen pole
    name = ""; return;
  }
  if (group->dimIndex.base.empty()) {
    name = ""; return;              // chyba logiky cmsis
  }
  // printf("dimIndex=%s, name=%s, dim = %ld, increment = %ld\n", group->dimIndex.base.c_str(), name.c_str(), group->dim.base, group->dimIncrement.base);
  vector<string> vs = split_string (group->dimIndex.base, '-');
  if (vs.size() == 2u) {
    const string bs = vs[0], es = vs[1];
    long begin = 0l, end = 0l;
    begin = cmsis_toUlong(bs);
    end   = cmsis_toUlong(es);
    const long cl = end - begin + 1l;
    if (cl != (long) group->dim.base) { // tato kontrola bude postačující
      name = ""; return;                // pokud selže, jména nerozvíjet, ničemu to nevadí
    }
    string res;
    const char * fmt = name.c_str();
    for (long n=begin; n<=end; n++) {
      string os = cprintf("%ld,", n);
      res += cprintf(fmt, os.c_str());
    }
    res  = res.substr (0, res.size() - 1u);
    name = res;
    //printf("res=%s\n", name.c_str());
    return;
  }
  vs = split_string (group->dimIndex.base, ',');
  if (vs.size() != group->dim.base) {
    name = ""; return;
  }
  const char * fmt = name.c_str();
  string res;
  for (auto & str: vs) {
    res += cprintf(fmt, str.c_str()) + ',';
  }
  res  = res.substr (0, res.size() - 1u);
  name = res;
  // printf("res=%s\n", name.c_str());
}
static void reg_rename (RegisterPart & r, char & c) {
  string s (r.baseName);
  r.baseName += c++;
  CERR << "reg_rename " << s << " to " << r.baseName << '\n';
}
static void field_rename (FieldPart & r, char & c) {
  string s (r.name);
  r.name += c++;
//CERR << "field_rename " << s << " to " << r.name << '\n';
}
static void enum_rename (EnumValuesPart & r, char & c) {
  string s (r.name);
  r.name += c++;
//CERR << "field_rename " << s << " to " << r.name << '\n';
}
//////////////////////////////////////////////
DeviceTree::DeviceTree (const CmsisTree & p) noexcept : MandatoryPart(this),
             cmsis(p), peripherals(), headerName() {
};
static TYPES_WITH type_from_long (const unsigned long size) {
  TYPES_WITH res = TYPE_32BIT;
  if (size == 1ul) {
    res = TYPE_8BIT;
  } else if (size == 2ul) {
    res = TYPE_16BIT;
  } else if (size == 4ul) {
    res = TYPE_32BIT;
  } else {
    res = TYPE_32BIT;
    CERR << "Invalid size " << size << "\n";
  }
  return res;
}

void DeviceTree::convert() {
  name    = cmsis.device.name.base;
  comment = strip_wc (cmsis.device.description.base);
  address = 0ul;    // zde není důležité
  size    = 0ul;
//width   = type_from_long (cmsis.device.width.base >> 3);   // bus size
  width   = type_from_long (cmsis.device.size.base  >> 3);   // default register size
  if (cmsis.device.headerSystemFilename.base.size()) {
    headerName = cmsis.device.headerSystemFilename.base;
  } else headerName = cmsis.device.name.base + ".h";
  
  auto & vp = cmsis.device.peripherals.peripheral;
  for (auto & p: vp) {
    PeripheralPart np(root);
    np.convert (p);
    peripherals.push_back (np);
  }
  vector<InterruptPart> copy;
  for (auto & p: peripherals) {
    for (auto & i: p.interrupts) { 
      copy.push_back (i);
    }
  }
  addSystemInterrupts (copy);
  sort (copy.begin(), copy.end(), [] (InterruptPart & a, InterruptPart & b) {
    return a.value < b.value;
  });
  long old = 0;
  for (auto & i: copy) {
    if (i.value == old) continue;
    old = i.value;
    interrupts.push_back (i);
  }
}
void DeviceTree::validate() {
  for (auto & p: peripherals) p.validate();
  compareDevice();
  finalIndexing();
}
void PeripheralPart::convert(const peripheralType * p) {
  name    = /* p->prependToName.base + */ p->name.base + p->appendToName.base; // je otázka jak je myšleno
  comment = strip_wc (p->description.base);
  address = p->baseAddress.base;
  size    = 0lu;
  
  for (auto & b: p->addressBlock) {
    if (b.offset.base > size) size = b.offset.base;
    size += b.size.base;
  }
  groupName = p->groupName.base;
  if (p->derivedFrom) {
    PeripheralPart derived (root);
    derived.convert(p->derivedFrom);
    if (groupName.empty()) groupName = derived.groupName;
    if (comment  .empty()) comment   = derived.comment;
    if (!width)            width     = derived.width;
    baseName = p->derivedFrom->name.base;
    if (p->registers._register.size()) {
      CERR << "Derived peripheral " << name << " has a overrive, not supported.";
      // TODO : override
    }
  }
  if (groupName.empty()) groupName = name;
  
  auto & vr = p->registers._register;
  for (auto & r: vr) {
    RegisterPart nr(root);
    nr.convert (r);
    registers.push_back (nr);
  }  
  for (auto & e: p->interrupt) {
    InterruptPart ip;
    ip.name    = e.name.base;
    ip.comment = strip_wc(e.description.base);
    ip.value   = e.value;
    interrupts.push_back (ip);
  }
}
void PeripheralPart::validate() {
  for (auto & r: registers) r.validate ();
  checkNames ();
  fillGaps   ();
  struct_len = makeUnion ();
}

void PeripheralPart::checkNames() {
  // Heuristika, obcházející chybu v popisu NXP.
  sort (registers.begin(), registers.end(), [] (RegisterPart & a, RegisterPart & b) {
    return a.baseName < b.baseName;
  });
  vector<RegisterPart> copy;
  string obn;
  char index = 'A';
  for (auto & r: registers) {
    if (obn == r.baseName) {
      auto & last = copy.back();
      if (last.baseName == obn) reg_rename (last, index);
      reg_rename(r, index);
    } else {
      obn   = r.baseName;
      index = 'A';
    }
    copy.push_back (r);
  }
  registers.clear();
  for (auto & r: copy) registers.push_back (r);
}
void RegisterPart::checkNames() {
  // Heuristika, obcházející chybu v popisu NXP.
  sort (fields.begin(), fields.end(), [] (FieldPart & a, FieldPart & b) {
    return a.name < b.name;
  });
  vector<FieldPart> copy;
  string obn;
  char index = 'A';
  for (auto & r: fields) {
    if (obn == r.name) {
      auto & last = copy.back();
      if (last.name == obn) field_rename (last, index);
      field_rename(r, index);
    } else {
      obn   = r.name;
      index = 'A';
    }
    copy.push_back (r);
  }
  fields.clear();
  for (auto & r: copy) fields.push_back (r);
}
void FieldPart::checkNames() {
  if (eenum.values.empty()) return;
  // Heuristika, obcházející chybu v popisu NXP.
  sort (eenum.values.begin(), eenum.values.end(), [] (EnumValuesPart & a, EnumValuesPart & b) {
    return a.name < b.name;
  });
  vector<EnumValuesPart> copy;
  string obn;
  char index = 'A';
  for (auto & r: eenum.values) {
    if (obn == r.name) {
      auto & last = copy.back();
      if (last.name == obn) enum_rename (last, index);
      enum_rename(r, index);
    } else {
      obn   = r.name;
      index = 'A';
    }
    copy.push_back (r);
  }
  sort (copy.begin(), copy.end(), [] (EnumValuesPart & a, EnumValuesPart & b) {
    return a.value < b.value;
  });
  eenum.values.clear();
  for (auto & r: copy) eenum.values.push_back (r);
}

void PeripheralPart::fillGaps() {
  // sekundární třídění podle délky je nutné, jinak se může vyplnit blbě
  sort (registers.begin(), registers.end(), [] (RegisterPart & a, RegisterPart & b) {
    return a.size < b.size;
  });
  sort (registers.begin(), registers.end(), [] (RegisterPart & a, RegisterPart & b) {
    return a.address < b.address;
  });
  vector<RegisterPart> copy;
  unsigned long ofset = 0ul, gapn = 0lu;
  for (auto & r: registers) {
    if (ofset < r.address) {    // mezera, vyplnit
      const unsigned long gap = r.address - ofset;
      // printf("gap  (%ld) %s\n", gap, r.name.c_str());
      RegisterPart nr(root);
      nr.width    = TYPE_8BIT;
      nr.size     = gap;
      nr.address  = ofset;
      nr.access   = 0u;
      if ((gap & 3ul) == 0) {   // lépe je vyplnit největším typem, pokud to jde
        nr.width  = TYPE_32BIT;
        nr.size >>= 2;
      }
      nr.baseName = cprintf("UNUSED%ld", gapn++);
      copy.push_back (nr);
    }
    copy.push_back (r);
    ofset = r.address + r.width * r.size;
  }
  registers.clear();
  for (auto & r: copy) registers.push_back (r);
}
unsigned long PeripheralPart::makeUnion() {
  vector<RegisterPart> copy;
  unsigned long ofset = 0ul;
  for (auto & r: registers) {
    if (ofset > r.address) {   // same
      auto & last = copy.back();
      last.reg_union.push_back (r);
    } else copy.push_back (r);
    ofset = r.address + r.width * r.size;
  }
  registers.clear();
  for (auto & r: copy) registers.push_back (r);
  return ofset;
}
void RegisterPart::convert(const registerType * r) {
  TYPES_WITH defw = root ? root->width : TYPE_32BIT;
  address  = r->addressOffset.base;                      // offset in bytes
  comment  = strip_wc (r->description.base);
  access   = r->access.order;
  if (access == 0u) access = 3u;
  name     = r->name.base;  // jméno zde může být prázdné, pokud je to pole,
                            // ale v tt. případě může obsahovat list alternativních jmen
  baseName = name;          // pak je jméno zde, oříznuté o případné [%s]
  size     = 1ul;
  resetMask  = r->resetMask.base;
  resetValue = r->resetValue.base;
  const unsigned long rw = r->size.base >> 3;
  width = rw ? type_from_long (rw) : defw;
  // if (width != TYPE_32BIT) printf("%s: width=%d\n", name.c_str(), (int) width);
  if (r->dim.base) {                                      // pokud je specifikovano
    const unsigned long increment = r->dimIncrement.base; // pak musi byt i toto
    if (increment != width) {
      // Tady je problém např. Freescale - netuším, jak je to myšleno, ale nezapadá to
      // do koncepce C-čkové hlavičky. Je to nějak divně překrýváno.
      CERR << "register array " << baseName << " increment logic error (" << increment << " != " << width << ")\n";
    }
    size  = r->dim.base;
    evalStrings (name, baseName, r);
  }
  // printf("register: (%d) %s\n", access, name.c_str());
  // vector fields
  const fieldsType & vf = r->fields;
  for (auto e: vf.field) {
    FieldPart fp (root);
    fp.convert (*this, e);
    fields.push_back (fp);
  }
}
void RegisterPart::validate() {
  for (auto & f: fields) f.validate();
  checkNames();
  fillGaps  ();
}

void RegisterPart::fillGaps() {
  // setřídit podle ofsetu
  sort (fields.begin(), fields.end(), [] (FieldPart & a, FieldPart & b) {
    return a.address < b.address;    
  });
  vector<FieldPart> copy;
  unsigned long ofset = 0ul, gapn = 0lu;
  for (auto & f: fields) {
    if (ofset < f.address) {          // mezera, vyplnit
      const unsigned long gap = f.address - ofset;
      // printf("gap  (%ld) %s\n", gap, f.name.c_str());
      FieldPart nf (root);
      nf.width   = f.width;
      nf.size    = gap;
      nf.address = ofset;
      nf.access  = 0;
      nf.name    = cprintf("UNUSED%ld", gapn++);
      copy.push_back (nf);
      copy.push_back (f);
      ofset = f.address + f.size;
    } else if (ofset > f.address) {   // překryv, ignorovat, nic jiného s tím neudělám
                                      // union by asi byl zbytečný
      //printf("%s: ofs=%ld, adr=%ld\n", f.name.c_str(), ofset, f.address);
      CERR << "Register: " << name << " ignore field " << f.name << ", override\n";
    } else {                          // ok, navazuje
      copy.push_back (f);
      ofset = f.address + f.size;
    }
  }
  fields.clear();
  for (auto & e: copy) fields.push_back (e);
}

void FieldPart::convert(RegisterPart & p, fieldType & f) {
  name    = f.name.base;
  comment = strip_wc (f.description.base);
  width   = p.width;  // zděděno z registru kvůli bázovému typu uintXX_t
  address = f.range.from;
  size    = f.range.to - f.range.from + 1ul;
  access  = f.access.order;
  if (access == 0u) access = p.access;
  // printf("  field: [%ld:%ld](%d) %s [%s]\n", address, size, access, name.c_str(), f.access.base.c_str());
  auto & evs = f.enumeratedValues;
  if (!evs.enumeratedValue.empty()) {
    string es = evs.name.base;   // většinou je to jen "ENUM" nebo nic
    if (es.empty()) es = "ENUM";
    eenum.name = cprintf("%s_%s", name.c_str(), es.c_str());
    // printf("    %s : has enumeratedValues %s\n", name.c_str(), eenum.name.c_str());
    for (auto & e: evs.enumeratedValue) {
      // printf("    e:%s = %s /*%s*/\n", e.name.base.c_str(), e.value.base.c_str(), strip_wc(e.description.base).c_str());
      EnumValuesPart evp;
      evp.name    = name + '_';   // NXP hack, prodluzuje nazvy, ale nevadi
      evp.name   += e.name.base;
      evp.comment = strip_wc (e.description.base);
      evp.value   = e.value.base;
      eenum.add (evp);
    }
  }
}
void FieldPart::validate() {
  checkNames();
}
static const InterruptPartC sys_irqs [] = {
  {"Reset",           "1 Reset",                                -15},
  {"NonMaskableInt",  "2 Non Maskable Interrupt",               -14},
  {"HardFault",       "3 HardFault",                            -13},
  {"MemoryManagement","4 Cortex-M4 Memory Management Interrupt",-12},
  {"BusFault",        "5 Cortex-M4 Bus Fault Interrupt",        -11},
  {"UsageFault",      "6 Cortex-M4 Usage Fault Interrupt",      -10},
  {"SVCall",          "11 Cortex-Mx SV Call Interrupt",          -5},
  {"DebugMonitor",    "12 Cortex-M4 Debug Monitor Interrupt",    -4},
  {"PendSV",          "14 Cortex-Mx Pend SV Interrupt",          -2},
  {"SysTick",         "15 Cortex-Mx System Tick Interrupt",      -1},
};
void DeviceTree::addSystemInterrupts(vector<InterruptPart> & copy) {
  // return
  for (auto & e: sys_irqs) {
    InterruptPart p (e);
    copy.push_back  (p);
  }
}
void DeviceTree::finalIndexing() {
  int p_id = 0;
  for (auto & p: peripherals) {
    int r_id = 0;
    for (auto & r: p.registers) {
      int f_id = 0;
      for (auto & f: r.fields) {
        f.part_id = f_id;
        f_id += 1;
      }
      r.part_id = r_id;
      r_id += 1;
    }
    p.part_id = p_id;
    p_id += 1;
  }
}

