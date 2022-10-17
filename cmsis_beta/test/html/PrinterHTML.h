#ifndef PRINTERHTML_H
#define PRINTERHTML_H
#include "devicetree.h"

class PrinterHTML : public DeviceTree {
  public:
    explicit PrinterHTML (const CmsisTree & p) noexcept : DeviceTree (p) {}
    virtual ~PrinterHTML () {};
    void save (const char * name = nullptr) override;
    
    void dumpPeripherals (std::string & out);
    void dumpDescription (std::string & out, const int per, const int reg) const;
    void dumpDescription (std::string & out, const int per, const int reg, const int fie) const;
    void dumpPeripheral  (std::string & out, const int id) const;
  protected:
    void fillFieldGaps   ();
    void dumpRegister    (const RegisterPart & r, std::string & out, const int per) const;
    void dumpField       (const FieldPart    & f, std::string & out, const int per, const int reg) const;
    void dumpEnums       (std::string & out, const std::vector<EnumValuesPart> & values) const;
    
};

#endif // PRINTERHTML_H
