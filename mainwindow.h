#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class SvdTree;

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    MainWindow();

  public slots:
    void open();
    void saveAs();
    void about();
    
    void classic(bool);
    void defines(bool);

  private:
    void createActions();
    void createMenus();

    SvdTree *svdTree;

    QMenu *fileMenu;
    QMenu *helpMenu;
    QMenu *formatMenu;
    QAction *openAct;
    QAction *saveAsAct;
    QAction *exitAct;
    QAction *aboutAct;
    QAction *aboutQtAct;
    
    QAction *classicAct;
    QAction *definesAct;
};

#endif
