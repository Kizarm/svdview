#include <QDebug>
#include <QRegExp>
#include <QDomDocument>
#include <stdlib.h>
#include "devicetree.h"
#include "utils.h"

DeviceTree::DeviceTree() {

}

DeviceTree::~DeviceTree() {

}
void DeviceTree::parse (const QDomElement & e) {
  device nd;
  dev = nd;       // vynulovat !!!
  parseDevice(e);
  dev.validate();
}
void DeviceTree::parseDevice (const QDomElement& e) {
  for (QDomElement n=e.firstChildElement(); !n.isNull(); n=n.nextSiblingElement()) {
    QDomElement child = n.firstChildElement();
    QString tn = n.tagName();
    if (child.isNull()) {
      property p;
      p.name  = tn;
      p.value = n.text();
      dev.properties.push_back(p);
      if (p.name == "name") {
        dev.name = p.value;
      } else if (p.name == "width") {
        bool ok;
        dev.default_width = p.value.toUInt(&ok, 0);
      }
    } else {
      if (tn == "peripherals") {
        parsePeripherals(n);
      } else if (tn == "cpu") {
        parseCpu (n);
      } else {
        qDebug() << "device = " << tn;
      }
    }
  }
}
void DeviceTree::parseCpu (const QDomElement& e) {
  for (QDomElement n=e.firstChildElement(); !n.isNull(); n=n.nextSiblingElement()) {
    QDomElement child = n.firstChildElement();
    QString tn = n.tagName();
    if (child.isNull()) {
      property p;
      p.name  = tn;
      p.value = n.text();
      dev.cpu.properties.push_back(p);
      //qDebug() << "empty cpu " << tn << n.text();
      if (p.name == "endian" && p.value != "little") {
        qDebug () << "configuration" << p.name << "not supported" << p.value;
        // pro BIG by bylo nutne zmenit poradi pri trideni, zas takova standa by to nebyla
      }
    } else {
      qDebug() << "cpu = " << tn;
    }
  }
}

void DeviceTree::parsePeripherals (const QDomElement& e) {
  for (QDomElement n=e.firstChildElement(); !n.isNull(); n=n.nextSiblingElement()) {
    QDomElement child = n.firstChildElement();
    QString tn = n.tagName();
    if (child.isNull()) {
      qDebug() << "empty " << tn;
    } else {
      if (tn == "peripheral") {
        peripheral p = parsePeripheral(n);
        dev.peripherals.push_back(p);
      } else {
        qDebug() << "unused d" << n.tagName();
      }
    }
  }
}
peripheral DeviceTree::parsePeripheral (const QDomElement & e) {
  peripheral p;
  for (QDomElement n=e.firstChildElement(); !n.isNull(); n=n.nextSiblingElement()) {
    QDomElement child = n.firstChildElement();
    QString tn = n.tagName();
    if (child.isNull()) {
      QString ts = n.text();
      if        (tn == "name") {
        p.name      = ts;
      } else if (tn == "description") {
        p.comment   = strip(ts);
      } else if (tn == "groupName") {
        p.groupname = ts;
      } else if (tn == "baseAddress") {
        bool ok = false;
        unsigned long v = ts.toULong(&ok, 0);
        if (ok) p.address = v;
        else    p.address = 0;
      } else {
        if (tn == "prependToName") {
        } else if (tn == "version") {
        } else
          qDebug() << "unused p" << tn;
      }
    } else {
      if (tn == "registers") {
        parseRegisters (n, p);
      } else if (tn == "addressBlock") {
        // TODO ale pro header asi zbytecne
      } else if (tn == "interrupt") {
        // TODO
        interrupt i = parseInterrupt(n);
        p.interrupts.push_back(i);
      } else {
        qDebug() << "peripheral = " << n.tagName();
      }
    }
  }
  QString deriv = e.attribute("derivedFrom");
  if (!deriv.isEmpty()) {
    peripheral * pd = dev.derived(deriv);
    //qDebug() << p.name << deriv << gr;
    if (pd) {
      if (!pd->groupname.isEmpty()) p.groupname = pd->groupname;
      if (!pd->comment  .isEmpty()) p.comment   = pd->comment;
    }
  }
  return p;
}
peripheral * device::derived (QString & old) {
  for (int n=0; n<peripherals.size(); n++) {
    peripheral & p = peripherals[n];
    if (p.name == old) {
      return & p;
    }
  }
  return 0;
}

interrupt DeviceTree::parseInterrupt (const QDomElement& e) {
  interrupt i;
  for (QDomElement n=e.firstChildElement(); !n.isNull(); n=n.nextSiblingElement()) {
    QDomElement child = n.firstChildElement();
    QString tn = n.tagName();
    if (child.isNull()) {
      QString ts = n.text();
      if        (tn == "name") {
        i.name = ts;
      } else if (tn == "description") {
        i.comment = ts;
      } else if (tn == "value") {
        bool ok = false;
        long v = ts.toLong(&ok, 0);
        if (ok) i.no = v;
      } else {
        qDebug() << "unused intr " << n.tagName();
      }
    } else {
      qDebug() << "interrupt = " << n.tagName();
    }
  }
  return i;
}

void DeviceTree::parseRegisters (const QDomElement & e, peripheral & per) {
  group g;
  for (QDomElement n=e.firstChildElement(); !n.isNull(); n=n.nextSiblingElement()) {
    QDomElement child = n.firstChildElement();
    QString tn = n.tagName();
    if (child.isNull()) {
      qDebug() << "empty " << tn;
    } else {
      if (tn == "register") {
        parseRegister(n, g);
      } else {
        qDebug() << "unused " << tn;
      }
    }
  }
  per.registers = g;
}
struct nxp_repeat {
  unsigned dim;
  unsigned increment;
  unsigned from, to;
  
  nxp_repeat() : dim(0), increment(0), from(0), to(0) {};
  void set (const char * str) {
    int n = sscanf (str, "%d-%d", &from, &to);
    if (n != 2) {
      n = sscanf (str, "%d,%d", &from, &to);    // Freescale
      if (n != 2) qDebug ("dim index error");
    }    
  }
  unsigned check (IOreg & r) {
    if (increment >= 8) {           // Freescale nelogicky !!!
      r.size = increment;
      increment >>= 3;
    }
    unsigned long w = r.size;
    if (w != (increment * 8))   qDebug() << "warn: array element size"     << r.name;
    if ((to - from + 1) != dim) qDebug() << "warn: array size logic error" << r.name;
    return dim;
  }
};
void DeviceTree::parseRegister (const QDomElement & e, group & g) {
  IOreg r;
  nxp_repeat rep;
  for (QDomElement n=e.firstChildElement(); !n.isNull(); n=n.nextSiblingElement()) {
    QDomElement child = n.firstChildElement();
    QString tn = n.tagName();
    bool ok = false;
    if (child.isNull()) {
      QString ts = n.text();
      if        (tn == "name") {
        r.name = ts;
      } else if (tn == "displayName") {
        r.display = ts;
      } else if (tn == "alternateRegister") {
        r.altername = ts;
      } else if (tn == "description") {
        r.comment = strip(ts);
      } else if (tn == "addressOffset") {
        unsigned long v = ts.toULong(&ok, 0);
        if (ok) r.offset = v;
        else    r.offset = 0;
      } else if (tn == "size") {
        unsigned long v = ts.toULong(&ok, 0);
        if (ok) r.size = v;
        else    r.size = 0;
      } else if (tn == "access") {
        r.access = ref_access (ts);
      } else if (tn == "resetValue") {
        unsigned long v = ts.toULong(&ok, 0);
        if (ok) r.reset = v;
        else    r.reset = 0;
      } else if (tn == "dim") {
        rep.dim = ts.toUInt(&ok, 0);
      } else if (tn == "dimIncrement") {
        rep.increment = ts.toUInt(&ok, 0);
      } else if (tn == "dimIndex") {
        rep.set (ts.toLatin1().constData());
      } else if (tn == "resetMask") {
        //  NXP, asi k nicemu
      } else if (tn == "readAction") {
        // NXP, asi k nicemu
      } else if (tn == "alternateGroup") {
        // TODO ATMEL
      } else {
        qDebug() << "unused r : " << n.tagName() << " = " << n.text();
      }
    } else {
      if        (tn == "fields") {
        parseFields (n, r);
      } else {
        qDebug() << "register = " << n.tagName();
      }
    }
  }
  if (!r.size) r.size = dev.default_width;
  if (rep.dim) {
    r.name.remove(QRegExp("%[a-z]{1}"));
    r.name.remove("[]");
    r.array = rep.check(r);
    /*  NXP přidává jména %s jako číslice, pokud to není v závorce [%s],
     * zde se k tomu budeme chovat tak, jako by to v závorce bylo - tedy jako pole
    SWRAP s(r.name);
    unsigned long ofs = 0;
    for (unsigned n=rep.from; n<=rep.to; n++) {
      IOreg nr = r;
      const char * fmt = s;
      char buf [16], name[64];
      snprintf(buf,  16, "%d", n);
      snprintf(name, 64, fmt, buf);
      nr.name   = name;
      nr.offset = r.offset + ofs;
      ofs += rep.increment;
      g.registers.push_back(nr);
    }
    */
  }
  g.append(r);    // append je push_back s kontrolou názvu
}
void DeviceTree::parseFields (const QDomElement& e, IOreg& reg) {
  for (QDomElement n=e.firstChildElement(); !n.isNull(); n=n.nextSiblingElement()) {
    QDomElement child = n.firstChildElement();
    QString tn = n.tagName();
    if (child.isNull()) {
      qDebug() << "empty " << tn;
    } else {
      if (tn == "field") {
        field f = parseField(n, reg);
        if (f.name == "RESERVED") {
          // NXP repeats this field in register
        } else {
          reg.fields.push_back(f);
        }
      } else {
        qDebug() << "unused " << tn;
      }
    }
  }
}
field DeviceTree::parseField (const QDomElement& e, IOreg& reg) {
  field f;
  f.access = reg.access;
  for (QDomElement n=e.firstChildElement(); !n.isNull(); n=n.nextSiblingElement()) {
    QDomElement child = n.firstChildElement();
    QString tn = n.tagName();
    if (child.isNull()) {
      QString ts = n.text();
      if        (tn == "name") {
        f.name = ts;
      } else if (tn == "description") {
        f.comment = strip(ts);
      } else if (tn == "bitOffset") {
        bool ok = false;
        unsigned long v = ts.toULong(&ok, 0);
        if (ok) f.offset = v;
        else    f.offset = 0;
      } else if (tn == "bitWidth") {
        bool ok = false;
        unsigned long v = ts.toULong(&ok, 0);
        if (ok) f.size = v;
        if (!f.size) {
          qDebug() << "field size error " << f.name;
          f.size = 1;
        }
      } else if (tn == "access") {
        f.access = ref_access (ts);
      } else if (tn == "bitRange") {    // NXP a j.
        set_bits (ts.toLatin1().constData(), f.offset, f.size);
      } else if (tn == "lsb") {         // nekde je to takto
        bool ok = false;
        unsigned long v = ts.toULong(&ok, 0);
        if (ok) f.offset = v;
      } else if (tn == "msb") {
        bool ok = false;
        unsigned long v = ts.toULong(&ok, 0);
        if (ok) f.size = v - f.offset + 1;
      } else {
        qDebug() << "unused f : " << n.tagName() << " = " << n.text();
      }
    } else {
      if (tn == "enumeratedValues") {
        // TODO asi uzitecne, ale s enum jsou potize
        if (f.size > 1) { // jinak je to nanic, je jen ano/ne, ale i tak jsou to vetsinou blbosti.
          //qDebug() << "enums" << tn << "for" << reg.name ;
          parseEnums (n, f);
        }
      } else {
        qDebug() << "field = " << n.tagName();
      }
    }
  }
  return f;
}
void DeviceTree::parseEnums (const QDomElement& e, field & fld) {
  f_enums fe;
  for (QDomElement n=e.firstChildElement(); !n.isNull(); n=n.nextSiblingElement()) {
    QDomElement child = n.firstChildElement();
    QString tn = n.tagName();
    if (child.isNull()) {
      QString ts = n.text();
      if        (tn == "name") {
        fe.name = ts;
      } else {
        qDebug() << "enum = " << tn << ts;
      }
    } else {
      if (tn == "enumeratedValue") {
        parseEnum(n,fe);
      } else
        qDebug() << "enums = " << n.tagName();
    }
  }
  fld.values = fe;
}
void DeviceTree::parseEnum (const QDomElement& e, f_enums & enm) {
  f_enum fe;
  for (QDomElement n=e.firstChildElement(); !n.isNull(); n=n.nextSiblingElement()) {
    QDomElement child = n.firstChildElement();
    QString tn = n.tagName();
    if (child.isNull()) {
      QString ts = n.text();
      if        (tn == "name") {
        fe.name    = ts;
      } else if (tn == "value") {
        fe.value   = ts;
      } else if (tn == "description") {
        fe.comment = ts;
      } else {
        qDebug() << "enum = " << tn << ts;
      }
    } else {
      qDebug() << "enum = " << n.tagName();
    }
  }
  enm.enums.push_back(fe);
}

void DeviceTree::dump() {
  QString out = "\n";
  dev.dump(out);
  qDebug() << out;
}
void DeviceTree::write (QIODevice * d) {
  QTextStream stream (d);
  QString name = dev.name + "_GENERATED_HEADER";
  QString out;
  out += "#ifndef " + name + "\n";
  out += "#define " + name + "\n";
  out += "#include <stdint.h>\n\n";
  out += "#ifdef __cplusplus\nextern \"C\" {\n#endif // __cplusplus\n\n";
  out += "#define __RO volatile const\n";
  out += "#define __WO volatile\n";
  out += "#define __RW volatile\n";
  out += "#define __ROB const\n";
  out += "#define __WOB\n";
  out += "#define __RWB\n\n";
  dev.dump(out);
  out += "\n#ifdef __cplusplus\n};\n#endif // __cplusplus\n";
  out += "#endif // " + name + "\n";
  test (out);
  stream << out;
}
void DeviceTree::test (QString & out) {
  QString tst;
  tst += "\n#ifdef SIZE_TEST\n";
  tst += "static int size_test (void) {\n";
  tst += "  int res = 0;\n";
  for (int n=0; n<dev.regsdef.size(); n++) {
    group g = dev.regsdef[n];
    if (g.system) break;
    QString num;
    num.sprintf("%s) != %ld) res += 1;\n", typedef_suffix, g.size);
    tst += "  if (sizeof(" + g.name + num;
  }
  tst += "  return res;\n";
  tst += "}\n#endif // SIZE_TEST\n";
  tst += "\n";
  out += tst;
}

