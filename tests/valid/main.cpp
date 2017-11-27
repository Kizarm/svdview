#include <dirent.h>
#include <string.h>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

/** Validace souborů v adresáři, rekurzívně.
 * 
 * Těch validních na první pokus zase tak moc není.
 * 
 * */

using namespace std;
// nechci ty ruzne vypisy gcc ...
extern int RunCmd (const char * cmd);
/*
static int RunCmd (const char * cmd) {
  return system(cmd);
}
*/
#define SRED   "\x1B[31;1m"
#define SGREEN "\x1B[32;1m"
#define SBLUE  "\x1B[34;1m"
#define SDEFC  "\x1B[0m"

static vector<string> ValidFiles;
static const char * tname = "size";
static const char * hname = "test.h";

int FileValid (const char * file) {
  const int mlen = 4096;
  char cmd [mlen];
  remove (hname);
  remove (tname);
  printf("TEST \"%s\"", file);
  snprintf(cmd, mlen, "../../svdview %s %s", file, hname);
  //printf("make header => \"%s\"\n", cmd);
  int result = RunCmd(cmd);
  if (result) return 1;
  snprintf(cmd, mlen, "gcc -Os size.c -o %s", tname);
  //printf("compile     => \"%s\"\n", cmd);
  result = RunCmd(cmd);
  if (result) return 2;
  snprintf(cmd, mlen, "./%s", tname);
  //printf("run test    => \"%s\"\n", cmd);
  result = RunCmd(cmd);
  if (result) return 3;
  printf(SGREEN"\nFILE \"%s\" OK.\n"SDEFC, file);
  string name(file);
  ValidFiles.push_back(name);
  return 0;
}

int DirValid (const char * file) {
  int      len;
  DIR    * adr;
  struct dirent * info;

  adr = opendir (file);
  if (adr == NULL) {
    printf ("Nelze otevrit adresar \"%s\"\n", file);
    return (-1);
  }
  len = 0;
  for (;;) {
    info = readdir (adr);
    if (info == NULL) break;
    if (!strcmp (info->d_name, "." )) continue;
    if (!strcmp (info->d_name, "..")) continue;
    string sname = file;
    sname += '/';
    sname += info->d_name;
    if (info->d_type == DT_DIR) {
      DirValid(sname.c_str());
    } else {
      if (sname.rfind(".svd") != string::npos) {
        int valid = FileValid (sname.c_str());
        if (valid) {
          printf (SRED" ERROR - %d\n"SDEFC, valid);
        }
      }
    }
  }
  closedir (adr);
  return len;
}

int main (int argc, char *argv[]) {
  DirValid("../../data");
  
  FILE * out = fopen ("valids.txt", "w");
  for (unsigned i=0; i<ValidFiles.size(); i++) {
    string & s = ValidFiles.at(i);
    fprintf(out, "%s\n", s.c_str());
  }
  fclose(out);
  remove (hname);
  remove (tname);
  return 0;
}

