#ifndef DEVICE_DESC_H
#define DEVICE_DESC_H
#include <QRegExp>
#include <QVector>
#include <QString>

enum ACCESS {
  UNMODIFY   = 0,
  READ_ONLY  = 1,
  WRITE_ONLY = 2,
  READ_WRITE = 3,
};
struct IOreg;
struct group;
struct field;
struct f_enums;

struct f_enum {
  QString         name;
  QString         value;
  QString         comment;
  
  f_enum() : name(), value(), comment() {};
  void dump (QString & out, const f_enums * parent, const unsigned id=0) const;
};
struct f_enums {
  QString         name;
  QVector<f_enum> enums;
  
  f_enums() : name(), enums() {};
  void dump (QString & out, const field * parent, const unsigned id=0) const;
};
struct field {
  QString        name;
  QString        comment;
  unsigned long  size;
  unsigned long  offset;
  ACCESS         access;
  f_enums        values;
  
  field() : name(), comment(),size(1),offset(0),access(READ_WRITE),values() {};
  void dump (QString & out, const IOreg * parent, const unsigned id=0) const;
  void defs (QString & out, const IOreg * parent, const QString & prefix) const;
  void validate (const IOreg * parent);
};
struct IOreg {
  QString        name;
  QString        display;
  QString        altername;
  QString        comment;
  unsigned long  size;    // v bitech !!!
  unsigned long  offset;
  unsigned long  reset;
  unsigned long  array;   // 1 -> simple register, 0 chyba
  ACCESS         access;
  QVector<field> fields;
  QVector<IOreg> alternatives;
  
  IOreg() : name(), display(), altername(), comment(), size(0), offset(0),
            reset(0), array(1), access(READ_WRITE), fields(), alternatives() {};
  void dump (QString & out, const group * parent, const unsigned u, const unsigned id=0) const;
  void defs (QString & out, const group * parent, const unsigned id=0) const;
  void meth (QString & out, const group * parent, const unsigned id=0) const;
  void validate ();
  bool eq (IOreg & r, group * parent);
};
struct interrupt {
  QString name;
  QString comment;
  long    no;
  
  interrupt() : name(), comment(), no(0) {};
  void dump (QString & out, const unsigned id=0) const;
  void defs (QString & out, const unsigned id=0) const;
  void func (QString & out, const unsigned id=0) const;
  void validate ();
};
struct group {
  QString        name;
  QVector<IOreg> registers;
  unsigned long  size;
  int            rename;
  bool           system;      /*!< pokud nechám v konstruktoru false, vypíše se celé zařízení, včetně Cortex jádra.
  To vadí při použití CMSIS - předefinuje např. NVIC */
  
  group() : name(), registers(), size(0), rename(0), system (true) {};
  void dump (QString & out, const unsigned id=0) const;
  void defs (QString & out, const unsigned id=0) const;
  void validate ();
  bool eq (group & g);
  void append (IOreg & r);
  bool not_used () const;
};
struct peripheral {
  QString             name;
  QString             comment;
  QString             groupname;
  unsigned long       address;
  QVector<interrupt>  interrupts;
  group               registers;
  
  peripheral() : name(), comment(), groupname(), address(0), interrupts(), registers() {};
  void dump (QString & out, const bool plus=false, const unsigned id=0) const;
};
struct property {
  QString name;
  QString value;
  
  property() : name(), value() {};
  void dump (QString & out, const unsigned id=0) const;
};
struct processor {
  QVector<property>   properties;
  
  processor() : properties() {};
  void dump (QString & out, const unsigned id=0) const;
};

struct device {
  unsigned            default_width;
  QString             name;
  QVector<property>   properties;
  processor           cpu;
  QVector<peripheral> peripherals;
  // Tyto 2 polozky musime vytvorit pri validaci z jednotlivych periferii
  QVector<interrupt>  interrupts;
  QVector<group>      regsdef;
  
  device() : default_width(32), name(), properties(), cpu(), peripherals(), interrupts(), regsdef() {};
  void dump (QString & out, const unsigned id=0) const;
  void defs (QString & out, const unsigned id=0) const;
  void validate ();
  void validate (QVector<peripheral> & p);
  peripheral * derived (QString & old);
};
/** ********************************************************************************************/
class SWRAP {
public:
  SWRAP(){data=0;};
  SWRAP(const QString & s) : data(strdup(s.toLatin1().constData())) {};
  ~SWRAP() { if (data) free (data); };
  operator char const * () const {return data;};
  void operator= (const QString & s) {
    if (data) {free (data); data = 0;}
    data = strdup(s.toLatin1().constData());};
private:
  char * data;
};

static const int    name_width     = 16;
static const char * typedef_suffix = "_Type";
static const char * struct_suffix  = "_s";
static const char * modificators[] = {"    ",  "__RO",  "__WO",  "__RW"};
static const char * bitsmodifics[] = {"     ", "__ROB", "__WOB", "__RWB"};


#endif // DEVICE_DESC_H
