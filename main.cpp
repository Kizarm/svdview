#include <QApplication>
#include "mainwindow.h"
#include "cmdline.h"
#include <qstyle.h>

int main (int argc, char *argv[]) {
  int res = 0;
  if (argc > 1) {
    QStringList list;
    for (int i=0; i<argc; i++) {
      list.append(QString(argv[i]));
    }
    CmdLine cmd (list);
    res = cmd.run();
  } else {
    //QApplication::setStyle("plastique");
    QApplication app (argc, argv);
    MainWindow mainWin;
    mainWin.show();
    res =  app.exec();
  }

  return res;
}
