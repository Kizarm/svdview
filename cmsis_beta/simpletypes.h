#ifndef SIMPLETYPES_H_
#define SIMPLETYPES_H_

struct NumberPair {
  unsigned long from, to;
};
struct dimArrayIndexType;

struct stringType {
  std::string base;
  explicit stringType () noexcept : base() {};
  stringType & operator= (const std::string & s) { base = s; return * this; }
};
struct scaledNonNegativeInteger {
  unsigned long base;
  explicit scaledNonNegativeInteger () noexcept : base(0ul) {};
  static constexpr const char * const restriction = R"---([+]?(0x|0X|#)?[0-9a-fA-F]+[kmgtKMGT]?)---";
  scaledNonNegativeInteger & operator= (const std::string & s) {
    check_pattern (s, restriction);
    base = cmsis_toUlong (s);
    return * this;
  }
};
struct identifierType {
  std::string base;
  static constexpr const char * const restriction = R"---([_A-Za-z0-9]*)---";
  identifierType & operator= (const std::string & s) {
    check_pattern (s, restriction);
    base = s;
    return * this;
  }
};
struct dimableIdentifierType {
  std::string base;
  static constexpr const char * const restriction = R"---(((%s)|(%s)[_A-Za-z]{1}[_A-Za-z0-9]*)|([_A-Za-z]{1}[_A-Za-z0-9]*(\[%s\])?)|([_A-Za-z]{1}[_A-Za-z0-9]*(%s)?[_A-Za-z0-9]*))---";
  dimableIdentifierType & operator= (const std::string & s);
};
struct protectionStringType {
  std::string base;
  static constexpr const char * const restriction = R"---([snp])---";
  protectionStringType & operator= (const std::string & s) {
    check_pattern (s, restriction);
    base = s;
    return * this;
  }
};
struct dimIndexType {
  std::string base;
  static constexpr const char * const restriction = R"---([0-9]+\-[0-9]+|[A-Z]-[A-Z]|[_0-9a-zA-Z]+(,\s*[_0-9a-zA-Z]+)+)---";
  dimIndexType & operator= (const std::string & s) {
    check_pattern (s, restriction);
    base = s;
    return * this;
  }
};
struct boolPresentType {
  bool value;
  explicit boolPresentType () noexcept : value(false) {};
  static constexpr StringEnumerator restriction [4] = {{"false", 1u},{"true", 2u},{"0", 1u},{"1", 2u}};
  boolPresentType & operator= (const std::string & s) {
    const unsigned order = check_enumeration (s, restriction, 4);
    value = order > 1u ? true : false;
    return * this;
  }
};
struct accessType {
  std::string base;
  unsigned    order;
  explicit accessType () noexcept : base(), order(0u) {};
  static constexpr StringEnumerator restriction [5] = {{"read-only",1u},{"write-only",2u},{"read-write",3u},{"writeOnce",4u},{"read-writeOnce",5u},};
  accessType & operator= (const std::string & s) { order = check_enumeration(s, restriction, 5); base = s; return * this; }
};
struct dataTypeType {
  std::string base;
  unsigned order;
  explicit dataTypeType () noexcept : base(), order(0u) {};
  static constexpr StringEnumerator restriction [16] = {{"uint8_t",1u},{"uint16_t",2u},{"uint32_t",3u},{"uint64_t",4u},{"int8_t",5u},{"int16_t",6u},{"int32_t",7u},{"int64_t",8u},{"uint8_t *",9u},{"uint16_t *",10u},{"uint32_t *",11u},{"uint64_t *",12u},{"int8_t *",13u},{"int16_t *",14u},{"int32_t *",15u},{"int64_t *",16u},};
  dataTypeType & operator= (const std::string & s) { order = check_enumeration(s, restriction, 16); base = s; return * this; }
};
struct modifiedWriteValuesType {
  std::string base;
  unsigned order;
  explicit modifiedWriteValuesType () noexcept : base(), order(0u) {};
  static constexpr StringEnumerator restriction [9] = {{"oneToClear",1u},{"oneToSet",2u},{"oneToToggle",3u},{"zeroToClear",4u},{"zeroToSet",5u},{"zeroToToggle",6u},{"clear",7u},{"set",8u},{"modify",9u},};
  modifiedWriteValuesType & operator= (const std::string & s) { order = check_enumeration(s, restriction, 9); base = s; return * this; }
};
struct readActionType {
  std::string base;
  unsigned    order;
  explicit readActionType () noexcept : base(), order(0u) {};
  static constexpr StringEnumerator restriction [4] = {{"clear",1u},{"set",2u},{"modify",3u},{"modifyExternal",4u},};
  readActionType & operator= (const std::string & s) { order = check_enumeration(s, restriction, 4); base = s; return * this; }
};
struct bitRangeType {
  std::string base;
  static constexpr const char * const restriction = R"---(\[([0-4])?[0-9]:([0-4])?[0-9]\])---";
  const  NumberPair toNumberPair  () const;
  bitRangeType & operator= (const std::string & s) { check_pattern(s, restriction); base = s; return * this; }
};
struct enumUsageType {
  std::string base;
  unsigned    order;
  explicit enumUsageType () noexcept : base(), order(0u) {};
  static constexpr StringEnumerator restriction [3] = {{"read",1u},{"write",2u},{"read-write",3u},};
  enumUsageType & operator= (const std::string & s) { order = check_enumeration(s, restriction, 9); base = s; return * this; }
};
struct enumeratedValueDataType {
  std::string base;
  static constexpr const char * const restriction = R"---([+]?(((0x|0X)[0-9a-fA-F]+)|([0-9]+)|((#|0b)[01xX]+)))---";
  enumeratedValueDataType & operator= (const std::string & s) { check_pattern(s, restriction); base = s; return * this; }
};
struct cpuNameType {
  std::string base;
  unsigned    order;
  explicit cpuNameType () noexcept : base(), order(0u) {};
  static constexpr StringEnumerator restriction [28] = {{"CM0",1u},{"CM0PLUS",2u},{"CM0+",3u},{"CM1",4u},{"CM3",5u},{"CM4",6u},{"CM7",7u},{"CM23",8u},{"CM33",9u},{"CM35P",10u},{"CM55",11u},{"CM85",12u},{"SC000",13u},{"SC300",14u},{"ARMV8MML",15u},{"ARMV8MBL",16u},{"ARMV81MML",17u},{"CA5",18u},{"CA7",19u},{"CA8",20u},{"CA9",21u},{"CA15",22u},{"CA17",23u},{"CA53",24u},{"CA57",25u},{"CA72",26u},{"SMC1",27u},{"other",28u},};
  cpuNameType & operator= (const std::string & s) { order = check_enumeration(s, restriction, 28u); base = s; return * this; }
};
struct revisionType {
  std::string base;
  static constexpr const char * const restriction = R"---(r[0-9]*p[0-9]*)---";
  explicit revisionType () noexcept : base() {};
  revisionType & operator= (const std::string & s) {
    check_pattern (s, restriction);
    base = s;
    return * this;
  }
};
struct endianType {
  std::string base;
  unsigned    order;
  static constexpr StringEnumerator restriction [4] = {{"little",1u},{"big",2u},{"selectable",3u},{"other",4u},};
  explicit endianType () noexcept : base() {};
  endianType & operator= (const std::string & s) {
    order = check_enumeration (s, restriction, 4);
    base  = s;
    return * this;
  }
};
///////////////////////// zatím nepoužito ///////////////////////////
struct descriptionStringType {
  std::string base;
  static constexpr const char * const restriction = R"---([\p{IsBasicLatin}\p{IsLatin-1Supplement}]*)---";
};
struct sauAccessType {
  std::string base;
  static constexpr const char * const restriction = R"---([cn])---";
};

#endif //  SIMPLETYPES_H_
