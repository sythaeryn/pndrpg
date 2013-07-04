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

#include "material_properties.h"
#include "material_property_editor.h"
#include "nel3d_interface.h"

#include <QMessageBox>

namespace MaterialEditor
{

	MatPropWidget::MatPropWidget( QWidget *parent ) :
	QDialog( parent )
	{
		setupUi( this );
		matPropEditWidget = new MatPropEditWidget();
		setupConnections();
		edit = false;
		changed = false;
		proxy = NULL;
	}

	MatPropWidget::~MatPropWidget()
	{
		clear();

		delete matPropEditWidget;
		matPropEditWidget = NULL;
	}

	void MatPropWidget::load( CRenderPassProxy *proxy )
	{
		clear();
		changed = false;
		this->proxy = new CRenderPassProxy( *proxy );

		std::string n;
		proxy->getName( n );
		nameEdit->setText( n.c_str() );

		std::vector< SMatProp > v;
		proxy->getProperties( v );

		std::vector< SMatProp >::iterator itr = v.begin();
		while( itr != v.end() )
		{
			SMatProp &mp = *itr;
			QTreeWidgetItem *item = new QTreeWidgetItem();

			item->setData( 0, Qt::DisplayRole, QString( mp.id.c_str() ) );
			item->setData( 1, Qt::DisplayRole, QString( mp.label.c_str() ) );
			
			QString type = SMatProp::typeIdToString( mp.type ).c_str();
			item->setData( 2, Qt::DisplayRole, type );

			treeWidget->addTopLevelItem( item );

			++itr;
		}
	}

	void MatPropWidget::clear()
	{
		treeWidget->clear();
		nameEdit->clear();
		if( this->proxy != NULL )
		{
			delete this->proxy;
			this->proxy = NULL;
		}
	}

	void MatPropWidget::onOKClicked()
	{
		if( proxy != NULL )
		{
			std::vector< SMatProp > v;
			SMatProp p;
			QTreeWidgetItem *item = NULL;
			std::string s;

			for( int i = 0; i < treeWidget->topLevelItemCount(); i++ )
			{
				item = treeWidget->topLevelItem( i );
				p.id = item->text( 0 ).toUtf8().data();
				p.label = item->text( 1 ).toUtf8().data();
				
				s = item->text( 2 ).toUtf8().data();
				p.type = SMatProp::typeStringToId( s );

				v.push_back( p );
			}

			proxy->setProperties( v );
		}

		clear();
		setResult( QDialog::Accepted );
		close();
	}

	void MatPropWidget::onCancelClicked()
	{
		clear();
		setResult( QDialog::Rejected );
		close();
	}

	void MatPropWidget::onAddClicked()
	{
		edit = false;
		changed = true;
		matPropEditWidget->clear();
		matPropEditWidget->show();
	}

	void MatPropWidget::onEditClicked()
	{
		QTreeWidgetItem *item = treeWidget->currentItem();
		if( item == NULL )
			return;

		MaterialProperty prop;
		prop.prop  = item->data( 0, Qt::DisplayRole ).toString();
		prop.label = item->data( 1, Qt::DisplayRole ).toString();
		prop.type  = item->data( 2, Qt::DisplayRole ).toString();
		
		edit = true;
		matPropEditWidget->setProperty( prop );
		matPropEditWidget->show();
	}

	void MatPropWidget::onRemoveClicked()
	{
		QTreeWidgetItem *item = treeWidget->currentItem();
		if( item == NULL )
			return;

		delete item;
		changed = true;
	}

	void MatPropWidget::onEditorOKClicked()
	{
		MaterialProperty prop;
		matPropEditWidget->getProperty( prop );

		if( edit )
		{
			QTreeWidgetItem *item = treeWidget->currentItem();

			MaterialProperty old;
			old.prop = item->text( 0 );
			old.label = item->text( 1 );
			old.type = item->text( 2 );

			if( old == prop )
				return;


			if( idExists( prop.prop ) )
			{
				QMessageBox::critical(
					this,
					tr( "Property Id" ),
					tr( "A property with that Id already exists" )
					);

				return;
			}

			if( labelExists( prop.label ) )
			{
				QMessageBox::critical(
					this,
					tr( "Property label" ),
					tr( "A property with that label already exists" )
					);
				return;
			}

			
			item->setData( 0, Qt::DisplayRole, prop.prop );
			item->setData( 1, Qt::DisplayRole, prop.label );
			item->setData( 2, Qt::DisplayRole, prop.type );
		}
		else
		{

			if( idExists( prop.prop ) )
			{
				QMessageBox::critical(
					this,
					tr( "Property Id" ),
					tr( "A property with that Id already exists" )
					);

				return;
			}

			if( labelExists( prop.label ) )
			{
				QMessageBox::critical(
					this,
					tr( "Property label" ),
					tr( "A property with that label already exists" )
					);
				return;
			}

			QTreeWidgetItem *item = new QTreeWidgetItem();
			item->setData( 0, Qt::DisplayRole, prop.prop );
			item->setData( 1, Qt::DisplayRole, prop.label );
			item->setData( 2, Qt::DisplayRole, prop.type );
			treeWidget->addTopLevelItem( item );
		}
		changed = true;

	}

	void MatPropWidget::setupConnections()
	{
		connect( okButton, SIGNAL( clicked( bool ) ), this, SLOT( onOKClicked() ) );
		connect( cancelButton, SIGNAL( clicked( bool ) ), this, SLOT( onCancelClicked() ) );
		connect( addButton, SIGNAL( clicked( bool ) ), this, SLOT( onAddClicked() ) );
		connect( editButton, SIGNAL( clicked( bool ) ), this, SLOT( onEditClicked() ) );
		connect( removeButton, SIGNAL( clicked( bool ) ), this, SLOT( onRemoveClicked() ) );
		connect( matPropEditWidget, SIGNAL( okClicked() ), this, SLOT( onEditorOKClicked() ) );
	}

	bool MatPropWidget::idExists( const QString &id )
	{
		int c = treeWidget->topLevelItemCount();
		for( int i = 0; i < c; i++ )
		{
			if( id == treeWidget->topLevelItem( i )->text( 0 ) )
				return true;
		}

		return false;
	}

	bool MatPropWidget::labelExists( const QString &label )
	{
		int c = treeWidget->topLevelItemCount();
		for( int i = 0; i < c; i++ )
		{
			if( label == treeWidget->topLevelItem( i )->text( 1 ) )
				return true;
		}

		return false;
	}

}

