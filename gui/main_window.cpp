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
#include <QSet>
#include <QSettings>
#include <QString>
#include <QResizeEvent>
#include <QMoveEvent>
#include <QClipboard>
#include <cstdlib>

#ifdef ENABLE_KDE_SUPPORT
#include <KApplication>
#include <KAction>
#include <KIcon>
#include <KLocale>
#include <KActionCollection>
#include <KNotification>
#include <KStandardAction>
#include <kaboutapplicationdialog.h>
#include <klineedit.h>
#include <kxmlguifactory.h>
#endif

#include "main_window.h"
#include "musiclibraryitemsong.h"
#include "preferences_dialog.h"
#include "about_dialog.h"
#include "statistics_dialog.h"
#include "lib/mpdstats.h"
#include "lib/mpdstatus.h"
#include "lib/mpdparseutils.h"
#include <QMessageBox>

VolumeSliderEventHandler::VolumeSliderEventHandler(MainWindow *w)
               : QObject(w), window(w)
{
}

bool VolumeSliderEventHandler::eventFilter(QObject *obj, QEvent *event)
{
	if(event->type() == QEvent::Wheel) {
		int numDegrees = static_cast<QWheelEvent *>(event)->delta() / 8;
		int numSteps = numDegrees / 15;
		if(numSteps>0) {
			for(int i=0;i<numSteps;++i)
				window->action_Increase_volume->trigger();
		} else {
			for(int i=0;i>numSteps;--i)
				window->action_Decrease_volume->trigger();
		}
		return true;
	}

	return QObject::eventFilter(obj, event);
}

int SliderStyle::styleHint(QStyle::StyleHint hint, const QStyleOption* option, const QWidget* widget, QStyleHintReturn* returnData) const
{
    if (hint == QStyle::SH_Slider_AbsoluteSetButtons)
        return (Qt::LeftButton | Qt::MidButton | Qt::RightButton);
    return QProxyStyle::styleHint(hint, option, widget, returnData);
}

#ifdef ENABLE_KDE_SUPPORT
MainWindow::MainWindow(QWidget *parent) : KXmlGuiWindow(parent),
#else
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
#endif
	lastState(MPDStatus::State_Inactive),
	lastSongId(-1),
	fetchStatsFactor(0),
	nowPlayingFactor(0),
	draggingPositionSlider(false),
	icon(QIcon(":/images/icon.svg")),
	icon_playing(QIcon(":/images/icon_playing.svg")),
    icon_paused(QIcon(":/images/icon_paused.svg")),
    addPlaylistToPlaylistPushButton(new QPushButton(this)),
    loadPlaylistPushButton(new QPushButton(this)),
    removePlaylistPushButton(new QPushButton(this)),
    savePlaylistPushButton(new QPushButton(this)),
    fetchInfoCheckBox(new QCheckBox(this)),
    consumeCheckBox(new QCheckBox(this)),
    searchLibraryLineEdit(new QLineEdit(this)),
    searchDirViewLineEdit(new QLineEdit(this)),
    searchPlaylistLineEdit(new QLineEdit(this)),
    replacePlaylistPushButton(new QPushButton(this)),
    dirViewReplacePlaylistPushButton(new QPushButton(this)),
    menu_Outputs(new QMenu(this)),
    shufflePlaylistPushButton(new QPushButton(this)),
    playlistsView(new QTreeView(this))
{		
#ifdef ENABLE_KDE_SUPPORT
	QWidget *widget = new QWidget(this);
	setupUi(widget);
	setCentralWidget(widget);

	KStandardAction::quit(kapp, SLOT(quit()), actionCollection());
	KStandardAction::preferences(this, SLOT(showPreferencesDialog()), actionCollection());

	KAction *action_Update_database = new KAction(this);
	action_Update_database->setText(i18n("Update database"));
	actionCollection()->addAction("updatedatabase", action_Update_database);

	KAction *action_Show_statistics = new KAction(this);
	action_Show_statistics->setText(i18n("Show statistics"));
	actionCollection()->addAction("showstatistics", action_Show_statistics);
       
	action_Prev_track = new KAction(this);
	action_Prev_track->setText(i18n("Previous track"));
	actionCollection()->addAction("prevtrack", action_Prev_track);
	action_Prev_track->setGlobalShortcut(KShortcut(Qt::META+Qt::Key_Left));

	action_Next_track = new KAction(this);
	action_Next_track->setText(i18n("Next track"));
	actionCollection()->addAction("nexttrack", action_Next_track);
	action_Next_track->setGlobalShortcut(KShortcut(Qt::META+Qt::Key_Right));

	action_Play_pause_track = new KAction(this);
	action_Play_pause_track->setText(i18n("Play/pause"));
	actionCollection()->addAction("playpausetrack", action_Play_pause_track);
	action_Play_pause_track->setGlobalShortcut(KShortcut(Qt::META+Qt::Key_C));

	action_Stop_track = new KAction(this);
	action_Stop_track->setText(i18n("Stop playing"));
	actionCollection()->addAction("stoptrack", action_Stop_track);
	action_Stop_track->setGlobalShortcut(KShortcut(Qt::META+Qt::Key_X));

	action_Increase_volume = new KAction(this);
	action_Increase_volume->setText(i18n("Increase volume"));
	actionCollection()->addAction("increasevolume", action_Increase_volume);
	action_Increase_volume->setGlobalShortcut(KShortcut(Qt::META+Qt::Key_Up));

	action_Decrease_volume = new KAction(this);
	action_Decrease_volume->setText(i18n("Decrease volume"));
	actionCollection()->addAction("decreasevolume", action_Decrease_volume);
	action_Decrease_volume->setGlobalShortcut(KShortcut(Qt::META+Qt::Key_Down));

	action_Add_to_playlist = new KAction(this);
	action_Add_to_playlist->setText(i18n("Add to Playlist"));
	action_Add_to_playlist->setIcon(KIcon("list-add"));
	actionCollection()->addAction("addtoplaylist", action_Add_to_playlist);

	action_Replace_playlist = new KAction(this);
	action_Replace_playlist->setText(i18n("Replace Playlist"));
	action_Replace_playlist->setIcon(KIcon("document-open"));
	actionCollection()->addAction("replaceplaylist", action_Replace_playlist);

	action_Load_playlist = new KAction(this);
	action_Load_playlist->setText(i18n("Load Playlist"));
	action_Load_playlist->setIcon(KIcon("document-open"));
	actionCollection()->addAction("loadplaylist", action_Load_playlist);

	action_Remove_playlist = new KAction(this);
	action_Remove_playlist->setText(i18n("Delete Playlist"));
	action_Remove_playlist->setIcon(KIcon("list-remove"));
	actionCollection()->addAction("removeplaylist", action_Remove_playlist);

	action_Remove_from_playlist = new KAction(this);
	action_Remove_from_playlist->setText(i18n("Remove"));
	action_Remove_from_playlist->setIcon(KIcon("list-remove"));
	action_Remove_from_playlist->setShortcut(QKeySequence::Delete);
	actionCollection()->addAction("removefromplaylist", action_Remove_from_playlist);

	action_Clear_playlist = new KAction(this);
	action_Clear_playlist->setText(i18n("Clear Playlist"));
	action_Clear_playlist->setIcon(KIcon("edit-clear-list"));
	actionCollection()->addAction("clearplaylist", action_Clear_playlist);
	
	action_Copy_song_info = new KAction(this);
	action_Copy_song_info->setText(i18n("Copy song info"));
	actionCollection()->addAction("copysonginfo", action_Copy_song_info);
	
	action_Rename_playlist = new KAction(this);
	action_Rename_playlist->setText(i18n("Rename playlist"));
	actionCollection()->addAction("renameplaylist", action_Rename_playlist);
	
	setupGUI();
#else
	action_Prev_track = new QAction(tr("Previous track"), this);
	action_Next_track = new QAction(tr("Next track"), this);
	action_Play_pause_track = new QAction(tr("Play/pause current track"), this);
	action_Stop_track = new QAction(tr("Stop playing"), this);
	action_Increase_volume = new QAction(tr("Increase volume"), this);
	action_Decrease_volume = new QAction(tr("Decrease volume"), this);
	action_Add_to_playlist = new QAction(tr("Add to Playlist"), this);
	action_Replace_playlist = new QAction(tr("Replace Playlist"), this);
	action_Load_playlist = new QAction(tr("Load"), this);
	action_Remove_playlist = new QAction(tr("Remove"), this);
	action_Remove_from_playlist = new QAction(tr("Remove"), this);
	action_Remove_from_playlist->setShortcut(QKeySequence::Delete);
	action_Clear_playlist = new QAction(tr("Clear Playlist"), this);
	action_Copy_song_info = new QAction(tr("Copy song info"), this);
	action_Copy_song_info->setShortcuts(QKeySequence::Copy);
	action_Rename_playlist = new QAction(tr("Rename Playlist"), this);
      
	setupUi(this);
#endif
	
	action_Crop_playlist = new QAction(tr("Crop playlist"), this);

	/*
	 * Restore size and position of GUI
	 * If we have a config variable for them
	 */
	if (settings.contains("size")) {
		resize(settings.value("size").toSize());
	}
	if (settings.contains("position")) {
		move(settings.value("position").toPoint());
	}
	/*
	 * Restore music library search field
	 * If we have a config variable for them
	 */
	if (settings.contains("musiclibrary/searchfieldindex")) {
		searchFieldComboBox->setCurrentIndex(settings.value("musiclibrary/searchfieldindex").toInt());
	}

	setVisible(true);

	// Setup event handler for volume adjustment
	volumeSliderEventHandler = new VolumeSliderEventHandler(this);

// KDE does this for us
#ifndef ENABLE_KDE_SUPPORT
	this->setWindowIcon(icon);
#endif

#ifdef ENABLE_KDE_SUPPORT
	playbackNext = KIcon("media-skip-forward");
	playbackPrev = KIcon("media-skip-backward");
	playbackPlay = KIcon("media-playback-start");
	playbackPause = KIcon("media-playback-pause");
	playbackStop = KIcon("media-playback-stop");
	QIcon increaseVolume = KIcon("audio-volume-control");
	QIcon decreaseVolume = KIcon("audio-volume-control");

	addToPlaylistPushButton->setIcon(KIcon("list-add"));
	replacePlaylistPushButton->setIcon(KIcon("document-open"));
	dirViewAddToPlaylistPushButton->setIcon(KIcon("list-add"));
	dirViewReplacePlaylistPushButton->setIcon(KIcon("document-open"));

	savePlaylistPushButton->setIcon(KIcon("document-save-as"));
	removeAllFromPlaylistPushButton->setIcon(KIcon("edit-clear-list"));
	removeFromPlaylistPushButton->setIcon(KIcon("list-remove"));

	addPlaylistToPlaylistPushButton->setIcon(KIcon("list-add"));
	loadPlaylistPushButton->setIcon(KIcon("document-open"));
	removePlaylistPushButton->setIcon(KIcon("list-remove"));

#else
	QCommonStyle style;
	playbackStop = style.standardIcon(QStyle::SP_MediaStop);
	playbackNext = style.standardIcon(QStyle::SP_MediaSkipForward);
	playbackPrev = style.standardIcon(QStyle::SP_MediaSkipBackward);
	playbackPlay = style.standardIcon(QStyle::SP_MediaPlay);
	playbackPause = style.standardIcon(QStyle::SP_MediaPause);
	QIcon increaseVolume = style.standardIcon(QStyle::SP_MediaVolume);
	QIcon decreaseVolume = style.standardIcon(QStyle::SP_MediaVolume);
#endif
	playPauseTrackButton->setDefaultAction(action_Play_pause_track);
	stopTrackButton->setDefaultAction(action_Stop_track);
	nextTrackButton->setDefaultAction(action_Next_track);
	prevTrackButton->setDefaultAction(action_Prev_track);

	action_Play_pause_track->setIcon(playbackPlay);
	action_Stop_track->setIcon(playbackStop);
	action_Next_track->setIcon(playbackNext);
	action_Prev_track->setIcon(playbackPrev);
	action_Increase_volume->setIcon(increaseVolume);
	action_Decrease_volume->setIcon(decreaseVolume);

	// Tray stuff
	if (setupTrayIcon() && settings.value("systemtray").toBool())
		trayIcon->show();

	// Playlists
	setupPlaylists();

	// Start connection threads
	mpd.start();
	mpdDb.start();

	// Set connection data
	mpd.setHostname(settings.value("connection/host").toString());
	mpd.setPort(settings.value("connection/port").toInt());
	mpd.setPassword(settings.value("connection/password").toString());
	mpdDb.setHostname(settings.value("connection/host").toString());
	mpdDb.setPort(settings.value("connection/port").toInt());
	mpdDb.setPassword(settings.value("connection/password").toString());

	while(!mpd.connectToMPD()) {
		if(!showPreferencesDialog())
			exit(EXIT_FAILURE);

		mpd.setHostname(settings.value("connection/host").toString());
		mpd.setPort(settings.value("connection/port").toInt());
		mpd.setPassword(settings.value("connection/password").toString());
		mpdDb.setHostname(settings.value("connection/host").toString());
		mpdDb.setPort(settings.value("connection/port").toInt());
		mpdDb.setPassword(settings.value("connection/password").toString());

		qWarning("Retrying");
	}

	playlistsModel.connectMpd(
		settings.value("connection/host").toString(),
		settings.value("connection/port").toInt(),
		settings.value("connection/password").toString()
	);

        // Slider stuff
	volumeSlider->installEventFilter(volumeSliderEventHandler);
	volumeSlider->setStyle(new SliderStyle(volumeSlider->style()));
	positionSlider->setStyle(new SliderStyle(positionSlider->style()));

	libraryProxyModel.setSourceModel(&musicLibraryModel);
	libraryTreeView->setModel(&libraryProxyModel);
	libraryTreeView->sortByColumn(0, Qt::AscendingOrder);
	libraryTreeView->setDragEnabled(true);
	libraryTreeView->setContextMenuPolicy(Qt::ActionsContextMenu);
	libraryTreeView->addAction(action_Add_to_playlist);
	libraryTreeView->addAction(action_Replace_playlist);

	dirProxyModel.setSourceModel(&dirviewModel);
	dirTreeView->setModel(&dirProxyModel);
	dirTreeView->sortByColumn(0, Qt::AscendingOrder);
	dirTreeView->setDragEnabled(true);
	dirTreeView->setContextMenuPolicy(Qt::ActionsContextMenu);
	dirTreeView->addAction(action_Add_to_playlist);
	dirTreeView->addAction(action_Replace_playlist);

	// Playlists model
	connect(&playlistsModel, SIGNAL(playlistLoaded()), this, SLOT(startPlayingSong()));

	// Playlist
	connect(&playlistModel, SIGNAL(filesAddedInPlaylist(const QStringList, const int, const int)), this,
	        SLOT(addFilenamesToPlaylist(const QStringList, const int, const int)));
	connect(&playlistModel, SIGNAL(moveInPlaylist(const QList<quint32>, const int, const int)), this,
	        SLOT(movePlaylistItems(const QList<quint32>, const int, const int)));
	connect(&playlistModel, SIGNAL(playListStatsUpdated()), this,
		SLOT(updatePlayListStatus()));

	// Playlist table view
	
	playlistProxyModel.setSourceModel(&playlistModel);
	playlistTableView->setModel(&playlistProxyModel);
	playlistTableView->verticalHeader()->hide();
	playlistTableView->verticalHeader()->setDefaultSectionSize(20); //sets default rowsize for tableview, not the best chosen location for that though
	playlistTableView->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
	playlistTableView->setDragEnabled(true);
	playlistTableView->setAcceptDrops(true);
	playlistTableView->setDropIndicatorShown(true);
	playlistTableView->setContextMenuPolicy(Qt::ActionsContextMenu);
	playlistTableView->addAction(action_Remove_from_playlist);
	playlistTableView->addAction(action_Clear_playlist);
	playlistTableView->addAction(action_Copy_song_info);
	playlistTableView->addAction(action_Crop_playlist);


	// PlaylistView
	setupPlaylistViewHeader();
	setupPlaylistViewMenu();

	// MPD
	connect(&mpd, SIGNAL(statsUpdated()), this, SLOT(updateStats()));
	connect(&mpd, SIGNAL(statusUpdated()), this, SLOT(updateStatus()), Qt::DirectConnection);
	connect(&mpd, SIGNAL(playlistUpdated(QList<Song *> *)), this, SLOT(updatePlaylist(QList<Song *> *)));
	connect(&mpd, SIGNAL(currentSongUpdated(const Song *)), this, SLOT(updateCurrentSong(const Song *)));
	connect(&mpd, SIGNAL(mpdConnectionDied()), this, SLOT(mpdConnectionDied()));
	connect(&mpdDb, SIGNAL(musicLibraryUpdated(MusicLibraryItemRoot *, QDateTime)), &musicLibraryModel, SLOT(updateMusicLibrary(MusicLibraryItemRoot *, QDateTime)));
	connect(&mpdDb, SIGNAL(dirViewUpdated(DirViewItemRoot *)), &dirviewModel, SLOT(updateDirView(DirViewItemRoot *)));

	// Metadata
	connect(&metadataFetcher, SIGNAL(coverImage(QImage, QString, QString)), this, SLOT(setAlbumCover(QImage, QString, QString)));
	connect(&metadataFetcher, SIGNAL(releaseDate(QDate)), this, SLOT(setAlbumReleaseDate(QDate)));

	// GUI
#ifndef ENABLE_KDE_SUPPORT
	connect(action_Quit, SIGNAL(triggered()), qApp, SLOT(quit()));
	connect(action_Preferences, SIGNAL(triggered(bool)), this, SLOT(showPreferencesDialog()));
	connect(action_About, SIGNAL(triggered(bool)), this, SLOT(showAboutDialog()));
#endif
	connect(action_Update_database, SIGNAL(triggered(bool)), &mpdDb, SLOT(update()));
	connect(action_Show_statistics, SIGNAL(triggered(bool)), this, SLOT(showStatisticsDialog()));
       
	connect(action_Prev_track, SIGNAL(triggered(bool)), this, SLOT(previousTrack()));
	connect(action_Next_track, SIGNAL(triggered(bool)), this, SLOT(nextTrack()));
	connect(action_Play_pause_track, SIGNAL(triggered(bool)), this, SLOT(playPauseTrack()));
	connect(action_Stop_track, SIGNAL(triggered(bool)), this, SLOT(stopTrack()));

	connect(action_Increase_volume, SIGNAL(triggered(bool)), this, SLOT(increaseVolume()));
	connect(action_Decrease_volume, SIGNAL(triggered(bool)), this, SLOT(decreaseVolume()));
       
	connect(splitter, SIGNAL(splitterMoved(int, int)), this, SLOT(saveSplitterState(int, int)));
	connect(volumeSlider, SIGNAL(valueChanged(int)), this, SLOT(setVolume(int)));
	connect(positionSlider, SIGNAL(sliderPressed()), this, SLOT(positionSliderPressed()));
	connect(positionSlider, SIGNAL(sliderReleased()), this, SLOT(setPosition()));
	connect(positionSlider, SIGNAL(sliderReleased()), this, SLOT(positionSliderReleased()));
	connect(consumeCheckBox, SIGNAL(stateChanged(int)), this, SLOT(setConsume(int)));
	connect(randomCheckBox, SIGNAL(stateChanged(int)), this, SLOT(setRandom(int)));
	connect(repeatCheckBox, SIGNAL(stateChanged(int)), this, SLOT(setRepeat(int)));
	connect(searchFieldComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(searchMusicLibrary()));
	connect(searchFieldComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(musicLibarySearchFieldComboBoxChanged()));
	connect(searchLibraryLineEdit, SIGNAL(returnPressed()), this, SLOT(searchMusicLibrary()));
	connect(searchLibraryLineEdit, SIGNAL(textChanged(const QString)), this, SLOT(searchMusicLibrary()));
	connect(searchDirViewLineEdit, SIGNAL(returnPressed()), this, SLOT(searchDirView()));
	connect(searchDirViewLineEdit, SIGNAL(textChanged(const QString)), this, SLOT(searchDirView()));
	connect(searchPlaylistLineEdit, SIGNAL(returnPressed()), this, SLOT(searchPlaylist()));
	connect(searchPlaylistLineEdit, SIGNAL(textChanged(const QString)), this, SLOT(searchPlaylist()));
	connect(playlistTableView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(playlistItemActivated(const QModelIndex &)));
	connect(removeAllFromPlaylistPushButton, SIGNAL(clicked(bool)), this, SLOT(clearPlaylist()));
	connect(removeFromPlaylistPushButton, SIGNAL(clicked(bool)), this, SLOT(removeFromPlaylist()));
	connect(libraryTreeView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(libraryItemActivated(const QModelIndex &)));
	connect(dirTreeView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(dirViewItemActivated(const QModelIndex &)));
	connect(addToPlaylistPushButton, SIGNAL(clicked(bool)), this, SLOT(addToPlaylistButtonActivated()));
	connect(replacePlaylistPushButton, SIGNAL(clicked(bool)), this, SLOT(replacePlaylist()));
	connect(dirViewAddToPlaylistPushButton, SIGNAL(clicked(bool)), this, SLOT(dirViewAddToPlaylistPushButtonActivated()));
	connect(dirViewReplacePlaylistPushButton, SIGNAL(clicked(bool)), this, SLOT(replacePlaylist()));	

	connect(action_Add_to_playlist, SIGNAL(activated()), this, SLOT(addToPlaylistItemActivated()));
	connect(action_Replace_playlist, SIGNAL(activated()), this, SLOT(replacePlaylist()));
	connect(action_Load_playlist, SIGNAL(activated()), this, SLOT(loadPlaylistPushButtonActivated()));
	connect(action_Remove_playlist, SIGNAL(activated()), this, SLOT(removePlaylistPushButtonActivated()));
	connect(action_Remove_from_playlist, SIGNAL(activated()), this, SLOT(removeFromPlaylist()));
	connect(action_Clear_playlist, SIGNAL(activated()), this, SLOT(clearPlaylist()));
	connect(action_Copy_song_info, SIGNAL(activated()), this, SLOT(copySongInfo()));
	connect(action_Crop_playlist, SIGNAL(activated()), this, SLOT(cropPlaylist()));
	
	// Position slider update
	connect(&elapsedTimer, SIGNAL(timeout()), this, SLOT(updatePositionSilder()));
	elapsedTimer.setInterval(1000);

	splitter->restoreState(settings.value("splitterSizes").toByteArray());
	
	// Outputs
	connect(&mpd, SIGNAL(outputsUpdated(const QList<Output *>)), this, SLOT(updateOutpus(const QList<Output *>)));
	#ifdef ENABLE_KDE_SUPPORT
		QMenu *m = (QMenu *)factory()->container("output_menu", this);
		connect(m, SIGNAL(triggered(QAction *)), this, SLOT(outputChanged(QAction *)));
	#else
		connect(menu_Outputs, SIGNAL(triggered(QAction *)), this, SLOT(outputChanged(QAction *)));
	#endif

	mpd.outputs();
	mpd.getStatus();
	mpd.getStats();

	/* Check if we need to get a new db or not */
	if (!musicLibraryModel.fromXML(MPDStats::getInstance()->dbUpdate()))
		mpdDb.listAllInfo(MPDStats::getInstance()->dbUpdate());

	mpd.playListInfo();

	mpdDb.listAll();

	playlistsModel.getPlaylists();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if (trayIcon != NULL && trayIcon->isVisible())
		hide();
	else
		qApp->quit();

	event->ignore();
}

/*
 * Implemented event handler (from QWidget) for resize event
 */
void MainWindow::resizeEvent(QResizeEvent * event)
{
	settings.setValue("size", event->size());
	QSize s = event->size();

	/*
	 * Also update position (since this may have changed as well
	 */
	const QPoint p = pos();
	settings.setValue("position", p);
}

/*
 * Implemented event handler (from QWidget) for move event
 */
void MainWindow::moveEvent(QMoveEvent * event)
{
	settings.setValue("position", event->pos());
}

void MainWindow::mpdConnectionDied()
{
	// Disconnect all threads
	mpdDb.disconnectFromMPD();
	// TODO Make sure we don't use the other connections anymore (playlists, etc.)
	
	// Stop timer(s)
	elapsedTimer.stop();

	// Reset GUI
	positionSlider->setValue(0);
	songTimeElapsedLabel->setText("00:00 / 00:00");

#ifndef ENABLE_KDE_SUPPORT
	action_Play_pause_track->setText("&Play");
#endif
	action_Play_pause_track->setIcon(playbackPlay);
	action_Play_pause_track->setEnabled(true);
	action_Stop_track->setEnabled(false);

	trackTitleLabel->setText("");
	trackArtistLabel->setText("");
	trackAlbumLabel->setText("");
	albumCoverLabel->setPixmap(QPixmap());

	// Show warning message
	QMessageBox::critical(this, "MPD connection gone", "The MPD connection died unexpectedly. This error is unrecoverable, please restart QtMPC.");
}

int MainWindow::showPreferencesDialog(const int tab)
{
	PreferencesDialog pref(tab, this);
	if (mpd.isConnected()) {
		if (trayIcon != NULL)
			connect(&pref, SIGNAL(systemTraySet(bool)), trayIcon, SLOT(setVisible(bool)));
		connect(&pref, SIGNAL(crossfadingChanged(const int)), this, SLOT(crossfadingChanged(const int)));
	}

	return pref.exec();
}

#ifndef ENABLE_KDE_SUPPORT
void MainWindow::showAboutDialog()
{
	AboutDialog about(this);
	about.exec();
}
#endif

void MainWindow::showStatisticsDialog()
{
	mpd.getStats();
	StatisticsDialog stats(this);
	stats.exec();
}

void MainWindow::setAlbumCover(QImage img, QString artist, QString album)
{
	if(img.isNull()) {
		albumCoverLabel->setText("No cover available");
		return;
	}

	// Display image
	QPixmap pixmap = QPixmap::fromImage(img);
	pixmap = pixmap.scaled(QSize(150, 150), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	albumCoverLabel->setPixmap(pixmap);

	// Save image to avoid downloading it next time
	QDir dir(QDir::home());
	if(!dir.exists(".QtMPC")) {
		if(!dir.mkdir(".QtMPC")) {
			qWarning("Couldn't create directory for storing album covers");
			return;
		}
	}

	dir.cd(".QtMPC");
	QString file(QFile::encodeName(artist + " - " + album + ".jpg"));
	img.save(dir.absolutePath() + QDir::separator() + file);
}

void MainWindow::setAlbumReleaseDate(QDate date)
{
	if (date.isValid())
		releaseDateLabel->setText(date.toString("d MMMM yyyy"));
}

void MainWindow::positionSliderPressed()
{
	draggingPositionSlider = true;
}

void MainWindow::positionSliderReleased()
{
	draggingPositionSlider = false;
}

void MainWindow::startPlayingSong()
{
	mpd.startPlayingSong();
}

void MainWindow::nextTrack()
{
	mpd.goToNext();
}

void MainWindow::stopTrack()
{
	mpd.stopPlaying();

       action_Stop_track->setEnabled(false);
#ifndef ENABLE_KDE_SUPPORT
       action_Play_pause_track->setText("&Play");
#endif
}

void MainWindow::playPauseTrack()
{
	MPDStatus * const status = MPDStatus::getInstance();

	if(status->state() == MPDStatus::State_Playing)
		mpd.setPause(true);
	else if(status->state() == MPDStatus::State_Paused)
		mpd.setPause(false);
	else
		mpd.startPlayingSong();

}

void MainWindow::previousTrack()
{
	mpd.goToPrevious();
}

void MainWindow::setPosition()
{
	mpd.setSeekId(MPDStatus::getInstance()->songId(), positionSlider->value());
}

void MainWindow::setVolume(int vol)
{
	mpd.setVolume(vol);
}

void MainWindow::increaseVolume()
{
	volumeSlider->triggerAction(QAbstractSlider::SliderPageStepAdd);
}

void MainWindow::decreaseVolume()
{
	volumeSlider->triggerAction(QAbstractSlider::SliderPageStepSub);
}

void MainWindow::setConsume(const int state)
{
	if(state == Qt::Checked)
		mpd.setConsume(true);
	else
		mpd.setConsume(false);
}

void MainWindow::setRandom(const int state)
{
	if(state == Qt::Checked)
		mpd.setRandom(true);
	else
		mpd.setRandom(false);
}

void MainWindow::setRepeat(const int state)
{
	if(state == Qt::Checked)
		mpd.setRepeat(true);
	else
		mpd.setRepeat(false);
}

void MainWindow::searchMusicLibrary()
{
	if(searchLibraryLineEdit->text().isEmpty()) {
		libraryProxyModel.setFilterRegExp("");
		return;
	}

	libraryProxyModel.setFilterField(searchFieldComboBox->currentIndex());
	libraryProxyModel.setFilterRegExp(searchLibraryLineEdit->text());
}

void MainWindow::musicLibarySearchFieldComboBoxChanged()
{
	settings.setValue("musiclibrary/searchfieldindex", searchFieldComboBox->currentIndex());
}

void MainWindow::searchDirView()
{
	if(searchDirViewLineEdit->text().isEmpty()) {
		dirProxyModel.setFilterRegExp("");
		return;
	}

	dirProxyModel.setFilterRegExp(searchDirViewLineEdit->text());
}

void MainWindow::searchPlaylist()
{
	if(searchPlaylistLineEdit->text().isEmpty()) {
		playlistProxyModel.setFilterRegExp("");
		return;
	}

	playlistProxyModel.setFilterRegExp(searchPlaylistLineEdit->text());
}

void MainWindow::updatePlaylist(QList<Song *> *songs)
{
	QList<qint32> selectedSongIds;
	qint32 firstSelectedSongId = -1;
	qint32 firstSelectedRow = -1;

	// remember selected song ids and rownum of smallest selected row in proxymodel (because that represents the visible rows)
	if (playlistTableView->selectionModel()->hasSelection()) {
		QModelIndexList items = playlistTableView->selectionModel()->selectedRows();
		QModelIndex index;
		QModelIndex sourceIndex;
		// find smallest selected rownum
		for(int i = 0; i < items.size(); i++) {
			index = items.at(i);
			sourceIndex = playlistProxyModel.mapToSource(index);
			selectedSongIds.append(playlistModel.getIdByRow(sourceIndex.row()));
			if (firstSelectedRow == -1 || index.row()<firstSelectedRow) {
				firstSelectedRow = index.row();
				firstSelectedSongId = playlistModel.getIdByRow(sourceIndex.row());
			}
		}
	}

	// refresh playlist
	playlistModel.updatePlaylist(songs);

	// reselect song ids or minrow if songids were not found (songs removed)
	bool found =  false;
	if (selectedSongIds.size() > 0) {
		qint32 newCurrentRow = playlistModel.getRowById(firstSelectedSongId);
		QModelIndex newCurrentSourceIndex = playlistModel.index(newCurrentRow,0);
		QModelIndex newCurrentIndex = playlistProxyModel.mapFromSource(newCurrentSourceIndex);
		playlistTableView->setCurrentIndex(newCurrentIndex);

		qint32 row;
		QModelIndex sourceIndex;
		QModelIndex index;
		for(int i = 0; i < selectedSongIds.size(); i++) {
			row = playlistModel.getRowById(selectedSongIds.at(i));
			if (row >= 0) {
				found = true;
			}
			sourceIndex = playlistModel.index(row,0);
			index = playlistProxyModel.mapFromSource(sourceIndex);
			playlistTableView->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
		}
		if (!found) {
			// songids which were selected before the playlistupdate were not found anymore (they were removed) -> select firstSelectedRow again (should be the next song after the removed one)
			// firstSelectedRow contains the selected row of the proxymodel before we did the playlist refresh
			// check if rowcount of current proxymodel is smaller than that (last row as removed) and adjust firstSelectedRow when needed
			index = playlistProxyModel.index(firstSelectedRow, 0);
			if (firstSelectedRow > playlistProxyModel.rowCount(index)-1) {
				firstSelectedRow = playlistProxyModel.rowCount(index)-1;
			}
			index = playlistProxyModel.index(firstSelectedRow, 0);
			playlistTableView->setCurrentIndex(index);
			playlistTableView->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
		}
	}
}

void MainWindow::updateCurrentSong(const Song *song)
{
	QSettings settings;

	// Determine if album cover should be updated
	if(trackArtistLabel->text() != song->artist || trackAlbumLabel->text() != song->album) {
		// Check if cover is already cached
		QString dir(QDir::toNativeSeparators(QDir::homePath()) + QDir::separator() + ".QtMPC" + QDir::separator());
		QString file(QFile::encodeName(song->artist + " - " + song->album + ".jpg"));
		metadataFetcher.setArtist(song->artist);
		metadataFetcher.setAlbumTitle(song->album);
		if(QFile::exists(dir + file)) {
			// Display image
			QPixmap pixmap = QPixmap::fromImage(QImage(dir + file));
			pixmap = pixmap.scaled(QSize(150, 150), Qt::KeepAspectRatio, Qt::SmoothTransformation);
			albumCoverLabel->setPixmap(pixmap);
			metadataFetcher.setFetchCover(true);
		} else {
			// Fetch image
			metadataFetcher.setFetchCover(true);
		}
		releaseDateLabel->setText("");
		if(settings.value("fetchAlbumInfo", "true").toBool())
			metadataFetcher.AlbumInfo();
	}

	metadataFetcher.setTrackTitle(song->title);
	if(settings.value("fetchAlbumInfo", "true").toBool())
		metadataFetcher.TrackInfo();

	trackTitleLabel->setText(song->title);
	trackArtistLabel->setText(song->artist);
	trackAlbumLabel->setText(song->album);

	playlistModel.updateCurrentSong(song->id);

	if (settings.value("systemtrayPopup").toBool() && trayIcon != NULL && trayIcon->isVisible() && isHidden()) {
		if (!song->title.isEmpty() && !song->artist.isEmpty() && !song->album.isEmpty()) {
#ifdef ENABLE_KDE_SUPPORT
			const QString text(QString("<table>")
				+ QString("<tr><td>Artist:</td><td>") + song->artist + "</td></tr>"
				+ QString("<tr><td>Album:</td><td>") + song->album + "</td></tr>"
				+ QString("<tr><td>Song:</td><td>") + song->title + "</td></tr>"
				+ QString("<tr><td>Track:</td><td>") + QString::number(song->track) + "</td></tr>"
				+ QString("<tr><td>Length:</td><td>") + Song::formattedTime(song->time) + "</td></tr>"
				+ QString("</table>"));
			KNotification *notification = new KNotification("CurrentTrackChanged", this);
			notification->setText(text);
			notification->sendEvent();
#else
			const QString text(
				"album:  " + song->album + "\n"
				+ "track:  " + QString::number(song->track) + "\n"
				+ "length: " + Song::formattedTime(song->time)
			);

			trayIcon->showMessage(song->artist + " - " + song->title, text,
				QSystemTrayIcon::Information, 5000);
#endif
		}
	}

	if (song->track > 0)
		toolTipText = QString::number(song->track) + " - " + song->artist + " - " + song->title + "\n";
	else
		toolTipText = song->artist + " - " + song->title + "\n";
	toolTipText += "album:  " + song->album + "\n";

	delete song;
}

void MainWindow::updateStats()
{
	MPDStats * const stats = MPDStats::getInstance();

	/*
	 * Check if remote db is more recent than local one
	 * Also update the dirview
	 */
	if(lastDbUpdate.isValid() && stats->dbUpdate() > lastDbUpdate) {
		mpdDb.listAllInfo(stats->dbUpdate());
		mpdDb.listAll();
	}

	lastDbUpdate = stats->dbUpdate();
}

void MainWindow::updateStatus()
{
	MPDStatus * const status = MPDStatus::getInstance();
	QString timeElapsedFormattedString;
	QSettings settings;

	if(!draggingPositionSlider) {
		if(status->state() == MPDStatus::State_Stopped
				|| status->state() == MPDStatus::State_Inactive) {
			positionSlider->setValue(0);
		} else {
			positionSlider->setRange(0, status->timeTotal());
			positionSlider->setValue(status->timeElapsed());
		}
	}

	volumeSlider->setValue(status->volume());

	if(status->consume())
		consumeCheckBox->setCheckState(Qt::Checked);
	else
		consumeCheckBox->setCheckState(Qt::Unchecked);

	if(status->random())
		randomCheckBox->setCheckState(Qt::Checked);
	else
		randomCheckBox->setCheckState(Qt::Unchecked);

	if(status->repeat())
		repeatCheckBox->setCheckState(Qt::Checked);
	else
		repeatCheckBox->setCheckState(Qt::Unchecked);

	if(status->state() == MPDStatus::State_Stopped
			|| status->state() == MPDStatus::State_Inactive) {
		timeElapsedFormattedString = "00:00 / 00:00";
	} else {
		timeElapsedFormattedString += Song::formattedTime(status->timeElapsed());
		timeElapsedFormattedString += " / ";
		timeElapsedFormattedString += Song::formattedTime(status->timeTotal());
		songTime = status->timeTotal();
	}

	songTimeElapsedLabel->setText(timeElapsedFormattedString);

	switch(status->state()) {
		case MPDStatus::State_Playing:
			#ifndef ENABLE_KDE_SUPPORT
			action_Play_pause_track->setText("&Pause");
			#endif
			action_Play_pause_track->setIcon(playbackPause);
			action_Play_pause_track->setEnabled(true);
			//playPauseTrackButton->setChecked(false);
			action_Stop_track->setEnabled(true);
			elapsedTimer.start();

			if (trayIcon != NULL)
				trayIcon->setIcon(icon_playing);

			break;
		case MPDStatus::State_Inactive:
		case MPDStatus::State_Stopped:
			#ifndef ENABLE_KDE_SUPPORT
			action_Play_pause_track->setText("&Play");
			#endif
			action_Play_pause_track->setIcon(playbackPlay);
			action_Play_pause_track->setEnabled(true);
			action_Stop_track->setEnabled(false);
                       
			trackTitleLabel->setText("");
			trackArtistLabel->setText("");
			trackAlbumLabel->setText("");
			albumCoverLabel->setPixmap(QPixmap());

			if (trayIcon != NULL)
				trayIcon->setIcon(icon);
			
			elapsedTimer.stop();

			break;
		case MPDStatus::State_Paused:
			#ifndef ENABLE_KDE_SUPPORT
			action_Play_pause_track->setText("&Play");
			#endif
			action_Play_pause_track->setIcon(playbackPlay);
			action_Play_pause_track->setEnabled(true);
			action_Stop_track->setEnabled(true);

			if (trayIcon != NULL)
				trayIcon->setIcon(icon_paused);
			
			elapsedTimer.stop();

			break;
		default:
			qDebug("Invalid state");
			break;
	}

	// Check if song has changed or we're playing again after being stopped
	// and update song info if needed
	if(lastState == MPDStatus::State_Inactive
			|| (lastState == MPDStatus::State_Stopped
				&& status->state() == MPDStatus::State_Playing)
			|| lastSongId != status->songId())
		mpd.currentSong();

	// Set TrayIcon tooltip
	/* No tooltip (ticket #204)
	QString text = toolTipText;
	text += "time: " + timeElapsedFormattedString;
	if (trayIcon != NULL)
		trayIcon->setToolTip(text);
	*/
	
	// Update status info
	lastState = status->state();
	lastSongId = status->songId();
	lastPlaylist = status->playlist();
}

void MainWindow::playlistItemActivated(const QModelIndex &index)
{
	QModelIndex sourceIndex = playlistProxyModel.mapToSource(index);
	mpd.startPlayingSongId(playlistModel.getIdByRow(sourceIndex.row()));
}

void MainWindow::clearPlaylist() {
	mpd.clear();
	searchPlaylistLineEdit->clear();
}

void MainWindow::removeFromPlaylist()
{
	const QModelIndexList items = playlistTableView->selectionModel()->selectedRows();
	QModelIndex sourceIndex;
	QList<qint32> toBeRemoved;

	if(items.isEmpty())
		return;

	for(int i = 0; i < items.size(); i++) {
		sourceIndex = playlistProxyModel.mapToSource(items.at(i));
		toBeRemoved.append(playlistModel.getIdByRow(sourceIndex.row()));
	}

	mpd.removeSongs(toBeRemoved);
}
void MainWindow::replacePlaylist()
{	
	mpd.clear();
	mpd.getStatus();
	searchPlaylistLineEdit->clear();
        if (libraryTreeView->isVisible()) {
            addSelectionToPlaylist();
        } else if (dirTreeView->isVisible()){
            addDirViewSelectionToPlaylist();
        }
}

void MainWindow::addSelectionToPlaylist()
{
	QStringList files;
	MusicLibraryItem *item;
	MusicLibraryItemSong *songItem;

	// Get selected view indexes
	const QModelIndexList selected = libraryTreeView->selectionModel()->selectedIndexes();
	int selectionSize = selected.size();

	if(selectionSize == 0) {
		QMessageBox::warning(
			this,
			tr("QtMPC: Warning"),
			tr("Couldn't add anything to the playlist because nothing is selected."
			   " Please select something first.")
		);
		return;
	}

	/*
	 * Loop over the selection. Only add files.
	 */
	for(int selectionPos = 0; selectionPos < selectionSize; selectionPos++) {
		const QModelIndex current = selected.at(selectionPos);
		item = static_cast<MusicLibraryItem *>(libraryProxyModel.mapToSource(current).internalPointer());

		switch(item->type()) {
			case MusicLibraryItem::Type_Artist: {
				for(quint32 i = 0; ; i++) {
					const QModelIndex album = current.child(i ,0);
					if (!album.isValid())
						break;

					for(quint32 j = 0; ; j++) {
						const QModelIndex track = album.child(j, 0);
						if (!track.isValid())
							break;
						const QModelIndex mappedSongIndex = libraryProxyModel.mapToSource(track);
						songItem = static_cast<MusicLibraryItemSong *>(mappedSongIndex.internalPointer());
						const QString fileName = songItem->file();
						if(!fileName.isEmpty() && !files.contains(fileName))
							files.append(fileName);
					}
				}
				break;
			}
			case MusicLibraryItem::Type_Album: {
				for(quint32 i = 0; ; i++) {
					QModelIndex track = current.child(i, 0);
					if (!track.isValid())
						break;
					const QModelIndex mappedSongIndex = libraryProxyModel.mapToSource(track);
					songItem = static_cast<MusicLibraryItemSong *>(mappedSongIndex.internalPointer());
					const QString fileName = songItem->file();
					if(!fileName.isEmpty() && !files.contains(fileName))
						files.append(fileName);
				}
				break;
			}
			case MusicLibraryItem::Type_Song: {
				const QString fileName = static_cast<MusicLibraryItemSong *>(item)->file();
				if(!fileName.isEmpty() && !files.contains(fileName))
					files.append(fileName);
				break;
			}
			default:
				break;
		}
	}

	if(!files.isEmpty()) {
		mpd.add(files);
		if(MPDStatus::getInstance()->state() != MPDStatus::State_Playing)
			mpd.startPlayingSong();

		libraryTreeView->selectionModel()->clearSelection();
	}
}

QStringList MainWindow::walkDirView(QModelIndex rootItem)
{
	QStringList files;

	DirViewItem *item = static_cast<DirViewItem *>(dirProxyModel.mapToSource(rootItem).internalPointer());

	if (item->type() == DirViewItem::Type_File) {
		return QStringList(item->fileName());
	}

	for(int i = 0; ; i++) {
		QModelIndex current = rootItem.child(i, 0);
		if (!current.isValid())
			return files;

		QStringList tmp = walkDirView(current);
		for (int j = 0; j < tmp.size(); j++) {
			if (!files.contains(tmp.at(j)))
				files << tmp.at(j);
		}
	}
	return files;
}

void MainWindow::addDirViewSelectionToPlaylist()
{
	QModelIndex current;
	QStringList files;
	DirViewItem *item;

	// Get selected view indexes
	const QModelIndexList selected = dirTreeView->selectionModel()->selectedIndexes();
	int selectionSize = selected.size();

	if(selectionSize == 0) {
		QMessageBox::warning(
			this,
			tr("QtMPC: Warning"),
			tr("Couldn't add anything to the playlist because nothing is selected."
			   " Please select something first.")
		);
		return;
	}

	for(int selectionPos = 0; selectionPos < selectionSize; selectionPos++) {
		current = selected.at(selectionPos);
		item = static_cast<DirViewItem *>(dirProxyModel.mapToSource(current).internalPointer());
		QStringList tmp;

		switch(item->type()) {
			case DirViewItem::Type_Dir:
				tmp = walkDirView(current);
				for (int i = 0; i < tmp.size(); i++) {
					if (!files.contains(tmp.at(i)))
						files << tmp.at(i);
				}
				break;
			case DirViewItem::Type_File:
				if (!files.contains(item->fileName()))
					files << item->fileName();
				break;
			default:
				break;
		}
	}

	if(!files.isEmpty()) {
		mpd.add(files);
		if(MPDStatus::getInstance()->state() != MPDStatus::State_Playing)
			mpd.startPlayingSong();

		dirTreeView->selectionModel()->clearSelection();
	}
}

void MainWindow::libraryItemActivated(const QModelIndex & /*index*/)
{
	MusicLibraryItem *item;
	const QModelIndexList selected = libraryTreeView->selectionModel()->selectedIndexes();
	int selectionSize = selected.size();
	if (selectionSize != 1) return; //doubleclick should only have one selected item
	
	const QModelIndex current = selected.at(0); 	
	item = static_cast<MusicLibraryItem *>(libraryProxyModel.mapToSource(current).internalPointer());
	if(item->type() == MusicLibraryItem::Type_Song) {
		addSelectionToPlaylist();
	}
}

void MainWindow::dirViewItemActivated(const QModelIndex & /*index*/)
{
	DirViewItem *item;
	const QModelIndexList selected = dirTreeView->selectionModel()->selectedIndexes();
	int selectionSize = selected.size();
	if (selectionSize != 1) return; //doubleclick should only have one selected item

	const QModelIndex current = selected.at(0);
	item = static_cast<DirViewItem *>(dirProxyModel.mapToSource(current).internalPointer());
	if(item->type() == DirViewItem::Type_File) {
		addDirViewSelectionToPlaylist();
	}
}

void MainWindow::addToPlaylistItemActivated()
{
    if (libraryTreeView->isVisible()) {
	addSelectionToPlaylist();
    } else if (dirTreeView->isVisible()){
        addDirViewSelectionToPlaylist();
    } else if (playlistTableView->isVisible()){
        addPlaylistToPlaylistPushButtonActivated();
    }
}

void MainWindow::addToPlaylistButtonActivated()
{
	addSelectionToPlaylist();
}

void MainWindow::dirViewAddToPlaylistPushButtonActivated()
{
	addDirViewSelectionToPlaylist();
}

void MainWindow::crossfadingChanged(int seconds)
{
	mpd.setCrossfade(seconds);
}

void MainWindow::saveSplitterState(int, int)
{
	QSettings settings;
	settings.setValue("splitterSizes", splitter->saveState());
}

/**
 * Playlist stats are updated.
 * Display it on the bottom of the playlist
 *
 * @param status QString with status
 */
void MainWindow::updatePlayListStatus()
{
	MPDStats * const stats = MPDStats::getInstance();
	QString status = "";

	if (stats->playlistSongs() == 0) {
		playListStatsLabel->setText(status);
		return;
	}

	status = QString::number(stats->playlistArtists());
	if (stats->playlistArtists() == 1) {
		status += " Artist, ";
	} else {
		status += " Artists, ";
	}

	status += QString::number(stats->playlistAlbums());
	if (stats->playlistAlbums() == 1) {
		status += " Album, ";
	} else {
		status += " Albums, ";
	}

	status += QString::number(stats->playlistSongs());
	if (stats->playlistSongs() == 1) {
		status += " Song, ";
	} else {
		status += " Songs, ";
	}

	status += "(";
	status += MPDParseUtils::seconds2formattedString(stats->playlistTime());
	status += ")";

	playListStatsLabel->setText(status);
}

void MainWindow::movePlaylistItems(const QList<quint32> items, const int row, const int size)
{
	mpd.move(items, row, size);
}

void MainWindow::addFilenamesToPlaylist(const QStringList filenames, const int row, const int size)
{
	mpd.addid(filenames, row, size);
	if(MPDStatus::getInstance()->state() != MPDStatus::State_Playing)
		mpd.startPlayingSong();
}

void MainWindow::updatePositionSilder()
{
	QString timeElapsedFormattedString;
	
	if (positionSlider->value() != positionSlider->maximum()) {
		positionSlider->setValue(positionSlider->value() + 1);
		
		timeElapsedFormattedString += Song::formattedTime(positionSlider->value());
		timeElapsedFormattedString += " / ";
		timeElapsedFormattedString += Song::formattedTime(songTime);
		songTimeElapsedLabel->setText(timeElapsedFormattedString);
	}
}

void MainWindow::copySongInfo()
{
	const QModelIndexList items = playlistTableView->selectionModel()->selectedRows();
	QModelIndex sourceIndex;
	QString txt = "";
	QClipboard *clipboard = QApplication::clipboard();

	if(items.isEmpty())
		return;

	for(int i = 0; i < items.size(); i++) {
		sourceIndex = playlistProxyModel.mapToSource(items.at(i));
		Song *s = playlistModel.getSongByRow(sourceIndex.row());
		if (s != NULL) {
				if (txt != "") {
					txt += "\n";
				}
				txt += Song::formatSong(s);
		}
	}
	
	clipboard->setText(txt);
}

void MainWindow::updateOutpus(const QList<Output *> outputs)
{
	#ifndef ENABLE_KDE_SUPPORT
	
	menu_Outputs->clear();
	
	for (int i = 0; i < outputs.size(); i++) {
		Output *o = outputs.at(i);
		
		QAction *a = new QAction(o->name, menu_Outputs);
		a->setCheckable(true);
		a->setChecked(o->enabled);
		a->setData(o->id);
		
		menu_Outputs->addAction(a);
	}
	
	#else
	
	QList<QAction *> actions;
	
	for (int i = 0; i < outputs.size(); i++) {
		Output *o = outputs.at(i);
		
		QAction *a = new QAction(o->name, this);
		a->setCheckable(true);
		a->setChecked(o->enabled);
		a->setData(o->id);
		
		actions << a;
	}	
	
	unplugActionList( "outputs_list" );
	plugActionList( "outputs_list", actions);

	#endif
}

void MainWindow::outputChanged(QAction *action) {
	if (action->isChecked()) {
		mpd.enableOutput(action->data().toInt());
	} else {
		mpd.disableOutput(action->data().toInt());
	}
}
