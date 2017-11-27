#ifndef XBELTREE_H
#define XBELTREE_H

#include <QDomDocument>
#include <QHash>
#include <QIcon>
#include <QTreeWidget>

class DeviceTree;

class SvdTree : public QTreeWidget {
    Q_OBJECT

  public:
    SvdTree (QWidget *parent = 0);
    ~SvdTree();

    bool read (QIODevice *device);
    bool write (QIODevice *device);

  private slots:
    //void updateDomElement (QTreeWidgetItem *item, int column);

  private:
    void parseFolderElement (const QDomElement &element,
                             QTreeWidgetItem *parentItem = 0);
    QTreeWidgetItem *createItem (const QDomElement &element,
                                 QTreeWidgetItem *parentItem = 0);

    QDomDocument domDocument;
    QHash<QTreeWidgetItem *, QDomElement> domElementForItem;
    QIcon folderIcon;
    QIcon bookmarkIcon;
    DeviceTree * processor;
};

#endif
