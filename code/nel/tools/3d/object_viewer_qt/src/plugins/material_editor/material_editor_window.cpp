// Object Viewer Qt Material Editor plugin <http://dev.ryzom.com/projects/ryzom/>
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

#include "material_editor_window.h"
#include "material_editor_constants.h"
#include "material_widget.h"
#include "shader_widget.h"
#include "material_properties.h"

#include "../core/icore.h"
#include "../core/core_constants.h"
#include "../core/core.h"
#include "../core/menu_manager.h"

#include <nel/misc/debug.h>

#include <QDockWidget>
#include <QFileDialog>

namespace MaterialEditor
{
	MaterialEditorWindow::MaterialEditorWindow(QWidget *parent) :
	QMainWindow(parent)
	{
		m_ui.setupUi(this);
		shaderWidget = new ShaderWidget();
		matPropWidget = new MatPropWidget();
		createMenus();
		createDockWidgets();
	}
	
	MaterialEditorWindow::~MaterialEditorWindow()
	{
		delete shaderWidget;
		shaderWidget = NULL;
		delete matPropWidget;
		matPropWidget = NULL;
	}

	void MaterialEditorWindow::onOpenClicked()
	{
		QString fn = QFileDialog::getOpenFileName(
			this,
			tr( "Open model" ),
			"/",
			tr( "Shape files ( *.shape )" )
			);


	}

	void MaterialEditorWindow::onNewMaterialClicked()
	{
	}

	void MaterialEditorWindow::onOpenMaterialClicked()
	{
		QString fn = QFileDialog::getOpenFileName( 
			this,
			tr( "Open material" ),
			"/",
			tr( "Material files ( *.nelmat )" )
			);
	}

	void MaterialEditorWindow::onSaveMaterialClicked()
	{
		QString fn = QFileDialog::getSaveFileName(
			this,
			tr( "Save material" ),
			"/",
			tr( "Material files ( *.nelmat )" )
			);
	}

	void MaterialEditorWindow::onEditMaterialClicked()
	{
		matPropWidget->show();
	}

	void MaterialEditorWindow::onShadersClicked()
	{
		shaderWidget->show();
	}
	
	void MaterialEditorWindow::createMenus()
	{
		Core::MenuManager *mm = Core::ICore::instance()->menuManager();

		QMenu *menu = mm->menu( Core::Constants::M_TOOLS );
		if( menu != NULL )
		{
			QMenu *m = menu->addMenu( tr( "Material Editor" ) );
			QAction *a;

			QMenu *mm = m->addMenu( tr( "Material" ) );
			{
				a = new QAction( tr( "New material" ), NULL );
				connect( a, SIGNAL( triggered( bool ) ), this, SLOT( onNewMaterialClicked() ) );
				mm->addAction( a );
				
				a = new QAction( tr( "Open material" ) , NULL );
				connect( a, SIGNAL( triggered( bool ) ), this, SLOT( onOpenMaterialClicked() ) );
				mm->addAction( a );
				
				a = new QAction( tr( "Save material" ), NULL );
				connect( a, SIGNAL( triggered( bool ) ), this, SLOT( onSaveMaterialClicked() ) );
				mm->addAction( a );
				
				a = new QAction( tr( "Edit material" ), NULL );
				connect( a, SIGNAL( triggered( bool ) ), this, SLOT( onEditMaterialClicked() ) );
				mm->addAction( a );
			}

			a = new QAction( tr( "Shaders" ), NULL );
			connect( a, SIGNAL( triggered( bool ) ), this, SLOT( onShadersClicked() ) );
			m->addAction( a );

		}
	}

	void MaterialEditorWindow::createDockWidgets()
	{
		QDockWidget *dock = new QDockWidget( tr( "Material" ), this );
		dock->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
		dock->setWidget( new MaterialWidget() );
		addDockWidget( Qt::RightDockWidgetArea, dock );
	}
	
}
