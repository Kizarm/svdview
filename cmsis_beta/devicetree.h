#ifndef DEVICETREE_H
#define DEVICETREE_H

class DeviceTree;
class CmsisTree;
struct registerType;
struct peripheralType;
struct fieldType;

enum TYPES_WITH {
  TYPE_BIT = 0, TYPE_8BIT = 1, TYPE_16BIT = 2, TYPE_32BIT = 4, 
};

struct MandatoryPart {
  DeviceTree *  root;
  int           part_id;
  std::string   name;
  std::string   comment;
  unsigned long address;  // adresa, resp. offset, value
  TYPES_WITH    width;    // délka prvku
  unsigned long size;     // počet prvků s délkou width (>1 = pole)
  bool          unused;   // true pokud se nepoužívá - není v popisu XML
  explicit MandatoryPart (DeviceTree * p) noexcept : root(p), part_id(0),
           name(), comment(), address(0lu), width(TYPE_BIT), size(0lu), unused(false) {};
  virtual ~MandatoryPart () {};
};
struct EnumValuesPart {
  std::string   name;
  std::string   comment;
  std::string   value;
  explicit EnumValuesPart () noexcept : name(), comment(), value() {};
  virtual ~EnumValuesPart () {};
};
struct EnumPart {
  std::string                 name;
  std::vector<EnumValuesPart> values;
  explicit EnumPart () noexcept : name(), values() {};
  void add (const EnumValuesPart & evp) {
    values.push_back (evp);
  }
  virtual ~EnumPart () {};
};
struct RegisterPart;
struct FieldPart : public MandatoryPart {   // width je zde bit, ale používá se hodnota zděděná z registru
  unsigned              access;
  EnumPart              eenum;
  explicit FieldPart (DeviceTree * p) noexcept : MandatoryPart(p), access(0u), eenum() {};
  virtual ~FieldPart () {};
  void convert (RegisterPart & p, fieldType & f);
  void validate   ();
  void checkNames ();
};
struct RegisterPart : public MandatoryPart {
  std::vector<FieldPart>    fields;
  std::vector<RegisterPart> reg_union;      // pokud mají registry stejnou adresu, zabalíme do unionu
  std::string               baseName;       // jméno ze kterého registru je odvozen
  unsigned                  access;
  unsigned long             resetMask;
  unsigned long             resetValue;
  explicit RegisterPart (DeviceTree * p) noexcept : MandatoryPart(p), 
           fields(), reg_union(), baseName(), access(0u), resetMask(0ul), resetValue(0xFFFFFFFFul) {};
  virtual ~RegisterPart () {};
  void convert (const registerType * r);
  void validate   ();
  void checkNames ();
  void fillGaps   ();
};
struct InterruptPartC {
  const char * name;
  const char * comment;
  int          value;
};
struct InterruptPart {
  std::string  name;
  std::string  comment;
  long         value;
  explicit InterruptPart () noexcept : name(), comment(), value(0l) {};
  explicit InterruptPart (const InterruptPartC & ic) noexcept : name (std::string(ic.name)),
                          comment(std::string(ic.comment)), value(ic.value) {};
  virtual ~InterruptPart () {};
};
struct PeripheralPart : public MandatoryPart {
  std::string                groupName;     // celkem není podstatné
  std::string                baseName;      // jméno ze které periferie je odvozeno
  std::vector<RegisterPart>  registers;
  std::vector<InterruptPart> interrupts;
  unsigned long              struct_len;    // pro kontrolu délky struktur
  explicit PeripheralPart (DeviceTree * p) noexcept : MandatoryPart(p), 
           groupName(), baseName(), registers(), interrupts(), struct_len(0ul) {};
  virtual ~PeripheralPart () { };
  void validate   ();
  void convert    (const peripheralType * p);
  void checkNames ();
  void fillGaps   ();
  unsigned long makeUnion ();
};
class PRINTER;
/**
 * @class DeviceTree
 * @brief Popis paměti až na jednotlivé bity.
 * Velká část informace se sice ztratí, ale pro popis struktury stačí.
 * Problém je v tom, že to každý výrobce chápe jinak a rozumně to složit
 * z CMSIS popisu je skoro nemožné. Takhle z toho jdou generovat C/C++
 * použitelné hlavičky. Vzhledem k tomu, že originální hlavičky používaly
 * seskupování periferií do skupin, což generovalo příšerné napřehledné uniony,
 * a protože struktury popisující registry byly globálně viditelné 
 * (a měly tak dlouhá jména), rozhodl jsem se toto předělat.
 * V C++ nic z toho vlastně není potřeba a bylo to poněkud nedomyšlené -
 * např. některé timery STM32 neobsahují všechny bity a při prekladu nebylo
 * možné zjistit, zda nepoužíváme neexistující bit, vždy se to muselo
 * kontrolovat pomocí RM. Asi se nepovede vygenerovat hlavičku pro cokoli,
 * ale hodně toho půjde udělat. Cílem je, aby Printer byl co nejjednodušší
 * a šel udělat pro C++/C nebo i html.
 * */
class DeviceTree : public MandatoryPart {
  const CmsisTree            & cmsis;
  std::vector<PeripheralPart>  peripherals;
  std::vector<InterruptPart>   interrupts;    // vyčištěná kopie z periferií (odstraněné duplicity)
  std::string                  headerName;
  friend class PRINTER;
  public:
    explicit DeviceTree (const CmsisTree & p) noexcept;
    virtual ~DeviceTree () {}
    void     convert    ();
    const std::vector<PeripheralPart> & get_peripheral () const { return peripherals; }
    virtual void save  (const char * name = nullptr) = 0;
  protected:
    // validace - použít až v rodiči. Doplní mezery v polích a změní názvy (aby to šlo přeložit)
    void validate ();
    /////// hack for STM32 ////////
    void compareDevice    (const int depth = 0);
    bool comparePeripheral(const PeripheralPart & A, const PeripheralPart & B);
    bool compareRegisters (const RegisterPart   & A, const RegisterPart   & B);
    bool compareFields    (const FieldPart      & A, const FieldPart      & B);
    /**
     * @brief Systémová přerušení.
     * Tohle zřejmě nebude ničemu vadit. U různých procesorů to je sice jinak,
     * v popisu to obvykle nebývá, ale pokud to tam je, překryje se to.
     * Protože je to užitečné, přidáme to, lze to jednoduše vypnout.
     * Pokud cpu dané přerušení nemá (cortex-m0), patrně je to jedno.
     * Jsou to jen místa v tabulce, poloha vždy sedí, jestli je tam něco navíc
     * CPU to nekontroluje - výjimkou jsou procesory NXP, ale tam je to nutné
     * stejně řešit jinak (bootloader kontroluje checksum).
     * @param copy kam se to přidá
     */
    void addSystemInterrupts (std::vector<InterruptPart> & copy);
    void finalIndexing ();
};

#endif // DEVICETREE_H
