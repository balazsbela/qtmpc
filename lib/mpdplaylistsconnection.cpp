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

#include <QMessageBox>

#include "mpderror.h"
#include "mpdplaylistsconnection.h"
#include "mpdparseutils.h"

MPDPlaylistsConnection::MPDPlaylistsConnection(QObject *parent)
	: MPDConnectionBase(parent)
{
}

MPDPlaylistsConnection::MPDPlaylistsConnection(const QString &host, const quint16 port,
QObject*parent)
	: MPDConnectionBase(host, port, parent)
{
}

void MPDPlaylistsConnection::listPlaylist()
{
	QByteArray *data;

	sendCommand("listplaylist");
	data = readFromSocket();

	delete data;
	disconnectFromMPD();
}

void MPDPlaylistsConnection::listPlaylistInfo()
{
}

void MPDPlaylistsConnection::listPlaylists()
{
	QByteArray *data;

	sendCommand("listplaylists");
	data = readFromSocket();

	emit playlistsRetrieved(MPDParseUtils::parsePlaylists(data));

	delete data;
	disconnectFromMPD();
}

void MPDPlaylistsConnection::load(QString name)
{
	QByteArray data("load ");
	data += "\"" + name.toUtf8().replace("\\", "\\\\").replace("\"", "\\\"") + "\"";

	sendCommand(data);

	if(!commandOk()) {
		qDebug("Couldn't load playlist");
	}
	disconnectFromMPD();
}

void MPDPlaylistsConnection::rename(const QString oldName, const QString newName)
{
	QByteArray data("rename ");
	data += "\"" + oldName.toUtf8().replace("\\", "\\\\").replace("\"", "\\\"") + "\"";
	data += " ";
	data += "\"" + newName.toUtf8().replace("\\", "\\\\").replace("\"", "\\\"") + "\"";

	sendCommand(data);

	if(!commandOk()) {
		qDebug("Couldn't rename playlist");
	}
	disconnectFromMPD();
}

void MPDPlaylistsConnection::rm(QString name)
{
	QByteArray data("rm ");
	data += "\"" + name.toUtf8().replace("\\", "\\\\").replace("\"", "\\\"") + "\"";

	sendCommand(data);

	if(!commandOk()) {
		qDebug("Couldn't remove playlist");
	}
	disconnectFromMPD();
}

void MPDPlaylistsConnection::save(QString name)
{
	QByteArray data("save ");
	data += "\"" + name.toUtf8().replace("\\", "\\\\").replace("\"", "\\\"") + "\"";

	sendCommand(data);

	MPDError *mpdError = commandResult();
	if(mpdError->isError()) {
		if(mpdError->errorString().endsWith("Playlist already exists\n")) {
			QMessageBox::warning(NULL, "Warning", "Playlist couldn't be saved since there already exists a playlist with the same name.");
		}
	}

	delete mpdError;
	disconnectFromMPD();
}

