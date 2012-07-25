// Object Viewer Qt GUI Editor plugin <http://dev.ryzom.com/projects/ryzom/>
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


#include "link_editor.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/widget_manager.h"

namespace GUIEditor
{
	LinkEditor::LinkEditor( QWidget *parent ) :
	QWidget( parent )
	{
		setupUi( this );
		setup();
		connect( okButton, SIGNAL( clicked( bool ) ), this, SLOT( onOKButtonClicked() ) );
		connect( cancelButton, SIGNAL( clicked( bool ) ), this, SLOT( hide() ) );
	}

	LinkEditor::~LinkEditor()
	{
	}

	void LinkEditor::setup()
	{
		expressionEdit->clear();
		targetEdit->clear();
		ahEdit->clear();
		ahParamEdit->clear();
		condEdit->clear();
	}

	void LinkEditor::setLinkId( uint32 linkId )
	{
		setup();
		currentLinkId = linkId;

		IParser *parser = CWidgetManager::getInstance()->getParser();
		SLinkData data;

		if( !parser->getLinkData( currentLinkId, data ) )
			return;

		expressionEdit->setPlainText( data.expr.c_str() );
		targetEdit->setText( data.target.c_str() );
		ahEdit->setText( data.action.c_str() );
		ahParamEdit->setText( data.params.c_str() );
		condEdit->setText( data.cond.c_str() );
	}

	void LinkEditor::onOKButtonClicked()
	{
		IParser *parser = CWidgetManager::getInstance()->getParser();
		SLinkData data;

		if( !parser->getLinkData( currentLinkId, data ) )
			return;

		data.expr = expressionEdit->toPlainText().toStdString();		
		data.target = targetEdit->text().toStdString();
		data.action = ahEdit->text().toStdString();
		data.params = ahParamEdit->text().toStdString();
		data.cond   = condEdit->text().toStdString();
		parser->updateLinkData( data.id, data );

		hide();
	}
}
