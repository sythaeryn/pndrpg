// Nel MMORPG framework - Error Reporter
//
// Copyright (C) 2015 Laszlo Kis-Adam
// Copyright (C) 2010 Ryzom Core <http://ryzomcore.org/>
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


#ifndef RCERROR_SOCKET
#define RCERROR_SOCKET

#include <QObject>
#include "rcerror_data.h"

class CRCErrorSocketPvt;
class QNetworkReply;

class CRCErrorSocket : public QObject
{
	Q_OBJECT

public:	
	CRCErrorSocket( QObject *parent );
	~CRCErrorSocket();

	void sendReport( const SRCErrorData &data );

Q_SIGNALS:
	void reportSent();
	void reportFailed();

private Q_SLOTS:
	void onFinished( QNetworkReply *reply );

private:
	CRCErrorSocketPvt *m_pvt;
};

#endif

