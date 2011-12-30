// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#include "tile_editor_main_window.h"

#include "nel/misc/path.h"

#include <QtGui/QMessageBox>
#include <QtGui/QApplication>
#include <QtGui/QMenu>
#include <QFileDialog>
#include <QInputDialog>

#include "../core/icore.h"
#include "../core/core_constants.h"
#include "../core/menu_manager.h"

#include "tile_model.h"
#include "tile_item.h"
#include "tile_item_delegate.h"

TileEditorMainWindow::TileEditorMainWindow(QWidget *parent)
	: QMainWindow(parent),
	m_ui(new Ui::TileEditorMainWindow)
{
	m_ui->setupUi(this);
	m_undoStack = new QUndoStack(this);

	// Retrieve the menu manager
	Core::ICore *core = Core::ICore::instance();
	Core::MenuManager *menuManager = core->menuManager();
	
	QMenu *m_tileEditorMenu;
	// Create tile rotation drop down toolbar menu.
	m_rotationMenu = new QMenu(tr("Rotate Tile"), m_ui->toolBar);
	m_rotationMenu->setIcon(QIcon(":/tileRotation/images/rotation0.png"));
	QList<QAction*> rotateActions;
	rotateActions.push_back(m_ui->actionRotateTile0);
	rotateActions.push_back(m_ui->actionRotateTile90);
	rotateActions.push_back(m_ui->actionRotateTile180);
	rotateActions.push_back(m_ui->actionRotateTile270);
	m_rotationMenu->addActions(rotateActions);
	m_ui->toolBar->addAction(m_rotationMenu->menuAction());

	// Create the tile zoom menu.
	m_zoomMenu = new QMenu(tr("Zoom"), m_ui->toolBar);
	QList<QAction*> zoomActions;

	m_tileEditorMenu = new QMenu(tr("Tile Editor"), core->menuManager()->menuBar());
	m_tileDisplayMenu = new QMenu(tr("Tile Display"), m_ui->toolBar);
	QList<QAction*> displayActions;
	displayActions.push_back(m_ui->actionTileDisplayFilename);
	displayActions.push_back(m_ui->actionTileDisplayIndex);
	m_ui->actionTileDisplayIndex->setChecked(true);
	m_tileDisplayMenu->addActions(displayActions);
	m_tileEditorMenu->addMenu(m_tileDisplayMenu);
	core->menuManager()->menuBar()->addMenu(m_tileEditorMenu);

	// Set up the list views.
	QStringList headers;
	headers << "Tile Set";
	m_model = new TileModel(headers, this);
	m_tileItemDelegate = new TileItemDelegate();

	// Set up the tile set list view.
	m_ui->tileSetLV->setModel(m_model);
	//m_ui->tileSetLV->setRootIndex(m_model->index(0,0));
	connect(m_ui->tileSetAddTB, SIGNAL(clicked()), this, SLOT(onTileSetAdd()));
	connect(m_ui->tileSetLV->selectionModel(),
             SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
             this, SLOT(changeActiveTileSet(const QModelIndex &, const QModelIndex &)));

	// 128x128 List View
	m_ui->listView128->setItemDelegate(m_tileItemDelegate);
	m_ui->listView128->setModel(m_model);
	m_ui->listView128->addAction(m_ui->actionAddTile);
	m_ui->listView128->addAction(m_ui->actionDeleteTile);
	m_ui->listView128->addAction(m_ui->actionReplaceImage);
	m_ui->listView128->addAction(m_ui->actionDeleteImage);

	// 256x256 List View
	m_ui->listView256->setItemDelegate(m_tileItemDelegate);
	m_ui->listView256->setModel(m_model);
	m_ui->listView256->addAction(m_ui->actionAddTile);
	m_ui->listView256->addAction(m_ui->actionDeleteTile);
	m_ui->listView256->addAction(m_ui->actionReplaceImage);
	m_ui->listView256->addAction(m_ui->actionDeleteImage);

	// Transition List View
	m_ui->listViewTransition->setItemDelegate(m_tileItemDelegate);
	m_ui->listViewTransition->setModel(m_model);
	m_ui->listViewTransition->addAction(m_ui->actionReplaceImage);
	m_ui->listViewTransition->addAction(m_ui->actionDeleteImage);

	// Displacement List View
	m_ui->listViewDisplacement->setItemDelegate(m_tileItemDelegate);
	m_ui->listViewDisplacement->setModel(m_model);
	m_ui->listViewDisplacement->addAction(m_ui->actionReplaceImage);
	m_ui->listViewDisplacement->addAction(m_ui->actionDeleteImage);

	
	// Connect context menu actions up.
	connect(m_ui->actionAddTile, SIGNAL(triggered(bool)), this, SLOT(onActionAddTile(bool)));
	connect(m_ui->actionDeleteTile, SIGNAL(triggered(bool)), this, SLOT(onActionDeleteTile(bool)));
	connect(m_ui->actionReplaceImage, SIGNAL(triggered(bool)), this, SLOT(onActionReplaceImage(bool)));
	connect(m_ui->actionDeleteImage, SIGNAL(triggered(bool)), this, SLOT(onActioneleteImage(bool)));
}

TileEditorMainWindow::~TileEditorMainWindow()
{
	delete m_ui;
	delete m_undoStack;
	delete m_rotationMenu;
	delete m_zoomMenu;
}

void TileEditorMainWindow::onActionAddTile(bool triggered)
{
	onActionAddTile(m_ui->tileViewTabWidget->currentIndex());
}

void TileEditorMainWindow::onActionDeleteTile(bool triggered)
{
}

void TileEditorMainWindow::onActionReplaceImage(bool triggered)
{
}

void TileEditorMainWindow::onActionDeleteImage(bool triggered)
{
}

void TileEditorMainWindow::onTileSetAdd()
{
	bool ok;
    QString text = QInputDialog::getText(this, tr("Add Tile Set"), tr("Enter Tile Set name:"), QLineEdit::Normal, "", &ok);
    if (ok && !text.isEmpty())
	{
		//if (ui.tileSetListWidget->findItems(text, Qt::MatchExactly).count() > 0)
		//{
		//	QMessageBox::information( this, tr("Error Adding Tile Set"), tr("This name already exists") );
		//}
		//else
		//{

		//QModelIndex index = m_ui->tileSetLV->selectionModel()->currentIndex();
		TileModel *model = static_cast<TileModel*>(m_ui->tileSetLV->model());

		//if(index.isValid())
		//{
		//	if(!model->insertRow(index.row()+1, index.parent()))
		//		return;

		//	//updateActions()

		//	for(int column=0; column<model->columnCount(index.parent()); column++)
		//	{
		//		QModelIndex child = model->index(index.row()+1, column, index.parent());
		//		model->setData(child, QVariant(text), Qt::EditRole);
		//	}
		//}
		//else
		//{
		
		
		// Create and append the new tile set to the model.
		TileSetNode *tileSet = model->createTileSetNode(text);
		
		// Retrieve how many rows there currently are and set the current index using that.
		m_ui->tileSetLV->reset();
		uint32 rows = model->rowCount();
		m_ui->tileSetLV->setCurrentIndex(model->index(rows-1, 0));
	}
}

void TileEditorMainWindow::onActionAddTile(int tabId)
{
	QFileDialog::Options options;
	QString selectedFilter;
	QStringList fileNames = QFileDialog::getOpenFileNames(this, "Choose Tile Texture", "." , "Images (*.png);;All Files (*.*)", &selectedFilter, options);
}

void TileEditorMainWindow::changeActiveTileSet(const QModelIndex &newIndex, const QModelIndex &oldIndex)
{
	TileModel *model = static_cast<TileModel*>(m_ui->tileSetLV->model());

	QModelIndex tile128Idx = model->index(0, 0, newIndex);
	QModelIndex tile256Idx = model->index(1, 0, newIndex);
	QModelIndex tileTransIdx = model->index(2, 0, newIndex);
	QModelIndex tileDispIdx = model->index(3, 0, newIndex);

	m_ui->listView128->setRootIndex(tile128Idx);
	m_ui->listView128->setCurrentIndex(m_ui->listView128->model()->index(0, 0, m_ui->listView128->rootIndex()));
	m_ui->listView256->setRootIndex(tile256Idx);
	m_ui->listView256->setCurrentIndex(m_ui->listView256->model()->index(0, 0, m_ui->listView256->rootIndex()));
	m_ui->listViewTransition->setRootIndex(tileTransIdx);
	m_ui->listViewTransition->setCurrentIndex(m_ui->listViewTransition->model()->index(0, 0, m_ui->listViewTransition->rootIndex()));
	m_ui->listViewDisplacement->setRootIndex(tileDispIdx);
	m_ui->listViewDisplacement->setCurrentIndex(m_ui->listViewDisplacement->model()->index(0, 0, m_ui->listViewDisplacement->rootIndex()));

	//nlinfo("number of rows in displacement: %d", tileDispIdx.model()->rowCount(tileDispIdx));

	//m_ui->listView128->reset();
	//m_ui->listView256->reset();
	//m_ui->listViewTransition->reset();
	//m_ui->listViewDisplacement->reset();
}
