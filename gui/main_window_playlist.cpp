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

#include <QtGui>
#include <QIcon>
#include <QPixmap>
#include <QString>
#include <QKeyEvent>
#include <QHeaderView>
#include <QSet>

#include "main_window.h"

void MainWindow::setupPlaylistViewMenu()
{
	/*
	 * Setup shuffleMenu
	 */
	action_Shuffle_entire_playlist = new QAction(tr("Playlist"), this);
	action_Shuffle_playlist_selection = new QAction(tr("Selection"), this);

	QMenu *shuffleMenu = new QMenu(this);
	shuffleMenu->addAction(action_Shuffle_entire_playlist);
	shuffleMenu->addAction(action_Shuffle_playlist_selection);
	shufflePlaylistPushButton->setMenu(shuffleMenu);
	
	connect(shuffleMenu, SIGNAL(aboutToShow()), this, SLOT(prepareShuffleMenu()));
	connect(action_Shuffle_entire_playlist, SIGNAL(triggered()), &mpd, SLOT(shuffle()));
	connect(action_Shuffle_playlist_selection, SIGNAL(triggered()), this, SLOT(shuffleSelection()));
	
	QSettings settings;

	playlistTableViewMenu = new QMenu(this);

	albumPlaylistViewAction = new QAction(tr("Album"), playlistTableViewMenu);
	albumPlaylistViewAction->setCheckable(true);
	albumPlaylistViewAction->setChecked(true);
	artistPlaylistViewAction = new QAction(tr("Artist"), playlistTableViewMenu);
	artistPlaylistViewAction->setCheckable(true);
	artistPlaylistViewAction->setChecked(true);
	titlePlaylistViewAction = new QAction(tr("Title"), playlistTableViewMenu);
	titlePlaylistViewAction->setCheckable(true);
	titlePlaylistViewAction->setChecked(true);
	timePlaylistViewAction = new QAction(tr("Time"), playlistTableViewMenu);
	timePlaylistViewAction->setCheckable(true);
	timePlaylistViewAction->setChecked(true);
	trackPlaylistViewAction = new QAction(tr("Track"), playlistTableViewMenu);
	trackPlaylistViewAction->setCheckable(true);
	trackPlaylistViewAction->setChecked(true);
	discPlaylistViewAction = new QAction(tr("Disc"), playlistTableViewMenu);
	discPlaylistViewAction->setCheckable(true);
	discPlaylistViewAction->setChecked(true);
	yearPlaylistViewAction = new QAction(tr("Year"), playlistTableViewMenu);
	yearPlaylistViewAction->setCheckable(true);
	yearPlaylistViewAction->setChecked(true);

	playlistTableViewMenu->addAction(albumPlaylistViewAction);
	playlistTableViewMenu->addAction(artistPlaylistViewAction);
	playlistTableViewMenu->addAction(titlePlaylistViewAction);
	playlistTableViewMenu->addAction(timePlaylistViewAction);
	playlistTableViewMenu->addAction(trackPlaylistViewAction);
	playlistTableViewMenu->addAction(discPlaylistViewAction);
	playlistTableViewMenu->addAction(yearPlaylistViewAction);

	//Restore state
	QByteArray state = settings.value("playlistTableViewHeader").toByteArray();

	//Restore
	if (!state.isEmpty()) {
		playlistTableViewHeader->restoreState(state);

		int hidden = playlistTableViewHeader->hiddenSectionCount();

		if (playlistTableViewHeader->isSectionHidden(0) ||
			playlistTableViewHeader->sectionSize(0) == 0) {
			titlePlaylistViewAction->setChecked(false);
			playlistTableViewHeader->setSectionHidden(0, true);
			playlistTableViewHeader->resizeSection(0, 100);
		} else {
			if (hidden == 5) {
				titlePlaylistViewAction->setDisabled(true);
			}
		}

		if (playlistTableViewHeader->isSectionHidden(1) ||
			playlistTableViewHeader->sectionSize(1) == 0) {
			artistPlaylistViewAction->setChecked(false);
			playlistTableViewHeader->setSectionHidden(1, true);
			playlistTableViewHeader->resizeSection(1, 100);
		} else {
			if (hidden == 5) {
				artistPlaylistViewAction->setDisabled(true);
			}
		}

		if (playlistTableViewHeader->isSectionHidden(2) ||
			playlistTableViewHeader->sectionSize(2) == 0) {
			albumPlaylistViewAction->setChecked(false);
			playlistTableViewHeader->setSectionHidden(2, true);
			playlistTableViewHeader->resizeSection(2, 100);
		} else {
			if (hidden == 5) {
				albumPlaylistViewAction->setDisabled(true);
			}
		}

		if (playlistTableViewHeader->isSectionHidden(3) ||
			playlistTableViewHeader->sectionSize(3) == 0) {
			trackPlaylistViewAction->setChecked(false);
			playlistTableViewHeader->setSectionHidden(3, true);
			playlistTableViewHeader->resizeSection(3, 100);
		} else {
			if (hidden == 5) {
				trackPlaylistViewAction->setDisabled(true);
			}
		}

		if (playlistTableViewHeader->isSectionHidden(4) ||
			playlistTableViewHeader->sectionSize(4) == 0) {
			timePlaylistViewAction->setChecked(false);
			playlistTableViewHeader->setSectionHidden(4, true);
			playlistTableViewHeader->resizeSection(4, 100);
		} else {
			if (hidden == 5) {
				timePlaylistViewAction->setDisabled(true);
			}
		}

		if (playlistTableViewHeader->isSectionHidden(5) ||
			playlistTableViewHeader->sectionSize(5) == 0) {
			discPlaylistViewAction->setChecked(false);
			playlistTableViewHeader->setSectionHidden(5, true);
			playlistTableViewHeader->resizeSection(5, 100);
		} else {
			if (hidden == 5) {
				discPlaylistViewAction->setDisabled(true);
			}
		}
		if (playlistTableViewHeader->isSectionHidden(6) ||
			playlistTableViewHeader->sectionSize(6) == 0) {
			yearPlaylistViewAction->setChecked(false);
			playlistTableViewHeader->setSectionHidden(6, true);
			playlistTableViewHeader->resizeSection(6, 100);
		} else {
			if (hidden == 6) {
				yearPlaylistViewAction->setDisabled(true);
			}
		}
	}

	connect(albumPlaylistViewAction, SIGNAL(toggled(bool)), this, SLOT(playListTableViewToggleAlbum(bool)));
	connect(artistPlaylistViewAction, SIGNAL(toggled(bool)), this, SLOT(playListTableViewToggleArtist(bool)));
	connect(timePlaylistViewAction, SIGNAL(toggled(bool)), this, SLOT(playListTableViewToggleTime(bool)));
	connect(trackPlaylistViewAction, SIGNAL(toggled(bool)), this, SLOT(playListTableViewToggleTrack(bool)));
	connect(titlePlaylistViewAction, SIGNAL(toggled(bool)), this, SLOT(playListTableViewToggleTitle(bool)));
	connect(discPlaylistViewAction, SIGNAL(toggled(bool)), this, SLOT(playListTableViewToggleDisc(bool)));
	connect(yearPlaylistViewAction, SIGNAL(toggled(bool)), this, SLOT(playListTableViewToggleYear(bool)));
}

void MainWindow::setupPlaylistViewHeader() {
	playlistTableViewHeader = playlistTableView->horizontalHeader();

	playlistTableViewHeader->setMovable(true);
	playlistTableViewHeader->setResizeMode(QHeaderView::Interactive);
	playlistTableViewHeader->setContextMenuPolicy(Qt::CustomContextMenu);

	connect(playlistTableViewHeader, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(playlistTableViewContextMenuClicked()));
	connect(playlistTableViewHeader, SIGNAL(sectionMoved(int, int, int)), this, SLOT(playListTableViewSaveState()));
	connect(playlistTableViewHeader, SIGNAL(sectionResized(int, int, int)), this, SLOT(playListTableViewSaveState()));
}

void MainWindow::playlistTableViewContextMenuClicked()
{
	playlistTableViewMenu->exec(QCursor::pos());
}

void MainWindow::playListTableViewToggleAlbum(const bool visible)
{
	if (visible) {
		albumPlaylistViewAction->setEnabled(true);
		artistPlaylistViewAction->setEnabled(true);
		titlePlaylistViewAction->setEnabled(true);
		timePlaylistViewAction->setEnabled(true);
		trackPlaylistViewAction->setEnabled(true);
		discPlaylistViewAction->setEnabled(true);
		yearPlaylistViewAction->setEnabled(true);
	} else {
		if (playlistTableViewHeader->hiddenSectionCount() == 4) {
			if (artistPlaylistViewAction->isChecked())
				artistPlaylistViewAction->setDisabled(true);
			if (titlePlaylistViewAction->isChecked())
				titlePlaylistViewAction->setDisabled(true);
			if (timePlaylistViewAction->isChecked())
				timePlaylistViewAction->setDisabled(true);
			if (trackPlaylistViewAction->isChecked())
				trackPlaylistViewAction->setDisabled(true);
			if (discPlaylistViewAction->isChecked())
				discPlaylistViewAction->setDisabled(true);
			if (yearPlaylistViewAction->isChecked())
				yearPlaylistViewAction->setDisabled(true);
		}
	}

	playlistTableViewHeader->setSectionHidden(2, !visible);
}

void MainWindow::playListTableViewToggleArtist(const bool visible)
{
	if (visible) {
		albumPlaylistViewAction->setEnabled(true);
		artistPlaylistViewAction->setEnabled(true);
		titlePlaylistViewAction->setEnabled(true);
		timePlaylistViewAction->setEnabled(true);
		trackPlaylistViewAction->setEnabled(true);
		discPlaylistViewAction->setEnabled(true);
                yearPlaylistViewAction->setEnabled(true);
	} else {
		if (playlistTableViewHeader->hiddenSectionCount() == 4) {
			if (albumPlaylistViewAction->isChecked())
				albumPlaylistViewAction->setDisabled(true);
			if (titlePlaylistViewAction->isChecked())
				titlePlaylistViewAction->setDisabled(true);
			if (timePlaylistViewAction->isChecked())
				timePlaylistViewAction->setDisabled(true);
			if (trackPlaylistViewAction->isChecked())
				trackPlaylistViewAction->setDisabled(true);
			if (discPlaylistViewAction->isChecked())
				discPlaylistViewAction->setDisabled(true);
			if (yearPlaylistViewAction->isChecked())
				yearPlaylistViewAction->setDisabled(true);
		}
	}

	playlistTableViewHeader->setSectionHidden(1, !visible);
}

void MainWindow::playListTableViewToggleTime(const bool visible)
{
	if (visible) {
		albumPlaylistViewAction->setEnabled(true);
		artistPlaylistViewAction->setEnabled(true);
		titlePlaylistViewAction->setEnabled(true);
		timePlaylistViewAction->setEnabled(true);
		trackPlaylistViewAction->setEnabled(true);
		discPlaylistViewAction->setEnabled(true);
                yearPlaylistViewAction->setEnabled(true);
	} else {
		if (playlistTableViewHeader->hiddenSectionCount() == 4) {
			if (artistPlaylistViewAction->isChecked())
				artistPlaylistViewAction->setDisabled(true);
			if (titlePlaylistViewAction->isChecked())
				titlePlaylistViewAction->setDisabled(true);
			if (albumPlaylistViewAction->isChecked())
				albumPlaylistViewAction->setDisabled(true);
			if (trackPlaylistViewAction->isChecked())
				trackPlaylistViewAction->setDisabled(true);
			if (discPlaylistViewAction->isChecked())
				discPlaylistViewAction->setDisabled(true);
			if (yearPlaylistViewAction->isChecked())
				yearPlaylistViewAction->setDisabled(true);
		}
	}

	playlistTableViewHeader->setSectionHidden(4, !visible);
}

void MainWindow::playListTableViewToggleTrack(const bool visible)
{
	if (visible) {
		albumPlaylistViewAction->setEnabled(true);
		artistPlaylistViewAction->setEnabled(true);
		titlePlaylistViewAction->setEnabled(true);
		timePlaylistViewAction->setEnabled(true);
		trackPlaylistViewAction->setEnabled(true);
		discPlaylistViewAction->setEnabled(true);
                yearPlaylistViewAction->setEnabled(true);
	} else {
		if (playlistTableViewHeader->hiddenSectionCount() == 4) {
			if (artistPlaylistViewAction->isChecked())
				artistPlaylistViewAction->setDisabled(true);
			if (titlePlaylistViewAction->isChecked())
				titlePlaylistViewAction->setDisabled(true);
			if (timePlaylistViewAction->isChecked())
				timePlaylistViewAction->setDisabled(true);
			if (albumPlaylistViewAction->isChecked())
				albumPlaylistViewAction->setDisabled(true);
			if (discPlaylistViewAction->isChecked())
				discPlaylistViewAction->setDisabled(true);
			if (yearPlaylistViewAction->isChecked())
				yearPlaylistViewAction->setDisabled(true);
		}
	}

	playlistTableViewHeader->setSectionHidden(3, !visible);
}

void MainWindow::playListTableViewToggleTitle(const bool visible)
{
	if (visible) {
		albumPlaylistViewAction->setEnabled(true);
		artistPlaylistViewAction->setEnabled(true);
		titlePlaylistViewAction->setEnabled(true);
		timePlaylistViewAction->setEnabled(true);
		trackPlaylistViewAction->setEnabled(true);
		discPlaylistViewAction->setEnabled(true);
                yearPlaylistViewAction->setEnabled(true);
	} else {
		if (playlistTableViewHeader->hiddenSectionCount() == 4) {
			if (artistPlaylistViewAction->isChecked())
				artistPlaylistViewAction->setDisabled(true);
			if (albumPlaylistViewAction->isChecked())
				albumPlaylistViewAction->setDisabled(true);
			if (timePlaylistViewAction->isChecked())
				timePlaylistViewAction->setDisabled(true);
			if (trackPlaylistViewAction->isChecked())
				trackPlaylistViewAction->setDisabled(true);
			if (discPlaylistViewAction->isChecked())
				discPlaylistViewAction->setDisabled(true);
			if (yearPlaylistViewAction->isChecked())
				yearPlaylistViewAction->setDisabled(true);
		}
	}

	playlistTableViewHeader->setSectionHidden(0, !visible);
}

void MainWindow::playListTableViewToggleDisc(const bool visible)
{
	if (visible) {
		albumPlaylistViewAction->setEnabled(true);
		artistPlaylistViewAction->setEnabled(true);
		titlePlaylistViewAction->setEnabled(true);
		timePlaylistViewAction->setEnabled(true);
		trackPlaylistViewAction->setEnabled(true);
		discPlaylistViewAction->setEnabled(true);
                yearPlaylistViewAction->setEnabled(true);
	} else {
		if (playlistTableViewHeader->hiddenSectionCount() == 4) {
			if (artistPlaylistViewAction->isChecked())
				artistPlaylistViewAction->setDisabled(true);
			if (albumPlaylistViewAction->isChecked())
				albumPlaylistViewAction->setDisabled(true);
			if (timePlaylistViewAction->isChecked())
				timePlaylistViewAction->setDisabled(true);
			if (trackPlaylistViewAction->isChecked())
				trackPlaylistViewAction->setDisabled(true);
			if (titlePlaylistViewAction->isChecked())
				titlePlaylistViewAction->setDisabled(true);
			if (yearPlaylistViewAction->isChecked())
				yearPlaylistViewAction->setDisabled(true);
		}
	}
	
	playlistTableViewHeader->setSectionHidden(5, !visible);
}

void MainWindow::playListTableViewToggleYear(const bool visible)
{
	if (visible) {
		albumPlaylistViewAction->setEnabled(true);
		artistPlaylistViewAction->setEnabled(true);
		titlePlaylistViewAction->setEnabled(true);
		timePlaylistViewAction->setEnabled(true);
		trackPlaylistViewAction->setEnabled(true);
		discPlaylistViewAction->setEnabled(true);
		yearPlaylistViewAction->setEnabled(true);
	} else {
		if (playlistTableViewHeader->hiddenSectionCount() == 4) {
			if (artistPlaylistViewAction->isChecked())
				artistPlaylistViewAction->setDisabled(true);
			if (albumPlaylistViewAction->isChecked())
				albumPlaylistViewAction->setDisabled(true);
			if (timePlaylistViewAction->isChecked())
				timePlaylistViewAction->setDisabled(true);
			if (trackPlaylistViewAction->isChecked())
				trackPlaylistViewAction->setDisabled(true);
			if (titlePlaylistViewAction->isChecked())
				titlePlaylistViewAction->setDisabled(true);
			if (discPlaylistViewAction->isChecked())
				titlePlaylistViewAction->setDisabled(true);
		}
	}

	playlistTableViewHeader->setSectionHidden(6, !visible);
}

void MainWindow::playListTableViewSaveState()
{
	QSettings settings;
	settings.setValue("playlistTableViewHeader", playlistTableViewHeader->saveState());
}

void MainWindow::prepareShuffleMenu()
{
	const QModelIndexList items = playlistTableView->selectionModel()->selectedRows();
	QModelIndex sourceIndex;
	quint32 first_id;
	quint32 last_id;

	/*
	 * We can't shuffle a selection if there is no selection
	 */
	if(items.isEmpty()) {
		action_Shuffle_playlist_selection->setEnabled(false);
		return;
	}

	sourceIndex = playlistProxyModel.mapToSource(items.at(0));
	first_id = sourceIndex.row();
	last_id = sourceIndex.row();

	for(int i = 1; i < items.size(); i++) {
		sourceIndex = playlistProxyModel.mapToSource(items.at(i));
		quint32 cur_id = sourceIndex.row();
		
		first_id = first_id < cur_id ? first_id : cur_id;
		last_id = last_id > cur_id ? last_id : cur_id;
	}

	/*
	 * Check if the selection is a range
	 */
	if ( ((last_id - first_id) + 1) == (quint32)items.size()) {
		action_Shuffle_playlist_selection->setEnabled(true);
	} else {
		action_Shuffle_playlist_selection->setEnabled(false);
	}
}

void MainWindow::shuffleSelection()
{
	const QModelIndexList items = playlistTableView->selectionModel()->selectedRows();
	QModelIndex sourceIndex;
	quint32 first_id;
	quint32 last_id;

	/*
	 * We can't shuffle a selection if there is no selection
	 */
	if(items.isEmpty()) {
		return;
	}

	sourceIndex = playlistProxyModel.mapToSource(items.at(0));
	first_id = sourceIndex.row();
	last_id = sourceIndex.row();

	for(int i = 1; i < items.size(); i++) {
		quint32 cur_id = playlistProxyModel.mapToSource(items.at(i)).row();

		first_id = first_id < cur_id ? first_id : cur_id;
		last_id = last_id > cur_id ? last_id : cur_id;
	}

	/*
	 * Check if the selection is a range
	 */
	if ( ((last_id - first_id) + 1) == (quint32)items.size()) {
		mpd.shuffle(first_id, last_id);
	} 
}

/*
 * Crop playlist
 * Do this by taking the set off all song id's and subtracting from that
 * the set of selected song id's. Feed that list to mpd.removeSongs
 */
void MainWindow::cropPlaylist()
{
	QSet<qint32> songs = playlistModel.getSongIdSet();
	QSet<qint32> selected;
	
	const QModelIndexList items = playlistTableView->selectionModel()->selectedRows();
	QModelIndex sourceIndex;

	if(items.isEmpty())
		return;

	for(int i = 0; i < items.size(); i++) {
		sourceIndex = playlistProxyModel.mapToSource(items.at(i));
		selected << playlistModel.getIdByRow(sourceIndex.row());
	}
	
	QList<qint32> toBeRemoved = (songs - selected).toList();

	mpd.removeSongs(toBeRemoved);
}
