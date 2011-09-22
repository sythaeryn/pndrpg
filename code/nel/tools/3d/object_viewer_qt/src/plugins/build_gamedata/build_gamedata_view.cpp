// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include "build_gamedata_view.h"

#include "nel/misc/path.h"

#include <QtGui/QMessageBox>
#include <QtGui/QApplication>


BuildGamedataView::BuildGamedataView(QWidget *parent) : QDockWidget(parent)
{
	m_ui.setupUi(this);
	m_process = new QProcess(this);
	m_process->setReadChannel(QProcess::StandardOutput);
	m_buttonMapper = new QSignalMapper(this);

	connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(readFromStdOut()) );
	
	connect(m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(receiveProcessError(QProcess::ProcessError)));
	connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(receiveProcessComplete(int, QProcess::ExitStatus)));
	connect(m_buttonMapper, SIGNAL(mapped(int)), this, SLOT(execute(int)));

	connect(m_ui.stepExportPushButton, SIGNAL(clicked()), m_buttonMapper, SLOT(map()));
	m_buttonMapper->setMapping(m_ui.stepExportPushButton, 1);

	connect(m_ui.stepBuildPushButton, SIGNAL(clicked()), m_buttonMapper, SLOT(map()));
	m_buttonMapper->setMapping(m_ui.stepBuildPushButton, 2);

	connect(m_ui.stepInstallPushButton, SIGNAL(clicked()), m_buttonMapper, SLOT(map()));
	m_buttonMapper->setMapping(m_ui.stepInstallPushButton, 3);

	connect(m_ui.stepDataShardPushButton, SIGNAL(clicked()), m_buttonMapper, SLOT(map()));
	m_buttonMapper->setMapping(m_ui.stepDataShardPushButton, 4);

	connect(m_ui.stepClientDevPushButton, SIGNAL(clicked()), m_buttonMapper, SLOT(map()));
	m_buttonMapper->setMapping(m_ui.stepClientDevPushButton, 5);

	connect(m_ui.stepClientPatchPushButton, SIGNAL(clicked()), m_buttonMapper, SLOT(map()));
	m_buttonMapper->setMapping(m_ui.stepClientPatchPushButton, 6);

	connect(m_ui.stepClientInstallPushButton, SIGNAL(clicked()), m_buttonMapper, SLOT(map()));
	m_buttonMapper->setMapping(m_ui.stepClientInstallPushButton, 7);

	connect(m_ui.exportBuildInstallPB, SIGNAL(clicked()), m_buttonMapper, SLOT(map()));
	m_buttonMapper->setMapping(m_ui.exportBuildInstallPB, 8);

	connect(m_ui.installClientDevPB, SIGNAL(clicked()), m_buttonMapper, SLOT(map()));
	m_buttonMapper->setMapping(m_ui.installClientDevPB, 9);

	connect(m_ui.installDataShardPB, SIGNAL(clicked()), m_buttonMapper, SLOT(map()));
	m_buttonMapper->setMapping(m_ui.installDataShardPB, 10);
}

BuildGamedataView::~BuildGamedataView()
{
}

void BuildGamedataView::execute(int id)
{
	nlinfo("starting execute step export");
	// Make sure the process isn't currently running.
	if(!m_process->state()==QProcess::NotRunning)
		return;

	QString exportScript;
	switch(id)
	{
	case 1:
		exportScript = "c:/ryzomcore/build_gamedata/1_export.py";
		break;
	case 2:
		exportScript = "c:/ryzomcore/build_gamedata/1_build.py";
		break;
	case 3:;
		exportScript = "c:/ryzomcore/build_gamedata/1_install.py";
		break;
	case 4:
		exportScript = "c:/ryzomcore/build_gamedata/1_data_shard.py";
		break;
	case 5:
		exportScript = "c:/ryzomcore/build_gamedata/1_client_dev.py";
		break;
	case 6:
		exportScript = "c:/ryzomcore/build_gamedata/1_client_patch.py";
		break;
	case 7:
		exportScript = "c:/ryzomcore/build_gamedata/1_client_install.py";
		break;
	case 8:
		exportScript = "c:/ryzomcore/build_gamedata/export_build_install.py";
		break;
	case 9:
		exportScript = "c:/ryzomcore/build_gamedata/install_client_dev.py";
		break;
	case 10:
		exportScript = "c:/ryzomcore/build_gamedata/install_data_shard.py";
		break;
	};

	nlinfo("about to start");
	QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
	m_process->setProcessEnvironment(env);
	m_process->setWorkingDirectory("c:/ryzomcore/build_gamedata");
	m_process->start("C:/Python27/python.exe", QStringList() << exportScript);
	nlinfo("started step execute export");

	
	toggleButtons(false);
}

void BuildGamedataView::toggleButtons(bool active)
{
	m_ui.stepExportPushButton->setEnabled(active);
	m_ui.stepBuildPushButton->setEnabled(active);
	m_ui.stepInstallPushButton->setEnabled(active);
	m_ui.stepDataShardPushButton->setEnabled(active);
	m_ui.stepClientDevPushButton->setEnabled(active);
	m_ui.stepClientPatchPushButton->setEnabled(active);
	m_ui.stepClientInstallPushButton->setEnabled(active);
	m_ui.exportBuildInstallPB->setEnabled(active);
	m_ui.installClientDevPB->setEnabled(active);
	m_ui.installDataShardPB->setEnabled(active);
}

void BuildGamedataView::readFromStdOut()
{
	m_ui.outputTextEdit->append(m_process->readAll());
}

void BuildGamedataView::receiveProcessError(QProcess::ProcessError error)
{
	nlinfo("running pocess failed!");
	if(error == QProcess::FailedToStart)
		nlinfo("failed to start.");
	if(error == QProcess::ReadError)
		nlinfo("read error");
	if(error == QProcess::Crashed)
		nlinfo("crashed.");
	if(error == QProcess::UnknownError)
		nlinfo("unknown error.");
	toggleButtons(true);
}

void BuildGamedataView::receiveProcessComplete(int exitCode, QProcess::ExitStatus exitStatus)
{
	nlinfo("running process completed.");
	toggleButtons(true);
}