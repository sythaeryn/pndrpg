/*
    Object Viewer Qt
    Copyright (C) 2010 Dzmitry Kamiahin <dnk-88@tut.by>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "stdpch.h"
#include "direction_widget.h"

// Qt includes
#include <QtGui/QInputDialog>
#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>

// NeL includes
#include <nel/misc/vector.h>

namespace NLQT
{
const int directionSize = 35;

CDirectionWidget::CDirectionWidget(QWidget *parent)
	: QWidget(parent), _globalName("")
{
	_ui.setupUi(this);

	_ui.globalPushButton->hide();

	connect(_ui.globalPushButton ,SIGNAL(clicked()), this, SLOT(setGlobalDirection()));
	connect(_ui.incVecIPushButton ,SIGNAL(clicked()), this, SLOT(incVecI()));
	connect(_ui.incVecJPushButton ,SIGNAL(clicked()), this, SLOT(incVecJ()));
	connect(_ui.incVecKPushButton ,SIGNAL(clicked()), this, SLOT(incVecK()));
	connect(_ui.decVecIPushButton ,SIGNAL(clicked()), this, SLOT(decVecI()));
	connect(_ui.decVecJPushButton ,SIGNAL(clicked()), this, SLOT(decVecJ()));
	connect(_ui.decVecKPushButton ,SIGNAL(clicked()), this, SLOT(decVecK()));

	connect(_ui.zenithSpinBox ,SIGNAL(valueChanged(int)), this, SLOT(setNewSphericalCoord()));
	connect(_ui.azimuthSpinBox ,SIGNAL(valueChanged(int)), this, SLOT(setNewSphericalCoord()));
		
	// Set default value +K
	setValue(NLMISC::CVector::K);
}

CDirectionWidget::~CDirectionWidget()
{
}

void CDirectionWidget::enabledGlobalVariable(bool enabled)
{
	_ui.globalPushButton->setVisible(enabled);
	setGlobalName("", false);
}

void CDirectionWidget::setValue(const NLMISC::CVector &value, bool emit, bool updateWidget)
{
	_value = value;

	float azimuth, zenith, r;
	value.cartesianToSpheric(r, azimuth, zenith);

	// rad -> deg
	azimuth *= 180.0 / NLMISC::Pi;
	zenith *= 180.0 / NLMISC::Pi;
	
	if (updateWidget)
	{
		_ui.azimuthSpinBox->blockSignals(true);
		_ui.zenithSpinBox->blockSignals(true);

		_ui.azimuthDial->setValue(azimuth);
		_ui.zenithDial->setValue(zenith);

		_ui.azimuthSpinBox->blockSignals(false);
		_ui.zenithSpinBox->blockSignals(false);
	}

	if (emit)
	{
		Q_EMIT valueChanged(_value);
	}
}

void CDirectionWidget::setGlobalName(const QString &globalName, bool emit)
{
	_globalName = globalName;

	_ui.incVecIPushButton->setEnabled(_globalName.isEmpty());
	_ui.incVecJPushButton->setEnabled(_globalName.isEmpty());
	_ui.incVecKPushButton->setEnabled(_globalName.isEmpty());
	_ui.decVecIPushButton->setEnabled(_globalName.isEmpty());
	_ui.decVecJPushButton->setEnabled(_globalName.isEmpty());
	_ui.decVecKPushButton->setEnabled(_globalName.isEmpty());

	if (emit)
		globalNameChanged(_globalName);
}

void CDirectionWidget::setGlobalDirection()
{
	bool ok;
	QString text = QInputDialog::getText(this, tr("Enter Name"),
										 "", QLineEdit::Normal,
										 QString(_globalName), &ok);

	if (ok)
		setGlobalName(text);
}

void CDirectionWidget::incVecI()
{
	setValue(NLMISC::CVector::I);
}

void CDirectionWidget::incVecJ()
{
	setValue(NLMISC::CVector::J);
}

void CDirectionWidget::incVecK()
{
	setValue(NLMISC::CVector::K);
}

void CDirectionWidget::decVecI()
{
	setValue( - NLMISC::CVector::I);
}

void CDirectionWidget::decVecJ()
{
	setValue( - NLMISC::CVector::J);
}

void CDirectionWidget::decVecK()
{
	setValue( - NLMISC::CVector::K);
}

void CDirectionWidget::setNewSphericalCoord()
{
	NLMISC::CVector v = _value;
	
	// deg -> rad
	float azimuth = _ui.azimuthSpinBox->value() * (NLMISC::Pi / 180.0);
	float zenith = _ui.zenithSpinBox->value() * (NLMISC::Pi / 180.0);

	v.sphericToCartesian(1.0, azimuth, zenith);
	
	setValue(v, true, false);
}

} /* namespace NLQT */