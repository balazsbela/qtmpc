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

#include <QInputDialog>
#include <QMessageBox>
#include <QKeyEvent>
#include <QAction>
#ifdef ENABLE_KDE_SUPPORT
#include <KAction>
#endif

#include "main_window.h"

void MainWindow::setupPlaylists()
{
	playlistsProxyModel.setSourceModel(&playlistsModel);
	playlistsView->setModel(&playlistsProxyModel);
	playlistsView->sortByColumn(0, Qt::AscendingOrder);
	playlistsView->setContextMenuPolicy(Qt::ActionsContextMenu);
	playlistsView->addAction(action_Add_to_playlist);
	playlistsView->addAction(action_Load_playlist);
	playlistsView->addAction(action_Remove_playlist);
	playlistsView->addAction(action_Rename_playlist);

    connect(addPlaylistToPlaylistPushButton, SIGNAL(clicked(bool)), this, SLOT(addPlaylistToPlaylistPushButtonActivated()));
	connect(loadPlaylistPushButton, SIGNAL(clicked(bool)), this, SLOT(loadPlaylistPushButtonActivated()));
	connect(playlistsView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(playlistsViewItemDoubleClicked(const QModelIndex &)));
	connect(removePlaylistPushButton, SIGNAL(clicked(bool)), this, SLOT(removePlaylistPushButtonActivated()));
	connect(savePlaylistPushButton, SIGNAL(clicked(bool)), this, SLOT(savePlaylistPushButtonActivated()));
	
	connect(action_Rename_playlist, SIGNAL(triggered()), this, SLOT(renamePlaylistActivated()));
	
	connect(&mpd, SIGNAL(storedPlayListUpdated()), this, SLOT(updateStoredPlaylists()));
}

void MainWindow::addPlaylistToPlaylistPushButtonActivated()
{
	const QModelIndexList items = playlistsView->selectionModel()->selectedRows();

	if(items.size() == 1) {
		QModelIndex sourceIndex = playlistsProxyModel.mapToSource(items.first());
		QString playlist_name = playlistsModel.data(sourceIndex, Qt::DisplayRole).toString();
		playlistsModel.loadPlaylist(playlist_name);
	}
}

void MainWindow::loadPlaylistPushButtonActivated()
{
	const QModelIndexList items = playlistsView->selectionModel()->selectedRows();

	if(items.size() == 1) {
		mpd.clear();
		mpd.getStatus();
		searchPlaylistLineEdit->clear();
		QModelIndex sourceIndex = playlistsProxyModel.mapToSource(items.first());
		QString playlist_name = playlistsModel.data(sourceIndex, Qt::DisplayRole).toString();
		playlistsModel.loadPlaylist(playlist_name);
	}
}

void MainWindow::playlistsViewItemDoubleClicked(const QModelIndex &index)
{
	QModelIndex sourceIndex = playlistsProxyModel.mapToSource(index);
	QString playlist_name = playlistsModel.data(sourceIndex, Qt::DisplayRole).toString();
	playlistsModel.loadPlaylist(playlist_name);
}

void MainWindow::removePlaylistPushButtonActivated()
{
	const QModelIndexList items = playlistsView->selectionModel()->selectedRows();

	if(items.size() == 1) {
		QModelIndex sourceIndex = playlistsProxyModel.mapToSource(items.first());
		QString playlist_name = playlistsModel.data(sourceIndex, Qt::DisplayRole).toString();

		QMessageBox msgBox;
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.setWindowTitle("Delete playlist?");
		msgBox.setText("Are you sure you want to delete playlist: " + playlist_name + "?");
		msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
		msgBox.setDefaultButton(QMessageBox::No);
		int ret = msgBox.exec();
		
		if (ret == QMessageBox::Yes) {
			playlistsModel.removePlaylist(playlist_name);
		}
	}
}

void MainWindow::savePlaylistPushButtonActivated()
{
	QString name = QInputDialog::getText(
		this,
		"Enter name",
		"Name"
	);

	if(!name.isEmpty())
		playlistsModel.savePlaylist(name);
}

/*
 * Idle command has returnd that stored_playlist:
 * "a stored playlist has been modified, renamed, created or deleted"
 * Update accordingly
 *
 * TODO: Implement
 */
void MainWindow::updateStoredPlaylists()
{
	qDebug() << "Need to update playlist";
	playlistsModel.getPlaylists();
}

void MainWindow::renamePlaylistActivated()
{
	const QModelIndexList items = playlistsView->selectionModel()->selectedRows();

	if(items.size() == 1) {
		QModelIndex sourceIndex = playlistsProxyModel.mapToSource(items.first());
		QString playlist_name = playlistsModel.data(sourceIndex, Qt::DisplayRole).toString();

		QString new_name = QInputDialog::getText(
		this,
		"Rename playlist",
		"Enter new name for playlist: " + playlist_name
		);
		
		if (!new_name.isEmpty()) {
			playlistsModel.renamePlaylist(playlist_name, new_name);
		}
	}
}
