#include <iostream>
#include <algorithm>
#include "cmsistree.h"
#include "devicetree.h"

using namespace std;
/** V popisech STM32 jsou duplicitní popisy (vzhledem k C/C++ hlavičkám) GPIO.
 * Nechápu proč, zřejmě je to nějak automaticky generováno z HDL periferií
 * a nikdo to už nekontroluje. Sice to nějak extra nepřekáží, ale zbytečně
 * to prodlužuje hlavičku a vadí to při zobecnění (abstrakce další vrstvy).
 * */
    /////// hack for STM32 ////////
void DeviceTree::compareDevice(const int depth) {
  // return;
  if (name.find ("STM32") == string::npos) return;
  printf ("%d.STM32 - Checking peripherals duplicity ...\n", depth);
  for (auto & A: peripherals) {
    if (A.registers.empty()) continue;
    for (auto & B: peripherals) {
      if (B.registers.empty())        continue;
      if (A.groupName != B.groupName) continue;
      if (A.address == B.address)     continue;
      // printf("compare %s <=> %s\n", A.name.c_str(), B.name.c_str());
      if (comparePeripheral(A,B)) {
        printf("  %s == %s, rename %s to %s\n", A.name.c_str(), B.name.c_str(), B.name.c_str(), A.name.c_str());
        B.baseName     = A.name;
        for (auto & p: peripherals) {
          if (p.baseName == B.name) {
            printf ("  + rename derived from %s to %s\n", p.baseName.c_str(), A.name.c_str());
            p.baseName     = A.name;
            p.registers.clear();
            p.struct_len = 0ul;
          }
        }
        B.registers.clear();
        B.struct_len = 0ul;
        compareDevice (depth + 1);
      }
    }
  }
}
bool DeviceTree::comparePeripheral(const PeripheralPart & A, const PeripheralPart & B) {
  const size_t lra = A.registers.size();
  const size_t lrb = B.registers.size();
  if (lra != lrb) {
    // printf("registers.size : %zd != %zd\n", lra, lrb);
    return false;
  }
  const size_t lr = lra < lrb ? lra : lrb;
  for (unsigned n=0; n<lr; n++) {
    if (! compareRegisters (A.registers[n], B.registers[n])) return false;
  }
  return true;
}
bool DeviceTree::compareRegisters(const RegisterPart & A, const RegisterPart & B) {
  if (A.address  != B.address) return false;
  if (A.baseName != B.baseName)return false;
//if (A.comment  != B.comment) return false; // komentář není podstatný
  if (A.access   != B.access)  return false;
  if (A.width    != B.width)   return false;
  if (A.size     != B.size)    return false;
  const size_t lra = A.fields.size();
  const size_t lrb = B.fields.size();
  if (lra != lrb) {
    // printf("fields.size : %zd != %zd\n", lra, lrb);
    return false;
  }
  const size_t lr = lra < lrb ? lra : lrb;
  for (unsigned n=0; n<lr; n++) {
    if (! compareFields (A.fields[n], B.fields[n])) return false;
  }
  return true;
}
bool DeviceTree::compareFields(const FieldPart & A, const FieldPart & B) {
  if (A.address  != B.address) return false;
  if (A.name     != B.name)    return false;
//if (A.comment  != B.comment) return false;
  if (A.access   != B.access)  return false;
  if (A.width    != B.width)   return false;
  if (A.size     != B.size)    return false;
  return true;
}

