#ifndef CMDLINE_H
#define CMDLINE_H
#include <QDomDocument>
#include "devicetree.h"

class QStringList;

class CmdLine {
  public:
    CmdLine(QStringList & params);
    int run ();
  protected:
    bool Open ();
    bool Save ();
    bool read  (QIODevice *device);
    bool write (QIODevice *device);
  private:
    DeviceTree processor;
    QString    inputFileName;
    QString    outputFileName;
    QString    progname;
    QDomDocument domDocument;
    
    bool help;
};

#endif // CMDLINE_H
