#include "cmdline.h"
#include <QDebug>
#include <QStringList>
#include <QFile>

extern bool NO_UNIONS;
extern bool CLASICDEF;

CmdLine::CmdLine (QStringList & params) : 
  processor(), inputFileName(), outputFileName(), progname() {
  help = false;
  progname = params.at(0);
  for (int i=1; i<params.size(); i++) {
    const QString & param = params.at(i);
    if (param.at(0) == '-') {
      for (int j=0; j<param.size(); j++) {
        if (param.at(j) == 'n') NO_UNIONS = true;
        if (param.at(j) == 'd') CLASICDEF = true;
        if (param.at(j) == 'h') help      = true;
      }
    }
    if (param.endsWith(".svd")) {
      inputFileName = param;
    } else if (param.endsWith(".h")) {
      outputFileName = param;
    }
  }
  if (inputFileName.isEmpty()) {
    help = true;
    return;
  }
  if (outputFileName.isEmpty()) {
    outputFileName = inputFileName;
    int n = inputFileName.lastIndexOf(".svd");
    outputFileName.replace(n, 4, QString(".h"));
  }
}
int CmdLine::run() {
  if (help) {
    qDebug() << "Usage:";
    qDebug("  %s [-nd] input.svd [output.h]", progname.toLatin1().constData());
    return 0;
  }
  qDebug() << inputFileName << "->" << outputFileName;
  if (!Open()) return 1;
  if (!Save()) return 1;
  return 0;
}
bool CmdLine::Open() {
  QFile file (inputFileName);
  if (!file.open (QFile::ReadOnly | QFile::Text)) {
    qDebug() << "Nelze otevrit" << inputFileName;
    return false;
  }
  if (!read(&file)) {
    qDebug() << "Nelze nacist" << inputFileName;
    return false;
  }
  return true;
}
bool CmdLine::read (QIODevice* device) {
  QString errorStr;
  int errorLine;
  int errorColumn;

  if (!domDocument.setContent (device, true, &errorStr, &errorLine, &errorColumn)) {
    QString s = QString("Parse error at line %1, column %2:\n%3")
    .arg (errorLine)
    .arg (errorColumn)
    .arg (errorStr);
    qDebug() << "chyba" << s;
    return false;
  }

  QDomElement root = domDocument.documentElement();
  if (root.tagName() != "device") {
    qDebug () << "The file is not an SVD file.";
    return false;
  } else if (root.hasAttribute ("schemaVersion")
        &&  !root.attribute    ("schemaVersion").contains("1.")) {
    qDebug() << "The file is not an SVD version 1.x file.";
    return false;
  }

  const char * name = "peripherals";
  QDomElement child = root;
  processor.parse (child);
  
  return true;
}

bool CmdLine::Save() {
  QFile file (outputFileName);
  if (!file.open (QFile::WriteOnly | QFile::Text)) {
    qDebug() << "Nelze otevrit" << outputFileName;
    return false;
  }
  if (!write(&file)) {
    qDebug() << "Nelze zapsat do" << outputFileName;
    return false;
  }
  return true;
}
bool CmdLine::write (QIODevice* device) {
  QTextStream out (device);
  processor.write (device);

  return true;
}
