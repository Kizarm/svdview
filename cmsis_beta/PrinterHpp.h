#ifndef PRINTERHPP_H
#define PRINTERHPP_H
#include "devicetree.h"
#include "cmsis.h"

class PrinterHpp : public DeviceTree {
  PRINT_ENUMS m_eprt;
  public:
    explicit PrinterHpp (const CmsisTree & p, const PRINT_ENUMS e = DECLARE_ONLY) noexcept :
                         DeviceTree(p), m_eprt (e) {}
    virtual ~PrinterHpp () {}
    void save (const char * name = nullptr) override;
  protected:
    void print (std::string & out);
    void printCpu              (std::string & out) const;
    void printInterrupts       (std::string & out, const int indent) const;
    void printStaticAsserts    (std::string & out, const int indent) const;
    void printPerpheralAddress (const PeripheralPart & p, std::string & out, const int indent) const;
    void printPerpheralDef     (const PeripheralPart & p, std::string & out, const int indent) const;
    
    void printRegisters        (const PeripheralPart & p, std::string & out) const;
    void printRegDef           (const RegisterPart   & r, std::string & out, const int indent) const;
    void printRegInst          (const RegisterPart   & r, std::string & out, const int indent) const;
    void printRegSimple        (const RegisterPart   & r, std::string & out, const int indent) const;
    void printMethods          (const std::string & regdef, const unsigned access, const unsigned long resetValue, std::string & out) const;
    
    void printFields           (const RegisterPart   & r, std::string & out) const;
    void printEnumerations     (const RegisterPart   & r, std::string & out) const;
    
    void StartupTemplate       (std::string & name) const;
    std::string determine_type (const FieldPart & f) const;
    size_t determine_type_len  (const RegisterPart & r) const;
};

#endif // PRINTERHPP_H
