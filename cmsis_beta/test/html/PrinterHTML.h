#ifndef PRINTERHTML_H
#define PRINTERHTML_H
#include "devicetree.h"

class PrinterHTML : public DeviceTree {
  public:
    explicit PrinterHTML (const CmsisTree & p) noexcept : DeviceTree (p) {}
    virtual ~PrinterHTML () {};
    void save (const char * name = nullptr) override;
    
    void dumpPeripherals (std::string & out);
    void dumpPeripheral  (std::string & out, const int id);
    void dumpDescription (std::string & out, const int per, const int reg);
    void dumpDescription (std::string & out, const int per, const int reg, const int fie);
    void fillFieldGaps   ();
  protected:
    void fillFieldGaps   (std::vector<FieldPart> & fld);
    void dumpRegister    (const RegisterPart & r, std::string & out, const int per);
    void dumpField       (const FieldPart    & f, std::string & out, const int per, const int reg);
    void dumpEnums       (std::string & out, const std::vector<EnumValuesPart> & values);
    
};

#endif // PRINTERHTML_H
