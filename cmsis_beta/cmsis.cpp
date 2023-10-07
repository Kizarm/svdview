#include <fstream>
#include <iostream>
#include <filesystem>
#include "cmsistree.h"
#include "devicetree.h"
#include "PrinterHpp.h"
#include "cmsis.h"

using namespace tinyxml2;

static bool process_file (const std::string & filename, CmsisTree & tree) {
  std::cout << filename << "\n";
  XMLDocument doc;
  XMLError eResult = doc.LoadFile (filename.c_str());
  if (eResult != XML_SUCCESS) return false;
  StripXMLInfo (doc.FirstChild());
  XMLNode * pRoot = doc.FirstChild()->NextSibling();
  if (pRoot == nullptr)       return false;
  Element     elem (nullptr, "root","");
  traveler (& elem, pRoot, 0);
  // 1. stupeň - převedeme xml do lépe zpracovatelného tvaru, xml zde zapomeneme
  tree.append (& elem);
  return true;
}
int create_from_svd (const char * filename, const enum PRINT_ENUMS pe) {
  
  CmsisTree tree;
  if (!process_file (filename, tree)) return -1;
  // 2. stupeň - složitý tvar cmsis převedeme do jednoduššího tvaru
  DeviceTree * dev = new PRINTER (tree, pe);
  std::cout << "converting to device ...\n";
  dev->convert();
  std::cout << "device converted OK.\n";
  dev->save   ();   // a ten už můžeme zapsat do souboru (název by měl být už v xml)
  delete dev;
  
  return 0;
}
