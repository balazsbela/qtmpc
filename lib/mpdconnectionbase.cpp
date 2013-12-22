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

#include "mpdconnectionbase.h"
#include "mpdparseutils.h"

MPDConnectionBase::MPDConnectionBase(QObject *parent)
	: QThread(parent),
		port(0)
{
}

MPDConnectionBase::MPDConnectionBase(const QString &host, const quint16 port, QObject*parent)
	: QThread(parent),
		hostname(host), port(port)
{
}

MPDConnectionBase::~MPDConnectionBase()
{
	sock.disconnectFromHost();
	quit();

	while(isRunning())
		msleep(100);
}

void MPDConnectionBase::disconnectFromMPD()
{
	sock.disconnectFromHost();
}

void MPDConnectionBase::setHostname(const QString &host)
{
	hostname = host;
}

void MPDConnectionBase::setPort(const quint16 port)
{
	this->port = port;
}

bool MPDConnectionBase::isConnected()
{
	return (sock.state() == QAbstractSocket::ConnectedState);
}

QByteArray * MPDConnectionBase::readFromSocket()
{
	QByteArray *data = new QByteArray;

	while(true) {
		while(sock.bytesAvailable() == 0) {
			qDebug("MPDConnectionBase: waiting for data.");
			if(sock.waitForReadyRead(5000)) {
				break;
			}
		}

		data->append(sock.readAll());

		if(data->endsWith("OK\n") || data->startsWith("OK") || data->startsWith("ACK")) {
			break;
		}
	}

	qDebug("Received: %s", data->constData());

	mutex.unlock();

	return data;
}

bool MPDConnectionBase::connectToMPD()
{
	QByteArray senddata;
	QByteArray *recvdata;

	if(!isConnected()) {
		if(hostname.isEmpty() || port == 0) {
			qWarning("MPDConnectionBase: no hostname and/or port supplied.");
			return false;
		}

		mutex.lock();
		sock.connectToHost(hostname, port);
		if(sock.waitForConnected(5000)) {
			qDebug("MPDConnectionBase established");
			recvdata = MPDConnectionBase::readFromSocket();

			if(recvdata->startsWith("OK MPD")) {
				qDebug("Received identification string");
			}

			delete recvdata;

			if(!password.isEmpty()) {
				mutex.lock();
				qDebug("MPDConnectionBase: setting password...");
				senddata = "password ";
				senddata += password.toUtf8();
				senddata += "\n";
				sock.write(senddata);
				sock.waitForBytesWritten(5000);

				if(MPDConnectionBase::commandOk()) {
					qDebug("MPDConnectionBase: password accepted");
				} else {
					qDebug("MPDConnectionBase: password rejected");
				}
			}

			return true;
		} else {
			mutex.unlock();
			qWarning("Couldn't connect");
			return false;
		}
	}

	qDebug("Already connected");
	return true;
}

/*
 * Use either this function or commandResult() to check whether a command
 * succeeded or not.
 */
bool MPDConnectionBase::commandOk()
{
	QByteArray data;

	while(!sock.canReadLine()) {
		qDebug("MPDConnectionBase: waiting for data.");
		if(sock.waitForReadyRead(5000)) {
			break;
		}
	}

	data += sock.readAll();
	
	mutex.unlock();

	if(data == "OK\n")
		return true;

	qWarning() << "MPD Error:" << data;

	return false;
}

/*
 * Use either this function or commandOk() to check whether a command succeeded
 * or not.
 */
MPDError * MPDConnectionBase::commandResult()
{
	QByteArray data;

	while(!sock.canReadLine()) {
		qDebug("MPDConnectionBase: waiting for data.");
		if(sock.waitForReadyRead(5000)) {
			break;
		}
	}

	data += sock.readAll();
	
	mutex.unlock();

	if(data == "OK\n")
		return new MPDError(MPDError::State_OK, QString());

	qWarning() << "MPD Error:" << data;

	return new MPDError(MPDError::State_Error, data);
}

void MPDConnectionBase::sendCommand(const QByteArray &command)
{
	if(!connectToMPD()) {
		return;
	}

	mutex.lock();
	
	qDebug() << "command: " << command;

	sock.write(command);
	sock.write("\n");
	sock.waitForBytesWritten(5000);
}

void MPDConnectionBase::clearError()
{
}

void MPDConnectionBase::closeConnection()
{
}

void MPDConnectionBase::getCommands()
{
}

void MPDConnectionBase::getNotCommands()
{
}

void MPDConnectionBase::setPassword(const QString &pass)
{
	this->password = pass;
}

void MPDConnectionBase::ping()
{
}
