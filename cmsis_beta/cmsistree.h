#ifndef CMSISTREE_H
#define CMSISTREE_H
#include <map>
#include "parser.h"
#include "utils.h"
#include "simpletypes.h"

/** SCHEMA
xmlns:xs = "http://www.w3.org/2001/XMLSchema"
elementFormDefault = "qualified"
attributeFormDefault = "qualified"
version = "1.3"
*/
struct writeConstraintType {
  bool                      writeAsRead;
  bool                      useEnumeratedValues;
  scaledNonNegativeInteger  minimum;
  scaledNonNegativeInteger  maximum;
  void parse (const Element * e);
};
struct addressBlockType {
  scaledNonNegativeInteger  offset;
  scaledNonNegativeInteger  size;
  stringType                usage;
  protectionStringType      protection;
};
struct interruptType {
  stringType                name;
  stringType                description;
  long                      value;
};
struct registerPropertiesGroup {  // Register Default Properties
  scaledNonNegativeInteger  size;
  accessType                access;         // nevyskytuje se
  protectionStringType      protection;
  scaledNonNegativeInteger  resetValue;
  scaledNonNegativeInteger  resetMask;
  virtual ~registerPropertiesGroup () {};
};
struct bitRangeLsbMsbStyle {
  scaledNonNegativeInteger  lsb;
  scaledNonNegativeInteger  msb;
};
struct bitRangeOffsetWidthStyle {
  scaledNonNegativeInteger  bitOffset;
  scaledNonNegativeInteger  bitWidth;
};
struct unusedType {
  void parse (const Element * e);
};
struct dimElementGroup {
  scaledNonNegativeInteger  dim;
  scaledNonNegativeInteger  dimIncrement;
  dimIndexType              dimIndex;
  identifierType            dimName;
//std::vector<dimArrayIndexType>  dimArrayIndex;
  unusedType                dimArrayIndex;
  virtual ~dimElementGroup () {};
};
struct enumeratedValueType {
  identifierType            name;
  stringType                description;
  enumeratedValueDataType   value;
  bool                      isDefault;
  
  void parse       (const Element * e);
};
struct dimArrayIndexType {
  identifierType            headerEnumName;
  enumeratedValueType       enumeratedValue;  // TODO - taky bude vector
};
struct enumerationType {
  identifierType            name;
  identifierType            headerEnumName;
  enumUsageType             usage;
  std::vector<enumeratedValueType>  enumeratedValue;
  
  virtual ~enumerationType() {};
  void parse       (const Element * e);
  void parseChilds (const Element * e);
};
struct fieldType : public dimElementGroup {
  // derivedFrom - neni pouzito
  dimableIdentifierType     name;
  stringType                description;
  NumberPair                range;
  /*
  bitRangeType              bitRange;  // choice bitRange:
  scaledNonNegativeInteger  bitOffset; // or bitOffset - bitWidth
  scaledNonNegativeInteger  bitWidth;
  scaledNonNegativeInteger  lsb;       // or lsb - msb
  scaledNonNegativeInteger  msb;
  */
  accessType                access;
  modifiedWriteValuesType   modifiedWriteValues;
  writeConstraintType       writeConstraint;
  readActionType            readAction;
  enumerationType           enumeratedValues;
  
  virtual ~fieldType() {};
  void parse       (const Element * e);
  void parseChilds (const Element * e);
};
struct fieldsType {
  std::vector<fieldType>    field;
  virtual ~fieldsType() {};
  void parse       (const Element * e);
};
struct registerType : public registerPropertiesGroup, dimElementGroup {
  dimableIdentifierType     name;
  stringType                displayName;
  stringType                description;
  identifierType            alternateGroup;
  dimableIdentifierType     alternateRegister;
  scaledNonNegativeInteger  addressOffset;
  dataTypeType              dataType;
  modifiedWriteValuesType   modifiedWriteValues;
  writeConstraintType       writeConstraint;
  readActionType            readAction;
  fieldsType                fields;
  
  registerType            * derivedFrom;
  
  virtual ~registerType () {};
  void parse       (const Element * e);
  void parseChilds (const Element * e);
};
struct clusterType {
  dimableIdentifierType     name;
  stringType                description;
  dimableIdentifierType     alternateCluster;
  identifierType            headerStructName;
  scaledNonNegativeInteger  addressOffset;
  registerType         *    _register;
  clusterType          *    cluster; // TODO
  
  void parse (const Element * e);
};
struct registersType {
  std::vector<clusterType*>    cluster;
  std::vector<registerType*>   _register;
  
  virtual ~registersType () {
    for (auto e: _register) delete e;
    for (auto e:   cluster) delete e;
  };
  void parse (const Element * e);
};
//////////////////////////////////////////
struct cpuType {
  // TODO : neobsahuje nic uzitecneho, vynechame
  std::map<std::string,std::string> data;
  /*
  cpuNameType               name;
  revisionType              revision;
  endianType                endian;
  bool                      mpuPresent;
  bool                      fpuPresent;
  bool                      fpuDP;
  bool                      dspPresent;
  bool                      icachePresent;
  bool                      dcachePresent;
  bool                      itcmPresent;
  bool                      dtcmPresent;
  bool                      vtorPresent;
  scaledNonNegativeInteger  nvicPrioBits;
  bool                      vendorSystickConfig;
  scaledNonNegativeInteger  deviceNumInterrupts;
  scaledNonNegativeInteger  sauNumRegions;
  struct  {
    scaledNonNegativeInteger  base;
    scaledNonNegativeInteger  limit;
    sauAccessType             access;
  } cosi;  // TODO
  */
  void parse (const Element * e);
};
struct peripheralType : public registerPropertiesGroup, dimElementGroup {
  dimableIdentifierType     name;                     // mandatory
  stringType                version;
  stringType                description;
  dimableIdentifierType     alternatePeripheral;
  stringType                groupName;
  identifierType            prependToName;
  identifierType            appendToName;
  dimableIdentifierType     headerStructName;
  stringType                disableCondition;
  scaledNonNegativeInteger  baseAddress;              // mandatory
  registersType             registers;
  std::vector<addressBlockType>    addressBlock;
  std::vector<interruptType>       interrupt;
  
  peripheralType          * derivedFrom;
  
  virtual ~peripheralType () {};
  void parse (const Element * e);
  void parseAdressBlock (const Element * e);
  void parseInterrupt   (const Element * e);
};
struct peripheralsType {
  std::vector<peripheralType*> peripheral;            // pointer kvůli derivedFrom
  virtual ~peripheralsType () { for (auto e: peripheral) delete e; };
  void parse (const Element * e);
};
struct VendorExtensions {
  // TODO : neobsahuje nic uzitecneho, vynechame, ale popis pameti by se mozna hodil
  std::map<std::string,std::string> data;
  void parse (const Element * e);
};
struct Device : public registerPropertiesGroup {
  scaledNonNegativeInteger  addressUnitBits;          // mandatory
  cpuType                   cpu;                      //
  stringType                description;              // mandatory
  identifierType            headerDefinitionsPrefix;  //
  identifierType            headerSystemFilename;     //
  stringType                licenseText;              //
  identifierType            name;                     // mandatory
  peripheralsType           peripherals;              // mandatory
  stringType                series;                   //
  stringType                vendor;                   //
  VendorExtensions          vendorExtensions;         //
  identifierType            vendorID;                 //
  stringType                version;                  // mandatory
  scaledNonNegativeInteger  width;                    // mandatory
  
  virtual ~Device () {};
  void parse (const Element * e);

};
//////////////////////////////////////////////////////
/**
 * @class CmsisTree
 * @brief Zde je podstatná část popisu SVD.
 * Celý strom SVD je hodně složitý, zde je pokus o jeho popis podle
 * <a href="https://www.keil.com/pack/doc/CMSIS/SVD/html/index.html">standardu</a>.
 * Některé položky vypadly, ale asi nebudou podstatné a dají se dodělat, v textu
 * je to označeno TODO.
 * */
class CmsisTree {
  public:
    Device  device;
  public:
    explicit CmsisTree () noexcept : device() {}
    virtual ~CmsisTree () {};
    void append     (const Element * e);
    void parse      (const Element * e);
  protected:
};

#endif // CMSISTREE_H
