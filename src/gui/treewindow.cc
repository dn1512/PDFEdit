/** @file
 TreeWindow - class providing treeview of PDF objects
 @author Martin Petricek
*/
#include <utils/debug.h>
#include "treewindow.h"
#include "settings.h"
#include "util.h"
#include <iostream>
#include <qlayout.h>
#include <qlistview.h>
#include "treedata.h"
#include "pdfutil.h"
#include "treeitempdf.h"
#include "treeitem.h"

namespace gui {

using namespace std;
using namespace util;

/** constructor of TreeWindow, creates window and fills it with elements, parameters are ignored
 @param parent Parent widget
 @param name Name of this widget (not used, just passed to QWidget)
 */
TreeWindow::TreeWindow(QWidget *parent/*=0*/,const char *name/*=0*/):QWidget(parent,name) {
 QBoxLayout *l=new QVBoxLayout(this);
 tree=new QListView(this);
 tree->setSorting(-1);
 selected=root=NULL;
 QObject::connect(tree, SIGNAL(selectionChanged(QListViewItem *)), this, SLOT(treeSelectionChanged(QListViewItem *)));
 l->addWidget(tree);
 tree->addColumn(tr("Object"));
 tree->addColumn(tr("Type"));
 tree->addColumn(tr("Data"));
 tree->setSelectionMode(QListView::Single/*Extended*/);
 tree->setColumnWidthMode(0,QListView::Maximum);
 tree->show();
 data=new TreeData(this,tree);
 QObject::connect(tree,SIGNAL(mouseButtonClicked(int,QListViewItem*,const QPoint &,int)),this,SLOT(mouseClicked(int,QListViewItem*,const QPoint &,int)));
 QObject::connect(tree,SIGNAL(doubleClicked(QListViewItem*,const QPoint &,int)),this,SLOT(mouseDoubleClicked(QListViewItem*,const QPoint &,int)));
}

/** Reload part of tree that have given item as root (including that item)
 Reloading will stop at reference targets
 @param item root of subtree to reload
 */
void TreeWindow::reloadFrom(TreeItemAbstract *item) {
 item->reload();
}

/** Slot called when someone click with mouse button anywhere in the tree
@param button Which button(s) are clicked (1=left, 2=right, 4=middle)
@param item Which item is clicked upon (NULL if clicked outside item)
@param coord Coordinates of mouseclick
@param column Clicked in which item's column? (if clicked on item)
*/
void TreeWindow::mouseClicked(int button,QListViewItem* item,const QPoint &coord,int column) {
// guiPrintDbg(debug::DBG_DBG,"Clicked in tree: " << button);
 emit treeClicked(button,item);
}

/** Slot called when someone doubleclick with left mouse button anywhere in the tree
@param item Which item is clicked upon (NULL if clicked outside item)
@param coord Coordinates of mouseclick
@param column Clicked in which item's column? (if clicked on item)
*/
void TreeWindow::mouseDoubleClicked(QListViewItem* item,const QPoint &coord,int column) {
// guiPrintDbg(debug::DBG_DBG,"DoubleClicked in tree:");
 emit treeClicked(8,item);
}

/** Re-read tree settings from global settings */
void TreeWindow::updateTreeSettings() {
 data->update();
 if (data->isDirty()) {
//  guiPrintDbg(debug::DBG_DBG,"update tree settings: is dirty");
  data->resetDirty();
  update();//Update treeview itself
 }
}

/** reinitialize tree after some major change */
void TreeWindow::reinit() {
 root->reload();
}

/** Paint event handler -> if settings have been changed, reload tree */
void TreeWindow::paintEvent(QPaintEvent *e) {
 if (data->needReload()) {
  guiPrintDbg(debug::DBG_DBG,"update tree settings: need reload");
  reinit(); //update object if necessary
  data->resetReload();
 }
 //Pass along
 QWidget::paintEvent(e);
}

/** Called when any settings are updated (in script, option editor, etc ...) */
void TreeWindow::settingUpdate(QString key) {
 //TODO: only once per bunch of tree/show... signals ... setting blocks
 guiPrintDbg(debug::DBG_DBG,"Settings observer: " << key);
 if (key.startsWith("tree/show")) { //Updated settings of what to show and what not
  updateTreeSettings();
 }
}

/** Called upon changing selection in the tree window
 @param item The item that was selected
 */
void TreeWindow::treeSelectionChanged(QListViewItem *item) {
// guiPrintDbg(debug::DBG_DBG,"Selected an item: " << item->text(0));
 TreeItem* it=dynamic_cast<TreeItem*>(item);
 selected=dynamic_cast<TreeItemAbstract*>(item);
 emit itemSelected();
 if (!it) { //Not holding IProperty
  //TODO: TreeItemAbstract & add "return QSObject" to TreeItemAbstract
  guiPrintDbg(debug::DBG_WARN,"Not a TreeItem: " << item->text(0));
  //todo: handle this type properly
  return;
 }
 //holding IProperty
// guiPrintDbg(debug::DBG_DBG,"Is a TreeItem: " << item->text(0));
 //We have a TreeItem -> emit signal with selected object
 emit objectSelected(item->text(0),it->getObject());
}

/** 
 Return QSCObject from currently selected item
 Caller is responsible for freeing object
 @return QSCObject from current item
*/
QSCObject* TreeWindow::getSelected() {
 if (!selected) return NULL; //nothing selected
 return selected->getQSObject();
}

/** 
 Return pointer to currently selected tree item
 @return current item
*/
TreeItemAbstract* TreeWindow::getSelectedItem() {
 if (!selected) return NULL; //nothing selected
 return selected;
}

/** Clears all items from TreeWindow */
void TreeWindow::clear() {
 QListViewItem *li;
 while ((li=tree->firstChild())) {
  delete li;
 }
 data->clear();
 root=NULL;
}

/** Init contents of treeview from given PDF document
 @param pdfDoc Document used to initialize treeview
 @param fileName Name of PDF document (will be shown in treeview as name of root element)
 */
void TreeWindow::init(CPdf *pdfDoc,const QString &fileName) {
 guiPrintDbg(debug::DBG_DBG,"Loading PDF into tree");
 assert(pdfDoc);
 clear();
 rootName=fileName;
 setUpdatesEnabled( FALSE );
 root=new TreeItemPdf(data,pdfDoc,tree,fileName); 
 root->setOpen(TRUE);
 setUpdatesEnabled( TRUE );
}

/** Init contents of treeview from given IProperty (dictionary, etc ...)
 @param doc IProperty used to initialize treeview
 */
void TreeWindow::init(boost::shared_ptr<IProperty> doc) {
 guiPrintDbg(debug::DBG_DBG,"Loading Iproperty into tree");
 clear();
 if (doc.get()) {
  setUpdatesEnabled( FALSE );
  root=TreeItem::create(data,tree,doc); 
  root->setOpen(TRUE);
  setUpdatesEnabled( TRUE );
 }
}

/** Resets th tree to be empty and show nothing */
void TreeWindow::uninit() {
 clear();
}

/** default destructor */
TreeWindow::~TreeWindow() {
 delete data;
 delete tree;
}

} // namespace gui
