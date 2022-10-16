#ifndef PRINTERHPP_H
#define PRINTERHPP_H
#include "devicetree.h"

enum PRINT_ENUMS {
  DECLARE_ONLY = 0,
  OLD_STYLE, NEW_STYLE,
};

class PrinterHpp : public DeviceTree {
  PRINT_ENUMS m_eprt;
  public:
    explicit PrinterHpp (const CmsisTree & p, const PRINT_ENUMS e = DECLARE_ONLY) noexcept :
                         DeviceTree(p), m_eprt (e) {}
    virtual ~PrinterHpp () {}
    void save (const char * name = nullptr) override;
  protected:
    void print (std::string & out);
    void printCpu              (std::string & out);
    void printInterrupts       (std::string & out, const int indent);
    void printStaticAsserts    (std::string & out, const int indent);
    void printPerpheralAddress (PeripheralPart & p, std::string & out, const int indent);
    void printPerpheralDef     (PeripheralPart & p, std::string & out, const int indent);
    
    void printRegisters        (PeripheralPart & p, std::string & out);
    void printRegDef           (RegisterPart   & r, std::string & out, const int indent);
    void printRegInst          (RegisterPart   & r, std::string & out, const int indent);
    void printRegSimple        (RegisterPart   & r, std::string & out, const int indent);
    void printMethods          (const std::string & regdef, const unsigned access, const unsigned long resetValue, std::string & out);
    
    void printFields           (RegisterPart   & r, std::string & out);
    void printEnumerations     (RegisterPart   & r, std::string & out);
    
    void StartupTemplate       (std::string & name);
    std::string determine_type (FieldPart & f);
    size_t determine_type_len  (RegisterPart & r);
};

#endif // PRINTERHPP_H
