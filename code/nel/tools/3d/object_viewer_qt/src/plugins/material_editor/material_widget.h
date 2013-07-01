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

#ifndef MATERIAL_WIDGET_H
#define MATERIAL_WIDGET_H

#include "ui_material_widget.h"
#include "material_observer.h"

namespace MaterialEditor
{
	class ShaderEditorWidget;
	class MatPropWidget;

	class MaterialWidget : public QWidget, public Ui::MaterialWidget, public CMaterialObserver
	{
		Q_OBJECT
	public:
		MaterialWidget( QWidget *parent = NULL );
		~MaterialWidget();

		void onPassAdded( const char *name );
		void onPassRemoved( const char *name );
		void onPassMovedUp( const char *name );
		void onPassMovedDown( const char *name );
		void onPassRenamed( const char *from, const char *to );

	private:
		void setupConnections();
		ShaderEditorWidget *shaderEditorWidget;
		MatPropWidget *matPropWidget;

	private Q_SLOTS:
		void onPassEditClicked();
		void onShaderEditClicked();
		void onShaderEditOKClicked();
	};

}

#endif
