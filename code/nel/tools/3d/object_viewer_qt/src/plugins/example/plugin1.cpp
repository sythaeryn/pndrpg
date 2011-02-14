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

	_plugMan->addObject(new CExampleSettingsPage(this));
	_plugMan->addObject(new CExampleAppPage(this));
	_plugMan->addObject(new CCoreListener(this));
	return true;
}

void MyPlugin::extensionsInitialized()
{
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