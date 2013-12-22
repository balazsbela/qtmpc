DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x000000
TEMPLATE = app
TARGET = QtMPC
DEPENDPATH += . gui lib
INCLUDEPATH += . lib ./external/libmaia/
CONFIG += qt
QT += http network xml widgets gui
RESOURCES = QtMPC.qrc
#DEFINES += QT_NO_DEBUG_OUTPUT

# Input
HEADERS += external/libmaia/maiaObject.h \
		external/libmaia/maiaFault.h \
		external/libmaia/maiaXmlRpcClient.h \
		gui/main_window.h \
		gui/musiclibraryitem.h \
		gui/musiclibraryitemroot.h \
		gui/musiclibraryitemartist.h \
		gui/musiclibraryitemalbum.h \
		gui/musiclibraryitemsong.h \
		gui/musiclibrarymodel.h \
		gui/musiclibraryproxymodel.h \
		gui/playlistsmodel.h \
		gui/playlistsproxymodel.h \
		gui/playlisttablemodel.h \
		gui/playlisttableproxymodel.h \
		gui/about_dialog.h \
		gui/preferences_dialog.h \
		gui/statistics_dialog.h \
		gui/dirviewmodel.h \
		gui/dirviewproxymodel.h \
		gui/dirviewitem.h \
		gui/dirviewitemdir.h \
		gui/dirviewitemfile.h \
		gui/dirviewitemroot.h \
		lib/lastfm_metadata_fetcher.h \
		lib/mpdconnectionbase.h \
		lib/mpdconnectionplayback.h \
		lib/mpddatabaseconnection.h \
		lib/mpderror.h \
		lib/mpdparseutils.h \
		lib/mpdplaylistsconnection.h \
		lib/mpdstats.h \
		lib/mpdstatus.h \
		lib/output.h \
		lib/playlist.h \
		lib/song.h \
		QtMPC_config.h

FORMS += gui/main_window.ui \
		gui/about_dialog.ui \
		gui/preferences_dialog.ui \
		gui/statistics_dialog.ui

SOURCES += main.cpp \
		external/libmaia/maiaObject.cpp \
		external/libmaia/maiaFault.cpp \
		external/libmaia/maiaXmlRpcClient.cpp \
		gui/main_window.cpp \
		gui/main_window_playlist.cpp \
		gui/main_window_playlists.cpp \
		gui/main_window_trayicon.cpp \
		gui/musiclibraryitem.cpp \
		gui/musiclibraryitemroot.cpp \
		gui/musiclibraryitemartist.cpp \
		gui/musiclibraryitemalbum.cpp \
		gui/musiclibraryitemsong.cpp \
		gui/musiclibrarymodel.cpp \
		gui/musiclibraryproxymodel.cpp \
		gui/playlistsmodel.cpp \
		gui/playlistsproxymodel.cpp \
		gui/playlisttablemodel.cpp \
		gui/playlisttableproxymodel.cpp \
		gui/about_dialog.cpp \
		gui/preferences_dialog.cpp \
		gui/statistics_dialog.cpp \
		gui/dirviewmodel.cpp \
		gui/dirviewproxymodel.cpp \
		gui/dirviewitem.cpp \
		gui/dirviewitemfile.cpp \
		gui/dirviewitemdir.cpp \
		gui/dirviewitemroot.cpp \
		lib/lastfm_metadata_fetcher.cpp \
		lib/mpdconnectionbase.cpp \
		lib/mpdconnectionplayback.cpp \
		lib/mpddatabaseconnection.cpp \
		lib/mpderror.cpp \
		lib/mpdparseutils.cpp \
		lib/mpdplaylistsconnection.cpp \
		lib/mpdstats.cpp \
		lib/mpdstatus.cpp \
		lib/output.cpp \
		lib/playlist.cpp \
		lib/song.cpp
