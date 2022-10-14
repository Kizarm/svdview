#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>
#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include "cmsistree.h"
#include "devicetree.h"
#include "PrinterHTML.h"
#include "decompress.h"

using namespace emscripten;
using namespace tinyxml2;

extern void setXml (const std:: string & xml);
extern std::string fromString ();

std::ostringstream CERR;
static PrinterHTML * PrinterInstance = nullptr;

void Init() {
  std::cout  << "Module Initialized\n";
}

std::string Convert () {
  Decompress dec;           // data jsou zde schována zabalena do C-čkového souboru, je to menší
  setXml (dec.get());
  return fromString();
}
std::string PrintPerif (const int order) {
  std::string result;
  if (!PrinterInstance) return result;
  PrinterInstance->dumpPeripheral (result, order);
  return result;
}
std::string PrintReg (const int per, const int reg) {
  std::string result;
  if (!PrinterInstance) return result;
  PrinterInstance->dumpDescription(result, per, reg);
  return result;
}
std::string PrintField (const int per, const int reg, const int fie) {
  // std::cout << "peripheral: " << per << ", register: " << reg << ", field: " << fie << "\n";
  std::string result;
  if (!PrinterInstance) return result;
  PrinterInstance->dumpDescription(result, per, reg, fie);
  return result;
}
void setXml (const std:: string & xml) {
  if (xml.empty()) return;
  std::cout << "size of file = " << xml.size() << "\n";
  CmsisTree tree;
  XMLDocument doc;
  XMLError eResult = doc.Parse (xml.c_str());
  if (eResult != XML_SUCCESS) return;
  StripXMLInfo (doc.FirstChild());
  XMLNode * pRoot = doc.FirstChild()->NextSibling();
  if (pRoot == nullptr)       return;
  Element     elem (nullptr, "root","");
  traveler (& elem, pRoot, 0);
  // 1. stupeň - převedeme xml do lépe zpracovatelného tvaru, xml zde zapomeneme
  tree.append (& elem);
  
  if (PrinterInstance) {
    delete PrinterInstance;
    PrinterInstance = nullptr;
  }
  PrinterInstance = new PrinterHTML (tree);
  std::cout << "converting to device ...\n";
  PrinterInstance->convert();
  std::cout << "device converted OK.\n";
}
std::string PrintERR () {
  std::string result, s (CERR.str());
  result += "<p class=\"err\">";
  if (s.empty()) result += "OK";
  else           result += s;
  result += "</p>\n";
  CERR.str(std::string()); // clear CERR
  return result;
}
std::string fromString () {
  std::string result;
  if (!PrinterInstance) return result;
  PrinterInstance->dumpPeripherals (result);
  return result;
}
EMSCRIPTEN_BINDINGS (cmsis) {
  function ("Init",       Init);
  function ("Convert",    Convert);
  function ("PrintPerif", PrintPerif);
  function ("setXml",     setXml);
  function ("fromString", fromString);
  function ("PrintERR",   PrintERR);
  // description
  function ("PrintReg",   PrintReg);
  function ("PrintField", PrintField);
}
