#ifndef PARSER_H
#define PARSER_H
#include <vector>
#include <string>
#include "./tinyxml2/tinyxml2.h"

/**
 * @class Element
 * @brief Kopie XML dokumentu.
 * Beze ztráty informace, původní XML parser je dost jednoduchá knihovna,
 * ale blbě se prochází, takže strom zkopírujeme do něčeho jednoduššího.
 * */
struct Attribute {
  std::string name, value;
  public:
    explicit Attribute (const char * n, const char * v) noexcept : name(n), value(v) {};
};
struct Element {
  Element   * parent;
  std::string name, value;

  std::vector<Attribute> attributes;
  /// Child musí být objekt vytvořený na haldě, odstraní jej korektně destruktor tt. třídy.
  std::vector<Element *> childs;
  public:
    explicit Element (Element * p, const char * n, const char * v) noexcept :
                      parent(p), name(n), value(v), attributes(), childs() {};
    explicit Element (const Element & o) noexcept :
                      parent (o.parent), name (o.name), value (o.value),
                      attributes (o.attributes), childs (o.childs) {};
    ~Element () { for (auto & e: childs) delete e; };
    void setAttrs (const std::vector<Attribute> & a);
    void append   (Element * e) { childs.push_back (e); };
};
extern void StripXMLInfo    (tinyxml2::XMLNode * node);
extern void traveler        (Element * element, tinyxml2::XMLNode * node, const int depth);

#endif // PARSER_H
