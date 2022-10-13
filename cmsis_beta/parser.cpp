#include "parser.h"

using namespace tinyxml2;
using namespace std;

static inline vector<Attribute> get_attributes (const XMLAttribute * attr) {
  vector<Attribute> result;
  for (const XMLAttribute * a = attr; a; a = a->Next()) {
    Attribute atr (a->Name(), a->Value());
    result.push_back (atr);
  }
  return result;
}
//////////////////////////////////////////////////
// Recursively navigates the XML and get rid of comments.
// https://stackoverflow.com/questions/43305359/parsing-comment-in-tinyxml2
void StripXMLInfo (XMLNode * node) {
  // All XML nodes may have children and siblings. So for each valid node, first we
  // iterate on it's (possible) children, and then we proceed to clear the node itself and jump
  // to the next sibling
  while (node) {
    if (node->FirstChild() != nullptr) StripXMLInfo (node->FirstChild());
    //Check to see if current node is a comment
    auto comment = dynamic_cast<XMLComment *> (node);
    if (comment) {
      // If it is, we ask the parent to delete this, but first move pointer
      // to next member so we don't get lost in a NULL reference
      node = node->NextSibling();
      comment->Parent()->DeleteChild (comment);
    } else  node = node->NextSibling();
  }
}
void traveler (Element * element, tinyxml2::XMLNode * node, const int depth) {
  for (XMLNode * n = node; n; n = n->NextSibling()) {
    XMLElement * elem = n->ToElement();
    if (!elem) continue;
    vector<Attribute>va = get_attributes (elem->FirstAttribute());
    const char * mname  = elem->Name();
    const char * elval  = elem->GetText() ? elem->GetText() : "";
    
    Element * newel = new Element (element, mname, elval);
    newel->setAttrs (va);
    element->append (newel);
    // printf("(%d) new {%s}{%s}\n", depth, mname, elval);
    
    XMLNode * child = n->FirstChild();
    if (child == nullptr) continue;
    traveler (newel, child, depth + 1);
  }
}
void Element::setAttrs (const std::vector<Attribute> & a) {
  for (auto & e: a) attributes.push_back (e);
}
