#ifndef __TREEITEMDICT_H__
#define __TREEITEMDICT_H__

#include <iproperty.h>
#include <qlistview.h>
#include "treeitem.h"
class QString;

namespace gui {

using namespace pdfobjects;

class TreeData;

class TreeItemDict : public TreeItem {
public:
 TreeItemDict(TreeData *_data,QListView *parent,boost::shared_ptr<IProperty> pdfObj,const QString name=QString::null,QListViewItem *after=NULL,const QString &nameId=NULL);
 TreeItemDict(TreeData *_data,QListViewItem *parent,boost::shared_ptr<IProperty> pdfObj,const QString name=QString::null,QListViewItem *after=NULL,const QString &nameId=NULL);
 virtual ~TreeItemDict();
 //From TreeItemAbstract interface
 virtual ChildType getChildType(const QString &name);
 virtual TreeItemAbstract* createChild(const QString &name,ChildType typ,QListViewItem *after=NULL);
 virtual QStringList getChildNames();
 QSCObject* getQSObject();
};

} // namespace gui

#endif
