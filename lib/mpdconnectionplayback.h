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

#ifndef MPDCONNECTIONPLAYBACK_H
#define MPDCONNECTIONPLAYBACK_H

#include <QMutex>
#include <QThread>
#include <QTcpSocket>
#include <QDateTime>

#include "mpdconnectionbase.h"
#include "mpdstats.h"
#include "mpdstatus.h"
#include "song.h"
#include "output.h"

class MPDConnectionPlayback : public MPDConnectionBase
{
	Q_OBJECT

	public:
		MPDConnectionPlayback(QObject *parent = 0);
		MPDConnectionPlayback(const QString &host, const quint16 port, QObject *parent = 0);
		~MPDConnectionPlayback();

		// Admin
		// TODO

		// Playlist
		void add(const QStringList &files);
		void addid(const QStringList &files, const quint32 pos, const quint32 size);
		void currentSong();
		void playListInfo();
		void removeSongs(const QList<qint32> &items);
		void move(const quint32 from, const quint32 to);
		void move(const QList<quint32> items, const quint32 pos, const quint32 size);
		void shuffle(const quint32 from, const quint32 to);
		// TODO

		// Playback
		void setCrossfade(const quint8 secs);
		void goToNext();
		void setPause(const bool toggle);
		void startPlayingSong(const quint32 song = 0);
		void startPlayingSongId(const quint32 song_id = 0);
		void goToPrevious();
		void setConsume(const bool toggle);
		void setRandom(const bool toggle);
		void setRepeat(const bool toggle);
		void setSeek(const quint32 song, const quint32 time);
		void setSeekId(const quint32 song_id, const quint32 time);
		void setVolume(const quint8 vol);
		void stopPlaying();
		
		// Output
		void outputs();
		void enableOutput(const quint8 id);
		void disableOutput(const quint8 id);

	public slots:
		// Playlist
		void clear();
		void shuffle();

		// Miscellaneous
		void getStats();
		void getStatus();

	protected:
		bool commandOk();
		void sendCommand(const QByteArray &command);

	protected slots:
		QByteArray * readFromSocket();
		
	private:
		bool isIdle;
		bool enableIdle;
		void initialize();
		void parseIdleReturn(const QByteArray &data);
		
	private slots:
		void idleDataReady();
		void onSocketStateChanged(QAbstractSocket::SocketState socketState);

	signals:
		void currentSongUpdated(const Song *song);
		void mpdConnectionDied();
		void playlistUpdated(QList<Song *> *songs);
		void statsUpdated();
		void statusUpdated();
		void storedPlayListUpdated();
		void outputsUpdated(const QList<Output *> outputs);
};

#endif
