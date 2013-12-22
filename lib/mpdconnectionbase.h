/*
 * Copyright (c) 2008 Sander Knopper (sander AT knopper DOT tk) and
 *                    Roeland Douma (roeland AT rullzer DOT com)
 *
 * This file is part of QtMPC.
 *
 * QtMPC is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * QtMPC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with QtMPC.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MPDCONNECTIONBASE_H
#define MPDCONNECTIONBASE_H

#include <QMutex>
#include <QThread>
#include <QTcpSocket>
#include <QDateTime>

#include "mpderror.h"
#include "mpdstats.h"
#include "mpdstatus.h"
#include "song.h"

class MPDConnectionBase : public QThread
{
	Q_OBJECT

	public:
		MPDConnectionBase(QObject *parent = 0);
		MPDConnectionBase(const QString &host, const quint16 port, QObject *parent = 0);
		~MPDConnectionBase();
		
		bool connectToMPD();
		void setHostname(const QString &host);
		void setPort(const quint16 port);
		bool isConnected();
		void disconnectFromMPD();

		// Miscellaneous
		void clearError();
		void closeConnection();
		void getCommands();
		void getNotCommands();
		void setPassword(const QString &pass);
		void ping();

	protected:
		QString hostname;
		quint16 port;
		QString password;
		QTcpSocket sock;
		QMutex mutex;

		virtual bool commandOk();
		virtual MPDError * commandResult();
		virtual void sendCommand(const QByteArray &command);

	protected slots:
		virtual QByteArray * readFromSocket();
};

#endif
