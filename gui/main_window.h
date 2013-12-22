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

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#ifdef ENABLE_KDE_SUPPORT
#include <KXmlGuiWindow>
#include <KStatusBar>
#else
#include <QMainWindow>
#endif

#include <QTimer>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QHeaderView>
#include <QStringList>
#include <QDateTime>
#include <QProxyStyle>
#include <QResizeEvent>
#include <QMoveEvent>

#ifdef ENABLE_KDE_SUPPORT
#include "ui_main_window_kde.h"
#else
#include "ui_main_window.h"
#endif

#include "gui/musiclibrarymodel.h"
#include "gui/musiclibraryproxymodel.h"
#include "gui/playlistsmodel.h"
#include "gui/playlistsproxymodel.h"
#include "gui/playlisttablemodel.h"
#include "gui/playlisttableproxymodel.h"
#include "gui/dirviewmodel.h"
#include "gui/dirviewproxymodel.h"
#include "lib/lastfm_metadata_fetcher.h"
#include "lib/mpdconnectionplayback.h"
#include "lib/mpddatabaseconnection.h"
#include "lib/output.h"

class KAction;
class MainWindow;

class VolumeSliderEventHandler : public QObject
{
	Q_OBJECT

	public:
		VolumeSliderEventHandler(MainWindow *w);

	protected:
		bool eventFilter(QObject *obj, QEvent *event);

	private:
		MainWindow * const window;
};

class SliderStyle : public QProxyStyle
{
public:
    SliderStyle(QStyle *style) : QProxyStyle(style) {}
    int styleHint(QStyle::StyleHint hint, const QStyleOption* option = 0, const QWidget* widget = 0, QStyleHintReturn* returnData = 0) const;
};

#ifdef ENABLE_KDE_SUPPORT
class MainWindow : public KXmlGuiWindow, private Ui::MainWindow
#else
class MainWindow : public QMainWindow, private Ui::MainWindow
#endif
{
	Q_OBJECT

	public:
		MainWindow(QWidget *parent = 0);

	protected:
		void closeEvent(QCloseEvent *event);
		void resizeEvent(QResizeEvent * event);
		void moveEvent(QMoveEvent * event);

	private:
		QSettings settings;
		MPDConnectionPlayback mpd;
		MPDDatabaseConnection mpdDb;
		MPDStatus::State lastState;
		quint32 songTime;
		qint32 lastSongId;
		quint32 lastPlaylist;
		QDateTime lastDbUpdate;
		int fetchStatsFactor;
		int nowPlayingFactor;
		PlaylistsModel playlistsModel;
		PlaylistsProxyModel playlistsProxyModel;
		PlaylistTableModel playlistModel;
		PlaylistTableProxyModel playlistProxyModel;
		dirViewModel dirviewModel;
		DirViewProxyModel dirProxyModel;
		MusicLibraryModel musicLibraryModel;
		MusicLibraryProxyModel libraryProxyModel;
		LastFmMetadataFetcher metadataFetcher;
		bool draggingPositionSlider;
		const QIcon icon;
		const QIcon icon_playing;
		const QIcon icon_paused;
		QIcon playbackStop;
		QIcon playbackNext;
		QIcon playbackPrev;
		QIcon playbackPause;
		QIcon playbackPlay;
		QHeaderView *playlistTableViewHeader;
		VolumeSliderEventHandler *volumeSliderEventHandler;
		QTimer elapsedTimer;
		bool setupTrayIcon();
		void setupPlaylistViewMenu();
		void setupPlaylistViewHeader();
#ifdef ENABLE_KDE_SUPPORT
		KAction *action_Prev_track;
		KAction *action_Next_track;
		KAction *action_Play_pause_track;
		KAction *action_Stop_track;
		KAction *action_Increase_volume;
		KAction *action_Decrease_volume;
		KAction *action_Add_to_playlist;
		KAction *action_Replace_playlist;
		KAction *action_Load_playlist;
		KAction *action_Remove_playlist;
		KAction *action_Remove_from_playlist;
		KAction *action_Clear_playlist;
		KAction *action_Copy_song_info;
		KAction *action_Rename_playlist;
		KAction *quitAction;
#else
		QAction *action_Prev_track;
		QAction *action_Next_track;
		QAction *action_Play_pause_track;
		QAction *action_Stop_track;
		QAction *action_Increase_volume;
		QAction *action_Decrease_volume;
		QAction *action_Add_to_playlist;
		QAction *action_Replace_playlist;
		QAction *action_Load_playlist;
		QAction *action_Remove_playlist;
		QAction *action_Remove_from_playlist;
		QAction *action_Clear_playlist;
		QAction *action_Copy_song_info;
		QAction *action_Rename_playlist;
		QAction *quitAction;
#endif
		QAction *albumPlaylistViewAction;
		QAction *artistPlaylistViewAction;
		QAction *titlePlaylistViewAction;
		QAction *timePlaylistViewAction;
		QAction *trackPlaylistViewAction;
		QAction *discPlaylistViewAction;
		QAction *yearPlaylistViewAction;
		
		QAction *action_Shuffle_entire_playlist;
		QAction *action_Shuffle_playlist_selection;
		QAction *action_Crop_playlist;

		QSystemTrayIcon *trayIcon;
		QMenu *trayIconMenu;
		QString toolTipText;
        QMenu *playlistTableViewMenu;

        // ---------- I don't understand why these are not inherited ----
        QPushButton *addPlaylistToPlaylistPushButton;
        QPushButton *loadPlaylistPushButton;
        QPushButton *removePlaylistPushButton;
        QPushButton *savePlaylistPushButton;
        QCheckBox *fetchInfoCheckBox;

        QCheckBox *consumeCheckBox;
        QLineEdit *searchLibraryLineEdit;
        QLineEdit *searchDirViewLineEdit;
        QLineEdit *searchPlaylistLineEdit;
        QPushButton *replacePlaylistPushButton;
        QPushButton *dirViewReplacePlaylistPushButton;
        QMenu *menu_Outputs;
        QPushButton *shufflePlaylistPushButton;
        QTreeView *playlistsView;

        //-----------------------

		void addSelectionToPlaylist();
		void addDirViewSelectionToPlaylist();
		QStringList walkDirView(QModelIndex rootItem);
		void setupPlaylists();
		
	private slots:
		int showPreferencesDialog(const int tab = 0);
		void mpdConnectionDied();
#ifndef ENABLE_KDE_SUPPORT
		void showAboutDialog();
#endif
		void showStatisticsDialog();
		void setAlbumCover(QImage, QString, QString);
		void setAlbumReleaseDate(QDate);
		void positionSliderPressed();
		void positionSliderReleased();
		void startPlayingSong();
		void nextTrack();
		void stopTrack();
		void playPauseTrack();
		void previousTrack();
		void setVolume(int vol);
		void increaseVolume();
		void decreaseVolume();
		void setPosition();
		void setConsume(const int);
		void setRandom(const int);
		void setRepeat(const int);
		void searchMusicLibrary();
		void musicLibarySearchFieldComboBoxChanged();
		void searchDirView();
		void searchPlaylist();
		void updatePlaylist(QList<Song *> *songs);
		void updateCurrentSong(const Song *song);
		void updateStats();
		void updateStatus();
		void updatePositionSilder();
		void playlistItemActivated(const QModelIndex &);
		void removeFromPlaylist();
		void clearPlaylist();
		void replacePlaylist();
		void libraryItemActivated(const QModelIndex &);
		void dirViewItemActivated(const QModelIndex &);
		void addToPlaylistButtonActivated();
		void addToPlaylistItemActivated();
		void dirViewAddToPlaylistPushButtonActivated();
		void trayIconClicked(QSystemTrayIcon::ActivationReason reason);
		void crossfadingChanged(const int seconds);
		void playlistTableViewContextMenuClicked();

		void saveSplitterState(int, int);

		void playListTableViewToggleAlbum(const bool visible);
		void playListTableViewToggleArtist(const bool visible);
		void playListTableViewToggleTrack(const bool visible);
		void playListTableViewToggleTime(const bool visible);
		void playListTableViewToggleTitle(const bool visible);
		void playListTableViewToggleDisc(const bool visible);
		void playListTableViewToggleYear(const bool visible);
		void playListTableViewSaveState();

		void updatePlayListStatus();

		void movePlaylistItems(const QList<quint32> items, const int row, const int size);
		void addFilenamesToPlaylist(const QStringList filenames, const int row, const int size);

		void addPlaylistToPlaylistPushButtonActivated();
		void loadPlaylistPushButtonActivated();
		void playlistsViewItemDoubleClicked(const QModelIndex &index);
		void removePlaylistPushButtonActivated();
		void savePlaylistPushButtonActivated();
		void renamePlaylistActivated();
		
		void updateStoredPlaylists();
		
		void copySongInfo();
		
		void updateOutpus(const QList<Output *> outputs);
		void outputChanged(QAction *action);
		
		void prepareShuffleMenu();
		void shuffleSelection();
		
		void cropPlaylist();

	friend class VolumeSliderEventHandler;
	friend class PlaylistEventHandler;
	friend class PlaylistsEventHandler;
};

#endif
