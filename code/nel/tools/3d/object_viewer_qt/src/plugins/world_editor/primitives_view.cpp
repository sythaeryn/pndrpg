// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2011  Dzmitry Kamiahin <dnk-88@tut.by>
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

// Project includes
#include "primitives_view.h"
#include "primitive_item.h"
#include "primitives_model.h"
#include "world_editor_actions.h"

#include "../core/core_constants.h"
#include "../landscape_editor/landscape_editor_constants.h"
#include "../landscape_editor/builder_zone_base.h"

// NeL includes
#include <nel/ligo/primitive.h>
#include <nel/ligo/ligo_config.h>
#include <nel/ligo/primitive_class.h>

// Qt includes
#include <QContextMenuEvent>
#include <QtGui/QMenu>
#include <QtGui/QFileDialog>

namespace WorldEditor
{

PrimitivesView::PrimitivesView(QWidget *parent)
	: QTreeView(parent),
	  m_undoStack(0),
	  m_worldEditorScene(0),
	  m_zoneBuilder(0),
	  m_primitivesTreeModel(0)
{
	setContextMenuPolicy(Qt::DefaultContextMenu);

	m_unloadAction = new QAction("Unload", this);
	m_unloadAction->setEnabled(false);

	m_saveAction = new QAction("Save", this);
	m_saveAction->setEnabled(false);
	m_saveAction->setIcon(QIcon(Core::Constants::ICON_SAVE));

	m_saveAsAction = new QAction("Save As...", this);
	m_saveAsAction->setIcon(QIcon(Core::Constants::ICON_SAVE_AS));
	m_saveAsAction->setEnabled(false);

	m_loadLandAction = new QAction("Load landscape file", this);
	m_loadLandAction->setIcon(QIcon(LandscapeEditor::Constants::ICON_ZONE_ITEM));

	m_loadPrimitiveAction = new QAction("Load primitive file", this);
	m_loadPrimitiveAction->setIcon(QIcon("./old_ico/root.ico"));

	m_newPrimitiveAction = new QAction("New primitive", this);

	m_deleteAction = new QAction("Delete", this);
	m_deleteAction->setEnabled(false);

	m_selectChildrenAction = new QAction("Select children", this);

	m_helpAction = new QAction("Help", this);
	m_helpAction->setEnabled(false);

	m_showAction = new QAction("Show", this);
	m_showAction->setEnabled(false);

	m_hideAction = new QAction("Hide", this);
	m_hideAction->setEnabled(false);

	connect(m_loadLandAction, SIGNAL(triggered()), this, SLOT(loadLandscape()));
	connect(m_loadPrimitiveAction, SIGNAL(triggered()), this, SLOT(loadRootPrimitive()));
	connect(m_newPrimitiveAction, SIGNAL(triggered()), this, SLOT(createRootPrimitive()));
	connect(m_selectChildrenAction, SIGNAL(triggered()), this, SLOT(selectChildren()));
	connect(m_deleteAction, SIGNAL(triggered()), this, SLOT(deletePrimitives()));

#ifdef Q_OS_DARWIN
	setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
#endif
}

PrimitivesView::~PrimitivesView()
{
}

void PrimitivesView::setUndoStack(QUndoStack *undoStack)
{
	m_undoStack = undoStack;
}

void PrimitivesView::setZoneBuilder(LandscapeEditor::ZoneBuilderBase *zoneBuilder)
{
	m_zoneBuilder = zoneBuilder;
}

void PrimitivesView::setWorldScene(WorldEditorScene *worldEditorScene)
{
	m_worldEditorScene = worldEditorScene;
}

void PrimitivesView::setModel(PrimitivesTreeModel *model)
{
	QTreeView::setModel(model);
	m_primitivesTreeModel = model;
}

void PrimitivesView::loadRootPrimitive()
{
	nlassert(m_undoStack);
	nlassert(m_primitivesTreeModel);

	QStringList fileNames = QFileDialog::getOpenFileNames(this,
							tr("Open NeL Ligo primitive file"), m_lastDir,
							tr("All NeL Ligo primitive files (*.primitive)"));

	setCursor(Qt::WaitCursor);
	if (!fileNames.isEmpty())
	{
		if (fileNames.count() > 1)
			m_undoStack->beginMacro("Load primitive files");

		Q_FOREACH(QString fileName, fileNames)
		{
			m_lastDir = QFileInfo(fileName).absolutePath();
			m_undoStack->push(new LoadRootPrimitiveCommand(fileName, m_worldEditorScene, m_primitivesTreeModel));
		}

		if (fileNames.count() > 1)
			m_undoStack->endMacro();
	}
	setCursor(Qt::ArrowCursor);
}

void PrimitivesView::loadLandscape()
{
	nlassert(m_undoStack);
	nlassert(m_zoneBuilder);
	nlassert(m_primitivesTreeModel);

	QStringList fileNames = QFileDialog::getOpenFileNames(this,
							tr("Open NeL Ligo land file"), m_lastDir,
							tr("All NeL Ligo land files (*.land)"));

	setCursor(Qt::WaitCursor);
	if (!fileNames.isEmpty())
	{
		if (fileNames.count() > 1)
			m_undoStack->beginMacro("Load land files");

		Q_FOREACH(QString fileName, fileNames)
		{
			m_lastDir = QFileInfo(fileName).absolutePath();
			m_undoStack->push(new LoadLandscapeCommand(fileName, m_primitivesTreeModel, m_zoneBuilder));
		}

		if (fileNames.count() > 1)
			m_undoStack->endMacro();
	}
	setCursor(Qt::ArrowCursor);
}

void PrimitivesView::createRootPrimitive()
{
	nlassert(m_undoStack);
	nlassert(m_primitivesTreeModel);

	m_undoStack->push(new CreateRootPrimitiveCommand("NewPrimitive", m_primitivesTreeModel));
}

void PrimitivesView::selectChildren()
{
	QModelIndexList indexList = selectionModel()->selectedRows();
	QModelIndex parentIndex = indexList.first();

	selectionModel()->clearSelection();
	selectChildren(parentIndex);
}

void PrimitivesView::deletePrimitives()
{
	nlassert(m_undoStack);
	nlassert(m_primitivesTreeModel);

	QModelIndexList indexList = selectionModel()->selectedRows();
}

void PrimitivesView::addNewPrimitiveByClass(int value)
{
	nlassert(m_undoStack);
	nlassert(m_primitivesTreeModel);

	QModelIndexList indexList = selectionModel()->selectedRows();

	PrimitiveNode *node = static_cast<PrimitiveNode *>(indexList.first().internalPointer());

	// Get class name
	QString className = node->primitiveClass()->DynamicChildren[value].ClassName.c_str();

	m_undoStack->push(new AddPrimitiveByClassCommand(className, m_primitivesTreeModel->pathFromIndex(indexList.first()),
					  m_primitivesTreeModel));
}

void PrimitivesView::generatePrimitives(int value)
{
}

void PrimitivesView::openItem(int value)
{
}

void PrimitivesView::contextMenuEvent(QContextMenuEvent *event)
{
	QWidget::contextMenuEvent(event);
	QModelIndexList indexList = selectionModel()->selectedRows();
	if (indexList.size() == 0)
		return;

	QMenu *popurMenu = new QMenu(this);

	if (indexList.size() == 1)
	{
		Node *node = static_cast<Node *>(indexList.first().internalPointer());
		switch (node->type())
		{
		case Node::WorldEditNodeType:
			fillMenu_WorldEdit(popurMenu);
			break;
		case Node::RootPrimitiveNodeType:
			fillMenu_RootPrimitive(popurMenu, indexList.first());
			break;
		case Node::LandscapeNodeType:
			fillMenu_Landscape(popurMenu);
			break;
		case Node::PrimitiveNodeType:
			fillMenu_Primitive(popurMenu, indexList.first());
			break;
		};
	}

	popurMenu->exec(event->globalPos());
	delete popurMenu;
	event->accept();
}

void PrimitivesView::selectChildren(const QModelIndex &parent)
{
	const int rowCount = model()->rowCount(parent);

	for (int i = 0; i < rowCount; ++i)
	{
		QModelIndex childIndex = parent.child(i, 0);
		selectionModel()->select(childIndex, QItemSelectionModel::Select);
		selectChildren(childIndex);
	}
}

void PrimitivesView::fillMenu_WorldEdit(QMenu *menu)
{
	menu->addAction(m_unloadAction);
	menu->addAction(m_saveAction);
	menu->addAction(m_saveAsAction);
	menu->addSeparator();
	menu->addAction(m_loadLandAction);
	menu->addAction(m_loadPrimitiveAction);
	menu->addAction(m_newPrimitiveAction);
	menu->addSeparator();
	menu->addAction(m_helpAction);
}

void PrimitivesView::fillMenu_Landscape(QMenu *menu)
{
	menu->addAction(m_deleteAction);
	menu->addSeparator();
	menu->addAction(m_showAction);
	menu->addAction(m_hideAction);
}

void PrimitivesView::fillMenu_RootPrimitive(QMenu *menu, const QModelIndex &index)
{
	menu->addAction(m_saveAction);
	menu->addAction(m_saveAsAction);
	fillMenu_Primitive(menu, index);
}

void PrimitivesView::fillMenu_Primitive(QMenu *menu, const QModelIndex &index)
{
	menu->addAction(m_deleteAction);
	menu->addAction(m_selectChildrenAction);
	menu->addAction(m_helpAction);
	menu->addSeparator();
	menu->addAction(m_showAction);
	menu->addAction(m_hideAction);

	QSignalMapper *addSignalMapper = new QSignalMapper(menu);
	QSignalMapper *generateSignalMapper = new QSignalMapper(menu);
	QSignalMapper *openSignalMapper = new QSignalMapper(menu);
	connect(addSignalMapper, SIGNAL(mapped(int)), this, SLOT(addNewPrimitiveByClass(int)));
	connect(generateSignalMapper, SIGNAL(mapped(int)), this, SLOT(generatePrimitives(int)));
	//connect(openSignalMapper, SIGNAL(mapped(int)), this, SLOT(openItem(int)));

	PrimitiveNode *node = static_cast<PrimitiveNode *>(index.internalPointer());
	const NLLIGO::CPrimitiveClass *primClass = node->primitiveClass();

	// What class is it ?
	if (primClass && primClass->DynamicChildren.size())
	{
		menu->addSeparator();

		// For each child, add a create method
		for (size_t i = 0; i < primClass->DynamicChildren.size(); i++)
		{
			// Get class name
			QString className = primClass->DynamicChildren[i].ClassName.c_str();

			// Get icon
			QIcon icon(QString("./old_ico/%1.ico").arg(className));

			// Create and add action in popur menu
			QAction *action = menu->addAction(icon, QString("Add %1").arg(className));
			addSignalMapper->setMapping(action, i);
			connect(action, SIGNAL(triggered()), addSignalMapper, SLOT(map()));
		}
	}

	// What class is it ?
	if (primClass && primClass->GeneratedChildren.size())
	{
		menu->addSeparator();

		// For each child, add a create method
		for (size_t i = 0; i < primClass->GeneratedChildren.size(); i++)
		{
			// Get class name
			QString childName = primClass->GeneratedChildren[i].ClassName.c_str();

			// Create and add action in popur menu
			QAction *action = menu->addAction(QString("Generate %1").arg(childName));
			generateSignalMapper->setMapping(action, i);
			connect(generateSignalMapper, SIGNAL(triggered()), addSignalMapper, SLOT(map()));
		}
	}
	/*
			// What class is it ?
			if (primClass)
			{
				// Look for files
				std::vector<std::string> filenames;

				// Filenames
				buildFilenameVector (*Selection.front (),  filenames);

				// File names ?
				if (!filenames.empty ())
				{
						// Add separator
						popurMenu->addSeparator();

						// Found ?
						for (uint i = 0; i < filenames.size(); i++)
						{
							// Add a menu entry
							pMenu->AppendMenu (MF_STRING, ID_EDIT_OPEN_FILE_BEGIN+i, ("Open "+NLMISC::CFile::getFilename (filenames[i])).c_str ());
						}
				}
	*/
}

} /* namespace WorldEditor */