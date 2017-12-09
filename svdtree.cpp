#include <QtGui>
#include <QMessageBox>
#include <QHeaderView>

#include "svdtree.h"
#include "devicetree.h"

SvdTree::SvdTree (QWidget *parent)
  : QTreeWidget (parent) {
  QStringList labels;
  labels << tr ("Property") << tr ("Description");
#if QT_MAJOR > 4
  header()->setSectionResizeMode (QHeaderView::Interactive);
#else
  header()->setResizeMode (QHeaderView::Interactive);
#endif //QT_MAJOR
  setHeaderLabels (labels);

  folderIcon.addPixmap (style()->standardPixmap (QStyle::SP_DirClosedIcon),
                        QIcon::Normal, QIcon::Off);
  folderIcon.addPixmap (style()->standardPixmap (QStyle::SP_DirOpenIcon),
                        QIcon::Normal, QIcon::On);
  bookmarkIcon.addPixmap (style()->standardPixmap (QStyle::SP_FileIcon));
  
  processor = new DeviceTree();
}
SvdTree::~SvdTree() {
  delete processor;
}

bool SvdTree::read (QIODevice *device) {
  QString errorStr;
  int errorLine;
  int errorColumn;

  if (!domDocument.setContent (device, true, &errorStr, &errorLine,
                               &errorColumn)) {
    QMessageBox::information (window(), tr ("SVD viewer"),
                              tr ("Parse error at line %1, column %2:\n%3")
                              .arg (errorLine)
                              .arg (errorColumn)
                              .arg (errorStr));
    return false;
  }

  QDomElement root = domDocument.documentElement();
  if (root.tagName() != "device") {
    QMessageBox::information (window(), tr ("SVD viewer"),
                              tr ("The file is not an SVD file."));
    return false;
  } else if (root.hasAttribute ("schemaVersion")
        &&  !root.attribute    ("schemaVersion").contains("1.")) {
    QMessageBox::information (window(), tr ("SVD viewer"),
                              tr ("The file is not an SVD version 1.x "
                                  "file."));
    return false;
  }

  clear();
  const char * name = "peripherals";
  QDomElement child = root;
  processor->parse (child);
  
  parseFolderElement (child);
  return true;
}

bool SvdTree::write (QIODevice *device) {

  QTextStream out (device);
  processor->write(device);
  
  return true;
}

void SvdTree::parseFolderElement (const QDomElement &element,
                                   QTreeWidgetItem *parentItem) {
  QTreeWidgetItem *item = createItem (element, parentItem);

  QDomElement child = element.firstChildElement();
  QString     title = element.tagName();
  QString     deriv = element.attribute("derivedFrom");
  QString     name  = element.firstChildElement("name").text();
  if (!deriv.isEmpty()) {
    name += " (" + deriv + ")";
  }
  if (title.isEmpty()) title = "???";

  item->setText (0, title);
  if (child.isNull()) {
    item->setIcon (0, bookmarkIcon);
    name = element.text();
    QBrush b(QColor(255,255,196));
    item->setBackground(1,b);
    item->setText (1, name);
  } else {
    item->setIcon (0, folderIcon);
    if (!name.isEmpty())
      item->setText (1, name);
  }

  setItemExpanded (item, false);

  while (!child.isNull()) {
    parseFolderElement (child, item);
    child = child.nextSiblingElement();
  }
}

QTreeWidgetItem *SvdTree::createItem (const QDomElement &element,
                                       QTreeWidgetItem *parentItem) {
  QTreeWidgetItem *item;
  if (parentItem) {
    item = new QTreeWidgetItem (parentItem);
  } else {
    item = new QTreeWidgetItem (this);
  }
  domElementForItem.insert (item, element);
  return item;
}
