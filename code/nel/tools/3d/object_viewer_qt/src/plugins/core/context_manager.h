// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#ifndef CONTEXT_MANAGER_H
#define CONTEXT_MANAGER_H

// Project includes
#include "core_global.h"

// Qt includes
#include <QtCore/QObject>

QT_BEGIN_NAMESPACE
class QTabWidget;
QT_END_NAMESPACE

namespace Core
{
class IContext;
struct ContextManagerPrivate;

class CORE_EXPORT ContextManager : public QObject
{
	Q_OBJECT

public:
	explicit ContextManager(QTabWidget *tabWidget);
	virtual ~ContextManager();

	Core::IContext* currentContext() const;
	Core::IContext* context(const QString &id) const;

Q_SIGNALS:
	// the default argument '=0' is important for connects without the oldContext argument.
	void currentContextChanged(Core::IContext *context, Core::IContext *oldContext = 0);

public Q_SLOTS:
	void activateContext(const QString &id);

private Q_SLOTS:
	void addContextObject(IContext *context);
	void removeContextObject(IContext *context);
	void currentTabChanged(int index);

private:
	int indexOf(const QString &id) const;

	ContextManagerPrivate *d;
};

} // namespace Core

#endif // CONTEXT_MANAGER_H
