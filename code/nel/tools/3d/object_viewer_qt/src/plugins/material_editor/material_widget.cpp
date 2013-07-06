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

#include "material_widget.h"
#include "shader_editor.h"
#include "material_properties.h"
#include "nel3d_interface.h"

namespace MaterialEditor
{

	MaterialWidget::MaterialWidget( QWidget *parent ) :
	QWidget( parent )
	{
		setupUi( this );
		shaderEditorWidget = new ShaderEditorWidget();
		matPropWidget = new MatPropWidget();
		setNel3DIface( NULL );
		setupConnections();
	}

	MaterialWidget::~MaterialWidget()
	{
		delete shaderEditorWidget;
		shaderEditorWidget = NULL;
		delete matPropWidget;
		matPropWidget = NULL;
	}

	void MaterialWidget::onNewMaterial()
	{
		passCB->clear();
		passButton->setEnabled( false );
	}

	void MaterialWidget::onMaterialLoaded()
	{
		CNelMaterialProxy mat = nl3dIface->getMaterial();
		
		std::vector< std::string > l;
		mat.getPassList( l );

		passCB->clear();

		std::vector< std::string >::const_iterator itr = l.begin();
		while( itr != l.end() )
		{
			passCB->addItem( itr->c_str() );
			++itr;
		}

		if( passCB->count() > 0 )
			passButton->setEnabled( true );
	}

	void MaterialWidget::onPassAdded( const char *name )
	{
		passCB->addItem( name );
		passButton->setEnabled( true );
	}

	void MaterialWidget::onPassRemoved( const char *name )
	{
		int i = passCB->findText( name );
		if( i == -1 )
			return;

		passCB->removeItem( i );
		if( passCB->count() == 0 )
			passButton->setEnabled( false );
	}

	void MaterialWidget::onPassMovedUp( const char *name )
	{
		int i = passCB->findText( name );
		if( i == -1 )
			return;
		if( i == 0 )
			return;

		QString t = passCB->itemText( i - 1 );
		passCB->setItemText( i - 1, name );
		passCB->setItemText( i, t );
		passCB->setCurrentIndex( i - 1 );
	}

	void MaterialWidget::onPassMovedDown( const char *name )
	{
		int i = passCB->findText( name );
		if( i == -1 )
			return;
		if( i == ( passCB->count() - 1 ) )
			return;

		QString t = passCB->itemText( i + 1 );
		passCB->setItemText( i + 1, name );
		passCB->setItemText( i, t );
		passCB->setCurrentIndex( i + 1 );
	}

	void MaterialWidget::onPassRenamed( const char *from, const char *to )
	{
		int i = passCB->findText( from );
		if( i == -1 )
			return;
		passCB->setItemText( i, to );
	}

	void MaterialWidget::onShaderAdded( const QString &name )
	{
		shaderCB->addItem( name );
	}

	void MaterialWidget::onShaderRemoved( const QString &name )
	{
		int i = shaderCB->findText( name );
		if( i < 0 )
			return;

		shaderCB->removeItem( i );
	}

	void MaterialWidget::getCurrentPass( QString &pass )
	{
		pass = passCB->currentText();
	}

	void MaterialWidget::setupConnections()
	{
		connect( passButton, SIGNAL( clicked( bool ) ), this, SLOT( onPassEditClicked() ) );
		connect( shaderButton, SIGNAL( clicked( bool ) ), this, SLOT( onShaderEditClicked() ) );
		connect( passCB, SIGNAL( currentIndexChanged( const QString& ) ), this, SLOT( onPassCBChanged( const QString& ) ) );

		connect( shaderEditorWidget, SIGNAL( okClicked() ), this, SLOT( onShaderEditOKClicked() ) );
	}

	void MaterialWidget::onPassEditClicked()
	{
		if( passCB->count() == 0 )
			return;

		CRenderPassProxy p = nl3dIface->getMaterial().getPass( passCB->currentText().toUtf8().data() );
		matPropWidget->load( &p );
		matPropWidget->exec();
		if( matPropWidget->getChanged() )
			Q_EMIT propsChanged();
	}

	void MaterialWidget::onShaderEditClicked()
	{
		shaderEditorWidget->show();
	}

	void MaterialWidget::onShaderEditOKClicked()
	{
	}

	void MaterialWidget::onPassCBChanged( const QString &text )
	{
		if( text.isEmpty() )
			return;

		Q_EMIT passChanged( text );
	}
}

