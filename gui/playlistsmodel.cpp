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

#include <QModelIndex>

#include "playlistsmodel.h"
#include "mpdparseutils.h"
#include "mpdstats.h"

PlaylistsModel::PlaylistsModel(QObject *parent)
	: QAbstractListModel(parent),
		m_playlists(NULL)
{
	connect(&m_mpdConnection, SIGNAL(playlistsRetrieved(QList<Playlist *> *)), this, SLOT(setPlaylists(QList<Playlist *> *)));
}

PlaylistsModel::~PlaylistsModel()
{
}

QVariant PlaylistsModel::headerData(int /*section*/, Qt::Orientation /*orientation*/, int /*role*/) const
{
	return QVariant();
}

int PlaylistsModel::rowCount(const QModelIndex &) const
{
	return (m_playlists == NULL ? 0 : m_playlists->size());
}

QVariant PlaylistsModel::data(const QModelIndex &index, int role) const
{
	if(!index.isValid())
		return QVariant();
	
	if(index.row() >= m_playlists->size())
		return QVariant();

	if(role == Qt::DisplayRole)
		return m_playlists->at(index.row())->m_name;

	return QVariant();
}

bool PlaylistsModel::connectMpd(QString host, quint16 port, QString password)
{
	m_mpdConnection.setHostname(host);
	m_mpdConnection.setPort(port);
	m_mpdConnection.setPassword(password);

	return m_mpdConnection.connectToMPD();
}

void PlaylistsModel::getPlaylists()
{
	m_mpdConnection.listPlaylists();
}

void PlaylistsModel::setPlaylists(QList<Playlist *> *playlists)
{
	beginResetModel();

	QList<Playlist *> *playlists_old;

	if(m_playlists == NULL) {
		m_playlists = playlists;
	} else {
		playlists_old = m_playlists;
		m_playlists = playlists;

		while(!playlists_old->isEmpty())
			delete playlists_old->takeLast();

		delete playlists_old;
	}

	endResetModel();
}

void PlaylistsModel::loadPlaylist(QString name)
{
	m_mpdConnection.load(name);

	emit playlistLoaded();
}

void PlaylistsModel::removePlaylist(QString name)
{
	m_mpdConnection.rm(name);
}

void PlaylistsModel::savePlaylist(QString name)
{
	m_mpdConnection.save(name);
}

void PlaylistsModel::renamePlaylist(const QString oldname, const QString newname)
{
	m_mpdConnection.rename(oldname, newname);
}