#include <QtGui>
#include <QStatusBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QAction>
#include <QMenu>
#include <QMenuBar>

#include "mainwindow.h"
#include "svdtree.h"

MainWindow::MainWindow() {
  svdTree = new SvdTree;
  setCentralWidget (svdTree);

  createActions();
  createMenus();

  statusBar()->showMessage (tr ("Ready"));

  setWindowTitle (tr ("SVD viewer"));
  setWindowIcon  (QIcon(":xml.png"));
  resize (480, 320);
}

void MainWindow::open() {
  QString fileName =
    QFileDialog::getOpenFileName (this, tr ("Open SVD File"),
                                  QDir::currentPath(),
                                  tr ("SVD Files (*.svd *.xml)"));
  if (fileName.isEmpty())
    return;

  QFile file (fileName);
  if (!file.open (QFile::ReadOnly | QFile::Text)) {
    QMessageBox::warning (this, tr ("SVD viewer"),
                          tr ("Cannot read file %1:\n%2.")
                          .arg (fileName)
                          .arg (file.errorString()));
    return;
  }

  if (svdTree->read (&file))
    statusBar()->showMessage (tr ("File loaded"), 2000);
}

void MainWindow::saveAs() {
  QString fileName =
    QFileDialog::getSaveFileName (this, tr ("Save header File"),
                                  QDir::currentPath(),
                                  tr ("C header Files (*.h)"));
  if (fileName.isEmpty())
    return;

  QFile file (fileName);
  if (!file.open (QFile::WriteOnly | QFile::Text)) {
    QMessageBox::warning (this, tr ("SVD viewer"),
                          tr ("Cannot write file %1:\n%2.")
                          .arg (fileName)
                          .arg (file.errorString()));
    return;
  }

  if (svdTree->write (&file))
    statusBar()->showMessage (tr ("File saved"), 2000);
}

void MainWindow::about() {
  QMessageBox::about (this, tr ("About SVD viewer"),
                      tr ("The <b>SVD viewer</b> example demonstrates how to "
                          "use Qt's DOM classes to read and write XML "
                          "documents."));
}

void MainWindow::createActions() {
  openAct = new QAction (tr ("&Open..."), this);
  openAct->setShortcuts (QKeySequence::Open);
  connect (openAct, SIGNAL (triggered()), this, SLOT (open()));

  saveAsAct = new QAction (tr ("&Save As..."), this);
  saveAsAct->setShortcuts (QKeySequence::SaveAs);
  connect (saveAsAct, SIGNAL (triggered()), this, SLOT (saveAs()));

  exitAct = new QAction (tr ("E&xit"), this);
  exitAct->setShortcuts (QKeySequence::Quit);
  connect (exitAct, SIGNAL (triggered()), this, SLOT (close()));

  aboutAct = new QAction (tr ("&About"), this);
  connect (aboutAct, SIGNAL (triggered()), this, SLOT (about()));

  aboutQtAct = new QAction (tr ("About &Qt"), this);
  connect (aboutQtAct, SIGNAL (triggered()), qApp, SLOT (aboutQt()));
  
  classicAct = new QAction (tr ("No dump unions"), this);
  classicAct->setCheckable(true);
  classicAct->setChecked(false);
  connect (classicAct, SIGNAL(toggled(bool)), this, SLOT(classic(bool)));
  definesAct = new QAction (tr ("Dump as defines"), this);
  definesAct->setCheckable(true);
  definesAct->setChecked(false);
  connect (definesAct, SIGNAL(toggled(bool)), this, SLOT(defines(bool)));
}

void MainWindow::createMenus() {
  fileMenu = menuBar()->addMenu (tr ("&File"));
  fileMenu->addAction (openAct);
  fileMenu->addAction (saveAsAct);
  fileMenu->addAction (exitAct);

  menuBar()->addSeparator();
  
  formatMenu = menuBar()->addMenu(tr ("For&mat"));
  formatMenu->addAction(classicAct);
  formatMenu->addAction(definesAct);
  
  menuBar()->addSeparator();

  helpMenu = menuBar()->addMenu (tr ("&Help"));
  helpMenu->addAction (aboutAct);
  helpMenu->addAction (aboutQtAct);
}

bool NO_UNIONS = false;
bool CLASICDEF = false;

void MainWindow::classic(bool b) {
  NO_UNIONS = b;
//qDebug("NO_UNIONS=%d", b);
}
void MainWindow::defines(bool b) {
  CLASICDEF = b;
//qDebug("CLASICDEF=%d", b);
}

