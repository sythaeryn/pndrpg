// Project includes
#include "plugin1.h"
#include "example_settings_page.h"
#include "simple_viewer.h"
#include "../core/iapp_page.h"
#include "../../extension_system/iplugin_spec.h"

// NeL includes
#include "nel/misc/debug.h"

// Qt includes
#include <QtCore/QObject>
#include <QtGui/QMessageBox>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QAction>
#include <QtGui/QMenuBar>

namespace Plugin
{

bool MyPlugin::initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString)
{
	Q_UNUSED(errorString);
	_plugMan = pluginManager;

	QMainWindow *wnd = qobject_cast<QMainWindow *>(objectByName("CMainWindow"));
	_plugMan->addObject(new CExampleSettingsPage(wnd));
	if (!wnd)
	{
		*errorString = tr("Not found QMainWindow Object Viewer Qt.");
		return false;
	}
	QMenu *helpMenu = qobject_cast<QMenu *>(objectByName("ovqt.Menu.Tools"));
	if (!helpMenu)
	{
		*errorString = tr("Not found QMenu Help.");
		return false;
	}
	_plugMan->addObject(new CExampleAppPage());
	return true;
}

void MyPlugin::extensionsInitialized()
{
	QMenu *helpMenu = qobject_cast<QMenu *>(objectByName("ovqt.Menu.Help"));
	nlassert(helpMenu);

	helpMenu->addSeparator();
	QAction *newAction = helpMenu->addAction("MyPlugin");

	connect(newAction, SIGNAL(triggered()), this, SLOT(execMessageBox()));
}

void MyPlugin::execMessageBox()
{
	QMainWindow *wnd = qobject_cast<QMainWindow *>(objectByName("CMainWindow"));
	nlassert(wnd);

	QMessageBox msgBox;
	msgBox.setText(wnd->objectName() + QString(": width=%1,height=%2").arg(wnd->width()).arg(wnd->height()));
	msgBox.exec();
}

void MyPlugin::setNelContext(NLMISC::INelContext *nelContext)
{
#ifdef NL_OS_WINDOWS
	// Ensure that a context doesn't exist yet.
	// This only applies to platforms without PIC, e.g. Windows.
	nlassert(!NLMISC::INelContext::isContextInitialised());
#endif // NL_OS_WINDOWS
	_LibContext = new NLMISC::CLibraryContext(*nelContext);
}

QString MyPlugin::name() const
{
	return "ExamplePlugin";
}

QString MyPlugin::version() const
{
	return "0.2";
}

QString MyPlugin::vendor() const
{
	return "dnk-88";
}

QString MyPlugin::description() const
{
	return "Example ovqt plugin.";
}

QList<QString> MyPlugin::dependencies() const
{
	return QList<QString>();
}

QObject* MyPlugin::objectByName(const QString &name) const
{
	Q_FOREACH (QObject *qobj, _plugMan->allObjects())
		if (qobj->objectName() == name)
			return qobj;
	return 0;
}

ExtensionSystem::IPluginSpec *MyPlugin::pluginByName(const QString &name) const
{
	Q_FOREACH (ExtensionSystem::IPluginSpec *spec, _plugMan->plugins())
		if (spec->name() == name)
			return spec;
	return 0;
}

}

Q_EXPORT_PLUGIN(Plugin::MyPlugin)