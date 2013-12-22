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

#ifndef MPDPLAYLISTSCONNECTION_H
#define MPDPLAYLISTSCONNECTION_H

#include "lib/mpdconnectionbase.h"
#include "lib/playlist.h"

class MusicLibraryItemArtist;
class DirViewItemRoot;
class QString;

class MPDPlaylistsConnection : public MPDConnectionBase
{
	Q_OBJECT

	public:
		MPDPlaylistsConnection(QObject *parent = 0);
		MPDPlaylistsConnection(const QString &host, const quint16 port, QObject *parent = 0);

		void listPlaylist();
		void listPlaylistInfo();
		void listPlaylists();
		void load(QString name);
		void rename(const QString oldName, const QString newName);
		void rm(QString name);
		void save(QString name);

	signals:
		void playlistsRetrieved(QList<Playlist *> *data);
};

#endif
