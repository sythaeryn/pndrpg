// Object Viewer Qt - Georges Editor Plugin - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2011  Adrian Jaekel <aj at elane2k dot com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "georges_treeview_dialog.h"

// Qt includes
#include <QtGui/QWidget>
#include <QSettings>
#include <QFileDialog>

// NeL includes
#include <nel/misc/path.h>
#include <nel/misc/file.h>
#include <nel/misc/o_xml.h>
#include <nel/georges/form.h>

// Project includes
#include "georges.h"
#include "georgesform_model.h"
#include "georgesform_proxy_model.h"
#include "formitem.h"
//#include "formdelegate.h"

using namespace NLMISC;
using namespace NLGEORGES;

namespace Plugin 
{

	CGeorgesTreeViewDialog::CGeorgesTreeViewDialog(QWidget *parent /*= 0*/)
		: QDockWidget(parent)
	{
		m_georges = new CGeorges;

		loadedForm = "";
		m_modified = false;

		m_ui.setupUi(this);
		m_ui.treeView->header()->setResizeMode(QHeaderView::ResizeToContents);
		m_ui.treeViewTabWidget->setTabEnabled (2,false);

		m_ui.checkBoxParent->setStyleSheet("background-color: rgba(0,255,0,30)");
		m_ui.checkBoxDefaults->setStyleSheet("background-color: rgba(255,0,0,30)");
		m_form = 0;

		//FormDelegate *formdelegate = new FormDelegate(this);
		//_ui.treeView->setItemDelegateForColumn(1, formdelegate);


		connect(m_ui.treeView, SIGNAL(doubleClicked (QModelIndex)),
			this, SLOT(doubleClicked (QModelIndex)));
		connect(m_ui.checkBoxParent, SIGNAL(toggled(bool)),
			this, SLOT(filterRows()));
		connect(m_ui.checkBoxDefaults, SIGNAL(toggled(bool)),
			this, SLOT(filterRows()));
	}

	CGeorgesTreeViewDialog::~CGeorgesTreeViewDialog()
	{
		//delete _ui.treeView->itemDelegateForColumn(1);
		delete m_form;
		//QSettings settings("RyzomCore", "GeorgesQt");
		//settings.setValue("dirViewGeometry", saveGeometry());
	}

	void CGeorgesTreeViewDialog::setForm(const CForm *form) 
	{
		m_form = (UForm*)form;
	}

	CForm* CGeorgesTreeViewDialog::getFormByName(const QString formName) 
	{
		if(NLMISC::CPath::exists(formName.toStdString()))
		{
			return (CForm*)m_georges->loadForm(formName.toStdString());
		}
		//else
		//{
		//	CForm *form = 0;
		//	// Load the DFN
		//	std::string extStr = NLMISC::CFile::getExtension( formName.toStdString() );
		//	QString dfnName = QString("%1.dfn").arg(extStr.c_str());
		//	UFormDfn *formdfn;
		//	if (NLMISC::CPath::exists(dfnName.toStdString()))
		//	{
		//		formdfn = _georges->loadFormDfn (dfnName.toStdString());
		//		if (!formdfn)
		//		{
		//			nlwarning("Failed to load dfn: %s", dfnName.toStdString().c_str());
		//			return 0;
		//		}
		//	}
		//	else
		//	{
		//		nlwarning("Cannot find dfn: %s", dfnName.toStdString().c_str());
		//		return 0;
		//	}

		//	form = new CForm;

		//	// Build the root element
		//	((CFormElmStruct*)&form->getRootNode())->build((CFormDfn*)formdfn);

		//	uint i;
		//	for (i=0; i<CForm::HeldElementCount; i++)
		//	{
		//		((CFormElmStruct*)(((CForm*)form)->HeldElements[i]))->build ((CFormDfn*)formdfn);
		//	}
		//	return form;
		//}
		return 0;
	}

	void CGeorgesTreeViewDialog::loadFormIntoDialog(CForm *form) 
	{

		if(form)
			m_form = form;
		else
			return;

		UFormElm *root = 0;
		root = &m_form->getRootNode();

		QStringList parents;
		for (uint i = 0; i < m_form->getNumParent(); i++) 
		{
			UForm *u = m_form->getParentForm(i);
			parents << u->getFilename().c_str();
		}

		QString comments;
		comments = m_form->getComment().c_str();

		if (!comments.isEmpty()) 
		{
			m_ui.treeViewTabWidget->setTabEnabled (1,true);
			m_ui.commentEdit->setPlainText(comments);
		}

		QStringList strList;
		std::set<std::string> dependencies;
		m_form->getDependencies(dependencies);

		QMap< QString, QStringList> deps;
		Q_FOREACH(std::string str, dependencies) 
		{
			QString file = str.c_str();
			if (str == m_form->getFilename()) continue;
			deps[file.remove(0,file.indexOf(".")+1)] << str.c_str();
		}
		nlinfo("typ's %d",deps["typ"].count());
		nlinfo("dfn's %d",deps["dfn"].count());

		//nlwarning(strList.join(";").toStdString().c_str());
		if (root) 
		{
			loadedForm = m_form->getFilename().c_str();

			CGeorgesFormModel *model = new CGeorgesFormModel(root,deps,comments,parents);
			CGeorgesFormProxyModel *proxyModel = new CGeorgesFormProxyModel();
			proxyModel->setSourceModel(model);
			m_ui.treeView->setModel(proxyModel);
			m_ui.treeView->expandAll();
			// this is a debug output row
			m_ui.treeView->hideColumn(3);

			filterRows();

		//		//_ui.treeView->setRowHidden(0,QModelIndex(),true);
		//		connect(model, SIGNAL(dataChanged(const QModelIndex, const QModelIndex)),
		//			this, SLOT(modifiedFile()));

			setWindowTitle(loadedForm);
		//		//Modules::mainWin().getTabBar();			
		}
	}

	void CGeorgesTreeViewDialog::addParentForm(CForm *form) 
	{
		//((CForm*)_form)->insertParent(((CForm*)_form)->getParentCount(), form->getFilename().c_str(), form);
	}

	void CGeorgesTreeViewDialog::modifiedFile( ) 
	{
		/*if (!_modified) 
		{
			_modified = true;
			setWindowTitle(windowTitle()+"*");
			Modules::mainWin().setWindowTitle(Modules::mainWin().windowTitle()+"*");
			Q_EMIT modified(_modified);
		}*/
	}

	void CGeorgesTreeViewDialog::write( ) 
	{

		//COFile file;
		//std::string s = CPath::lookup(loadedForm.toStdString(), false);
		//if (file.open (s)) 
		//{
		//	try	
		//	{
		//		if (loadedForm.contains(".typ")) 
		//		{
		//			//nlassert (Type != NULL);

		//			//// Write the file
		//			//// Modified ?
		//			//if (IsModified ())
		//			//{
		//			//	Type->Header.MinorVersion++;
		//			//	flushValueChange ();
		//			//}
		//			//Type->write (xmlStream.getDocument (), theApp.Georges4CVS);
		//			//modify (NULL, NULL, false);
		//			//flushValueChange ();
		//			//UpdateAllViews (NULL);
		//			//return TRUE;
		//		}
		//		else if (loadedForm.contains(".dfn"))	
		//		{
		//			//nlassert (Dfn != NULL);

		//			//// Write the file
		//			//if (IsModified ())
		//			//{
		//			//	Dfn->Header.MinorVersion++;
		//			//	flushValueChange ();
		//			//}
		//			//Dfn->write (xmlStream.getDocument (), lpszPathName, theApp.Georges4CVS);
		//			//modify (NULL, NULL, false);
		//			//UpdateAllViews (NULL);
		//			//return TRUE;
		//		}
		//		else 
		//		{
		//			nlassert (_form != NULL);

		//			// Write the file
		//			/*if (IsModified ())
		//			{
		//			((CForm*)(UForm*)Form)->Header.MinorVersion++;
		//			}*/
		//			//((CForm*)(UForm*)Form)->write (xmlStream.getDocument (), lpszPathName, theApp.Georges4CVS);
		//			_form->write(file, false);
		//			setWindowTitle(windowTitle().remove("*"));
		//			_modified = false;
		//			//if (strcmp (xmlStream.getErrorString (), "") != 0)
		//			//{
		//			//	char message[512];
		//			//	smprintf (message, 512, "Error while saving file: %s", xmlStream.getErrorString ());
		//			//theApp.outputError (message);
		//			//}
		//			//modify (NULL, NULL, false);
		//			//flushValueChange ();
		//			//UpdateAllViews (NULL);

		//			// Get the left view
		//			//CView* pView = getLeftView ();
		//		}
		//	}
		//	catch (Exception &e)
		//	{
		//		nlerror("Error while loading file: %s", e.what());
		//	}
		//}
		//else
		//{ //if (!file.open())
		//	nlerror("Can't open the file %s for writing.", s.c_str());
		//}
	}

	void CGeorgesTreeViewDialog::doubleClicked ( const QModelIndex & index ) 
	{
		// TODO: this is messy :( perhaps this can be done better
		CGeorgesFormProxyModel * proxyModel = 
			dynamic_cast<CGeorgesFormProxyModel *>(m_ui.treeView->model());
		CGeorgesFormModel *model = 
			dynamic_cast<CGeorgesFormModel *>(proxyModel->sourceModel());
		QModelIndex sourceIndex = proxyModel->mapToSource(index);

		CFormItem *item = model->getItem(sourceIndex);

		if (item->parent() && item->parent()->data(0) == "parents")
		{
			Q_EMIT changeFile(CPath::lookup(item->data(0).toString().toStdString(),false).c_str());
		}

		//// col containing additional stuff like icons
		//if (index.column() == 2) 
		//{
		//	QModelIndex in2 = m->index(in.row(),in.column()-1,in.parent());
		//	CFormItem *item = m->getItem(in2);
		//	QString value = item->data(1).toString();

		//	QString path = CPath::lookup(value.toStdString(),false).c_str();

		//	if(value.contains(".tga") || value.contains(".png")) 
		//	{
		//		QString file = QFileDialog::getOpenFileName(
		//			this,
		//			"Select a new image",
		//			path,
		//			"Images (*.png *.tga)"
		//			);
		//		if (file.isNull())
		//			return;
		//		QFileInfo info = QFileInfo(file);

		//		// TODO?
		//		// right way would be another delegate but im too lazy :)
		//		// so for now i just call it directly
		//		m->setData(in2, info.fileName());
		//		return;
		//	}
		//	else 
		//	{
		//		if (path.contains(".shape") || path.contains(".ps"))
		//		{
		//			if (Modules::objViewInt()) 
		//			{
		//				Modules::objViewInt()->resetScene();
		//				//Modules::config().configRemapExtensions();
		//				Modules::objViewInt()->loadMesh(path.toStdString(),"");
		//			}
		//			return;
		//		}
		//	} 

		//	// open eg parent files
		//	if (!path.isEmpty())
		//		Q_EMIT changeFile(path);

		//}
	}

	void CGeorgesTreeViewDialog::closeEvent(QCloseEvent *event) 
	{
		Q_EMIT closing();
		deleteLater();
	}

	void CGeorgesTreeViewDialog::filterRows()
	{
		nlinfo("CGeorgesTreeViewDialog::filterRows");
		CGeorgesFormProxyModel * mp = dynamic_cast<CGeorgesFormProxyModel *>(m_ui.treeView->model());
		CGeorgesFormModel *m = dynamic_cast<CGeorgesFormModel *>(mp->sourceModel());
		if (m) {
			m->setShowParents(m_ui.checkBoxParent->isChecked());
			m->setShowDefaults(m_ui.checkBoxDefaults->isChecked());
		}

		//CGeorgesFormProxyModel * mp = dynamic_cast<CGeorgesFormProxyModel *>(_ui.treeView->model());
		//CGeorgesFormModel *m = dynamic_cast<CGeorgesFormModel *>(mp->sourceModel());

		//for (int i = 0; i < m->rowCount(); i++) 
		//{
		//	const QModelIndex in = m->index(i,0);
		//	if (m->getItem(in)->nodeFrom() == UFormElm::NodeParentForm) 
		//	{
		//		if (newState == Qt::Checked) 
		//		{
		//			_ui.treeView->setRowHidden(in.row(),in.parent(),false);
		//		}
		//		else
		//		{
		//			_ui.treeView->setRowHidden(in.row(),in.parent(),true);
		//		}
		//	} 
		//	else 
		//	{ // search childs // recursive?
		//		for (int j = 0; j < m->rowCount(in); j++) 
		//		{
		//			const QModelIndex in2 = m->index(j,0,in);
		//			if (m->getItem(in2)->nodeFrom() == UFormElm::NodeParentForm) 
		//			{
		//				if (newState == Qt::Checked) 
		//				{
		//					_ui.treeView->setRowHidden(in2.row(),in,false);
		//				}
		//				else 
		//				{
		//					_ui.treeView->setRowHidden(in2.row(),in,true);
		//				}
		//			}
		//		}
		//	} // end of search childs
		//}
	}

} /* namespace NLQT */
