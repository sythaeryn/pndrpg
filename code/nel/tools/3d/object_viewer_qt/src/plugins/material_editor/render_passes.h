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

#ifndef RENDER_PASSES_H
#define RENDER_PASSES_H

#include "ui_render_passes.h"

namespace MaterialEditor
{
	class RenderPassesWidget : public QWidget, public Ui::RenderPassesWidget
	{
		Q_OBJECT
	public:
		RenderPassesWidget( QWidget *parent = NULL );
		~RenderPassesWidget();

	private:
		void setupConnections();

	private Q_SLOTS:
		void onOKClicked();
		void onCancelClicked();
		void onAddClicked();
		void onRemoveClicked();
		void onEditClicked();
		void onUpClicked();
		void onDownClicked();
	};
}

#endif

