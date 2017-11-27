#ifndef DEVICETREE_H
#define DEVICETREE_H
#include <QVector>
#include <QString>
#include "device.h"

class QDomElement;
class QIODevice;

class DeviceTree {
  public:
    DeviceTree();
    ~DeviceTree();
    void parse (const QDomElement & e);
    void write (QIODevice * d);
    void dump  ();
    void test  (QString & out);
  protected:
    void        parseDevice      (const QDomElement & e);
    void        parseCpu         (const QDomElement & e);
    void        parsePeripherals (const QDomElement & e);
    peripheral  parsePeripheral  (const QDomElement & e);
    void        parseRegisters   (const QDomElement & e, peripheral & per);
    void        parseRegister    (const QDomElement & e, group & grp);
    interrupt   parseInterrupt   (const QDomElement & e);
    void        parseFields      (const QDomElement & e, IOreg & reg);
    field       parseField       (const QDomElement & e, IOreg & reg);
    void        parseEnums       (const QDomElement & e, field & fld);
    void        parseEnum        (const QDomElement & e, f_enums & enm);
  private:
    device dev;
};

#endif // DEVICETREE_H
