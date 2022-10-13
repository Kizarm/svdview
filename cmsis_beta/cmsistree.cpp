#include <iostream>
#include <functional>
#include "cmsistree.h"

using namespace std;

void unusedType::parse(const Element * e) {
  CERR << "extra element " << e->name << '\n';
}
dimableIdentifierType & dimableIdentifierType::operator=(const std::string & s) {
  check_pattern (s, restriction);
  base = s;
//cout << "dimableIdentifierType: " << base << '\n';
  return * this;
}
const NumberPair bitRangeType::toNumberPair() const {
  NumberPair np;
  unsigned index = 0;
  vector<string> pair (2);
  for (const char c: base) {
    if      (c == '[') index = 0;
    else if (c == ']') index = 0;
    else if (c == ':') index = 1;
    else pair [index] += c;
  }
  np.from = cmsis_toUlong(pair.at(0));
  np.to   = cmsis_toUlong(pair.at(1));
  if (np.from > np.to) {
    const unsigned long tmp = np.from;
    np.from = np.to;
    np.to   = tmp;
  }
  return np;
}
//////////////////////////////////////////////////

void CmsisTree::append(const Element * e) {
  for (const Element * ch: e->childs) parse (ch);
}
void CmsisTree::parse(const Element * e) {
//printf("CmsisTree::parse: %s\n", e->name.c_str());
  if (e->name == "device") {
    for (const Element * ch: e->childs) device.parse (ch);
  } else {
    CERR << "! CmsisTree::parse\n";
  }
  // printf("sizeof device = %zd\n", sizeof (device));
}
void Device::parse(const Element * e) {
//printf("Device::parse: %s\n", e->name.c_str());
  map<string,function<void()>> dev_map = {    // je to přehlednější a patrně i rychlejší prohledávání
    // registerPropertiesGroup :
    {"size",                    [this, e]() { size                    = e->value; } },
    {"access",                  [this, e]() { access                  = e->value; } },
    {"protection",              [this, e]() { protection              = e->value; } },
    {"resetMask",               [this, e]() { resetMask               = e->value; } },
    {"resetValue",              [this, e]() { resetValue              = e->value; } },
    // end registerPropertiesGroup
    {"addressUnitBits",         [this, e]() { addressUnitBits         = e->value; } },
    {"cpu",                     [this, e]() { for (const Element * ch : e->childs) cpu.parse (ch); } },
    {"description",             [this, e]() { description             = e->value; } },
    {"headerDefinitionsPrefix", [this, e]() { headerDefinitionsPrefix = e->value; } },
    {"headerSystemFilename",    [this, e]() { headerSystemFilename    = e->value; } },
    {"licenseText",             [this, e]() { licenseText             = e->value; } },
    {"name",                    [this, e]() { name                    = e->value; } },
    {"peripherals",             [this, e]() { for (const Element * ch : e->childs) peripherals.parse (ch); } },
    {"series",                  [this, e]() { series                  = e->value; } },
    {"vendor",                  [this, e]() { vendor                  = e->value; } },
    {"vendorExtensions",        [this, e]() { vendorExtensions.parse(e); } },
    {"vendorID",                [this, e]() { vendorID                = e->value; } },
    {"version",                 [this, e]() { version                 = e->value; } },
    {"width",                   [this, e]() { width                   = e->value; } },
  //{"", [this, e]() { = e->value; } },
  };
  auto f = dev_map [e->name];
  if (f) f ();
  else  CERR << "! Device::parse \"" << e->name << "\"\n";
}
void VendorExtensions::parse(const Element * e) {
//printf(" VendorExtensions::parse: %s=\"%s\"\n", e->name.c_str(), e->value.c_str());
  for (const Element * ch: e->childs) {
    data [ch->name] = ch->value;
  }
}
void cpuType::parse(const Element * e) {
//printf(" cpuType::parse: %s=\"%s\"\n", e->name.c_str(), e->value.c_str());
  for (const Element * ch: e->childs) {
    data [ch->name] = ch->value;
  }
}
void peripheralsType::parse(const Element * e) {
//printf(" peripheralsType::parse: %s\n", e->name.c_str());
  if (e->name == "peripheral") {
    peripheralType * per = new peripheralType();
    for (const Element * ch: e->childs) per->parse (ch);
    const vector<Attribute> & attrs = e->attributes;
    for (auto & a: attrs) {
      if (a.name != "derivedFrom") continue;
      for (auto & p: peripheral) {
        if (p->name.base == a.value) per->derivedFrom = p;
      }
    }    
    peripheral.push_back (per);
  } else {
    CERR << "! peripheralsType::parse \"" << e->name << "\"\n";
  }
}
void peripheralType::parse(const Element * e) {
//printf("  peripheralType::parse: %s\n", e->name.c_str());
  map<string,function<void()>> per_map = {
    // registerPropertiesGroup :
    {"size",                [this, e]() { size                = e->value; } },
    {"access",              [this, e]() { access              = e->value; } },
    {"protection",          [this, e]() { protection          = e->value; } },
    {"resetMask",           [this, e]() { resetMask           = e->value; } },
    {"resetValue",          [this, e]() { resetValue          = e->value; } },
    // dimElementGroup :
    {"dim",                 [this, e]() { dim                 = e->value; } },
    {"dimIncrement",        [this, e]() { dimIncrement        = e->value; } },
    {"dimIndex",            [this, e]() { dimIndex            = e->value; } },
    {"dimName",             [this, e]() { dimName             = e->value; } },
    {"dimArrayIndex",       [this, e]() { dimArrayIndex.parse(e); } },
    // end dimElementGroup
    {"name",                [this, e]() { name                = e->value; } },
    {"version",             [this, e]() { version             = e->value; } },
    {"description",         [this, e]() { description         = e->value; } },
    {"alternatePeripheral", [this, e]() { alternatePeripheral = e->value; } },
    {"groupName",           [this, e]() { groupName           = e->value; } },
    {"prependToName",       [this, e]() { prependToName       = e->value; } },
    {"appendToName",        [this, e]() { appendToName        = e->value; } },
    {"headerStructName",    [this, e]() { headerStructName    = e->value; } },
    {"disableCondition",    [this, e]() { disableCondition    = e->value; } },
    {"baseAddress",         [this, e]() { baseAddress         = e->value; } },
    {"addressBlock",        [this, e]() { parseAdressBlock (e); } },
    {"interrupt",           [this, e]() { parseInterrupt   (e); } },
    {"registers",           [this, e]() { registers.parse  (e); } },
  };
  auto f = per_map [e->name];
  if (f) f ();
  else  CERR << "! peripheralType::parse \"" << e->name << "\"\n";
}
void peripheralType::parseAdressBlock(const Element * e) {
  addressBlockType ab;
  for (auto ch: e->childs) {
    if (ch->name == "offset") {
      ab.offset = ch->value;
    } else if (ch->name == "size") {
      ab.size = ch->value;
    } else if (ch->name == "usage") {
    } else if (ch->name == "protection") {
      ab.protection = ch->value;
    } else {
      CERR << "! peripheralType::parseAdressBlock \"" << ch->name << "\"\n";
    }
  }
  addressBlock.push_back (ab);
}
void peripheralType::parseInterrupt(const Element * e) {
  interruptType ni;
  for (auto ch: e->childs) {
    if (ch->name == "name") {
      ni.name = ch->value;
    } else if (ch->name == "description") {
      ni.description = ch->value;
    } else if (ch->name == "value") {
      ni.value = stoi (ch->value);
    } else {
      CERR << "! peripheralType::parseInterrupt \"" << ch->name << "\"\n";
    }
  }
  interrupt.push_back (ni);
}
void registersType::parse(const Element * e) {
  for (auto ch: e->childs) {
    if (ch->name == "register") {
      registerType * nr = new registerType();
      nr->parse (ch);
      _register.push_back (nr);
    } else if (ch->name == "cluster") {
      CERR << "clusters not supported\n";
    } else {
      CERR << "! registersType::parse \"" << ch->name << "\"\n";
    }
  }
}
void registerType::parse(const Element * e) {
  if (e->attributes.size()) CERR << "TODO registerType::parse derivedFrom\n";
  for (auto ch: e->childs) {
    parseChilds (ch);
  }
}
void registerType::parseChilds(const Element * e) {
//printf("  registerType::parseChild: %s\n", e->name.c_str());
  map<string,function<void()>> reg_map = {
    // registerPropertiesGroup :
    {"size",                [this, e]() { size                = e->value; } },
    {"access",              [this, e]() { access              = e->value; } },
    {"protection",          [this, e]() { protection          = e->value; } },
    {"resetMask",           [this, e]() { resetMask           = e->value; } },
    {"resetValue",          [this, e]() { resetValue          = e->value; } },
    // dimElementGroup :
    {"dim",                 [this, e]() { dim                 = e->value; } },
    {"dimIncrement",        [this, e]() { dimIncrement        = e->value; } },
    {"dimIndex",            [this, e]() { dimIndex            = e->value; } },
    {"dimName",             [this, e]() { dimName             = e->value; } },
    {"dimArrayIndex",       [this, e]() { dimArrayIndex.parse(e); } },
    // end dimElementGroup
    {"name",                [this, e]() { name                = e->value; } },
    {"displayName",         [this, e]() { displayName         = e->value; } },
    {"description",         [this, e]() { description         = e->value; } },
    {"alternateGroup",      [this, e]() { alternateGroup      = e->value; } },
    {"alternateRegister",   [this, e]() { alternateRegister   = e->value; } },
    {"addressOffset",       [this, e]() { addressOffset       = e->value; } },
    {"dataType",            [this, e]() { dataType            = e->value; } },
    {"modifiedWriteValues", [this, e]() { modifiedWriteValues = e->value; } },
    {"writeConstraint",     [this, e]() { writeConstraint.parse (e);      } },
    {"readAction",          [this, e]() { readAction          = e->value; } },
    {"fields",              [this, e]() { fields.parse (e); } },
  };
  auto f = reg_map [e->name];
  if (f) f ();
  else  CERR << "! registerType::parseChilds \"" << e->name << "\"\n";
}
void writeConstraintType::parse(const Element * e) {
  CERR << "! writeConstraintType::parse \"" << e->name << "\" TODO \n";
}
void clusterType::parse(const Element * e) {
}
void fieldsType::parse(const Element * e) {
  for (auto ch: e->childs) {
    // printf("%s\n", ch->name.c_str());
    if (ch->name == "field") {
      fieldType nf;
      nf.parse (ch);
      field.push_back (nf);
    } else {
      CERR << "! fieldsType::parse \"" << e->name << "\"\n";
    }
  }
}
void fieldType::parse(const Element * e) {
  //printf("%s\n", e->name.c_str());
  for (auto ch: e->childs) {
      parseChilds (ch);
  }
}
void fieldType::parseChilds(const Element * e) {
  // printf("  %s\n", e->name.c_str());
  map<string,function<void()>> field_map = {
    // dimElementGroup :
    {"dim",                 [this, e]() { dim                 = e->value; } },
    {"dimIncrement",        [this, e]() { dimIncrement        = e->value; } },
    {"dimIndex",            [this, e]() { dimIndex            = e->value; } },
    {"dimName",             [this, e]() { dimName             = e->value; } },
  //{"dimArrayIndex",       [this, e]() {  } },    // obslouzit jako vector
    // end dimElementGroup
    {"name",                [this, e]() { name                = e->value; } },
    {"description",         [this, e]() { description         = e->value; } },
    {"bitRange",            [this, e]() { bitRangeType brt; brt = e->value; range = brt.toNumberPair(); } },
    {"bitOffset",           [this, e]() { range.from = cmsis_toUlong(e->value); } },
    {"bitWidth",            [this, e]() { range.to   = range.from + cmsis_toUlong(e->value) - 1ul; } },
    {"lsb",                 [this, e]() { range.from = cmsis_toUlong(e->value); } }, // tohle neni otestovano
    {"msb",                 [this, e]() { range.to   = cmsis_toUlong(e->value); } }, // tohle neni otestovano
    {"access",              [this, e]() { access              = e->value; } },
    {"modifiedWriteValues", [this, e]() { modifiedWriteValues = e->value; } },
    {"writeConstraint",     [this, e]() { writeConstraint.parse(e); } },
    {"readAction",          [this, e]() { readAction          = e->value; } },
    {"enumeratedValues",    [this, e]() { enumeratedValues.parse(e); } },
  };
  auto f = field_map [e->name];
  if (f) f ();
  else  CERR << "! fieldType::parseChilds \"" << e->name << "\"\n";
}
void enumerationType::parse(const Element * e) {
  for (auto ch: e->childs) {
      parseChilds (ch);
  }
}
void enumerationType::parseChilds(const Element * e) {
  // printf("%s\n", e->name.c_str());
  map<string,function<void()>> enum_map = {
    {"name",            [this, e]() { name            = e->value; } },
    {"headerEnumName",  [this, e]() { headerEnumName  = e->value; } },
    {"usage",           [this, e]() { usage           = e->value; } },
    {"enumeratedValue", [this, e]() {  // vector
      enumeratedValueType ne;
      for (auto ch: e->childs) ne.parse (ch);
      enumeratedValue.push_back (ne);
    } },
  };
  auto f = enum_map [e->name];
  if (f) f ();
  else  CERR << "! enumerationType::parseChilds \"" << e->name << "\"\n";
}

void enumeratedValueType::parse(const Element * e) {
  //printf("  %s\n", e->name.c_str());
  map<string,function<void()>> enum_map = {
    {"name",         [this, e]() { name        = e->value; } },
    {"description",  [this, e]() { description = e->value; } },
    {"value",        [this, e]() { value       = e->value; } },
    // je to sice použito NXP, ale není to podstatné, skutečně je to malým písmem
    {"isDefault",    [this, e]() { if (e->value == "true") { isDefault = true; } else { isDefault = false ; } } },
  };
  auto f = enum_map [e->name];
  if (f) f ();
  else  CERR << "! enumeratedValueType::parse \"" << e->name << "\"\n";
}
