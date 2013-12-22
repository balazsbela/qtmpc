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

#include <QtCore>

#include "mpdconnectionplayback.h"
#include "mpdparseutils.h"
#include "output.h"

MPDConnectionPlayback::MPDConnectionPlayback(QObject *parent)
	: MPDConnectionBase(parent),
		isIdle(false), enableIdle(true)
{
	initialize();
}

MPDConnectionPlayback::MPDConnectionPlayback(const QString &host, const quint16 port,
QObject*parent)
	: MPDConnectionBase(host, port, parent),
		isIdle(false), enableIdle(true)
{
	initialize();
}

MPDConnectionPlayback::~MPDConnectionPlayback()
{
	disconnect(&sock, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));

	sock.disconnectFromHost();
	quit();

	while(isRunning())
		msleep(100);
}

void MPDConnectionPlayback::initialize()
{
	connect(&sock, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));
}

QByteArray * MPDConnectionPlayback::readFromSocket()
{
	QByteArray *data = new QByteArray;

	while(true) {
		while(sock.bytesAvailable() == 0) {
			qDebug("MPDConnectionPlayback: waiting for data.");
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
	/*
	 * Send idle command
	 */
	if (enableIdle == true && isIdle == false) {
		connect(&sock, SIGNAL(readyRead()), this, SLOT(idleDataReady()));
		qDebug() << "Enabling idle";
		sock.write("idle\n");
		sock.waitForBytesWritten();
		isIdle = true;
	}

	mutex.unlock();

	return data;
}

bool MPDConnectionPlayback::commandOk()
{
	QByteArray data;

	while(!sock.canReadLine()) {
		qDebug("MPDConnectionPlayback: waiting for data.");
		if(sock.waitForReadyRead(5000)) {
			break;
		}
	}

	data += sock.readAll();
	
	/*
	 * Send idle command
	 */
	if (enableIdle == true && isIdle == false) {
		connect(&sock, SIGNAL(readyRead()), this, SLOT(idleDataReady()));
		qDebug() << "Enabling idle";
		sock.write("idle\n");
		sock.waitForBytesWritten();
		isIdle = true;
	}

	mutex.unlock();

	if(data == "OK\n")
		return true;

	qWarning() << "MPD Error:" << data;

	return false;
}

void MPDConnectionPlayback::sendCommand(const QByteArray &command)
{
	if(!connectToMPD()) {
		return;
	}

	mutex.lock();
	
	qDebug() << "command: " << command;
	/*
	 * Disable idle 
	 */
	if (isIdle == true) {
		disconnect(&sock, SIGNAL(readyRead()), this, SLOT(idleDataReady()));
		qDebug() << "Disabling idle";
		sock.write("noidle\n");
		sock.waitForBytesWritten();
		while(!sock.canReadLine()) {
			qDebug("MPDConnectionPlayback: waiting for data.");
			if(sock.waitForReadyRead(5000)) {
				break;
			}
		}
		QByteArray data = sock.readAll();
	  
		if (data != "OK\n") {
			if (!data.endsWith("OK\n")) {
				qWarning() << "idle command did not exit sucsesfull";
				qWarning() << "--------------";
				qWarning() << data;
				qWarning() << "--------------";
			} else {
						enableIdle = false;
						isIdle = false;
						mutex.unlock();
						parseIdleReturn(data);
						mutex.lock();
						enableIdle = true;
			}
		} 
		isIdle = false;
	}

	sock.write(command);
	sock.write("\n");
	sock.waitForBytesWritten(5000);
}


/*
 * Playlist commands
 */

void MPDConnectionPlayback::add(const QStringList &files)
{
	QByteArray send = "command_list_begin\n";

	for(int i = 0; i < files.size(); i++) {
		send += "add \"";
		send += files.at(i).toUtf8();
		send += "\"\n";
	}

	send += "command_list_end";

	sendCommand(send);

	if(!commandOk()) {
		qDebug("Couldn't add song(s) to playlist");
	}
}

/**
 * Add all the files in the QStringList to the playlist at
 * postition post.
 *
 * NOTE: addid is not fully supported in < 0.14 So add everything
 * and then move everything to the right spot.
 *
 * @param files A QStringList with all the files to add
 * @param pos Position to add the files
 * @param size The size of the current playlist
 */
void MPDConnectionPlayback::addid(const QStringList &files, const quint32 pos, const quint32 size)
{
	QByteArray send = "command_list_begin\n";
	int cur_size = size;
	for(int i = 0; i < files.size(); i++) {
		send += "add \"";
		send += files.at(i).toUtf8();
		send += "\"";
		send += "\n";
		send += "move ";
		send += QByteArray::number(cur_size);
		send += " ";
		send += QByteArray::number(pos);
		send += "\n";
		cur_size++;
	}

	send += "command_list_end";

	sendCommand(send);

	if(!commandOk()) {
		qDebug("Couldn't add song(s) to playlist");
	}
}

void MPDConnectionPlayback::clear()
{
	sendCommand("clear");

	if(!commandOk()) {
		qDebug("Couldn't clear playlist");
	}
}

void MPDConnectionPlayback::removeSongs(const QList<qint32> &items)
{
	QByteArray send = "command_list_begin\n";

	for(int i = 0; i < items.size(); i++) {
		send += "deleteid ";
		send += QByteArray::number(items.at(i));
		send += "\n";
	}

	send += "command_list_end";

	sendCommand(send);

	if(!commandOk()) {
		qDebug("Couldn't remove songs from playlist");
	}
}

void MPDConnectionPlayback::move(const quint32 from, const quint32 to)
{
	QByteArray send = "move " + QByteArray::number(from) + " " + QByteArray::number(to);

	qWarning() << send;
	sendCommand(send);

	if(!commandOk()) {
		qDebug("Couldn't move files around in playlist");
	}
}

void MPDConnectionPlayback::move(const QList<quint32> items, const quint32 pos, const quint32 size)
{
	QByteArray send = "command_list_begin\n";
	QList<quint32> moveItems;

	moveItems.append(items);
	qSort(moveItems);

	int posOffset = 0;

	//first move all items (starting with the biggest) to the end so we don't have to deal with changing rownums
	for (int i = moveItems.size()-1; i >= 0; i--) {
		if (moveItems.at(i) < pos && moveItems.at(i) != size-1) {
			// we are moving away an item that resides before the destinatino row, manipulate destination row
			posOffset++;
		}
		send += "move ";
		send += QByteArray::number(moveItems.at(i));
		send += " ";
		send += QByteArray::number(size-1);
		send += "\n";
	}
	//now move all of them to the destination position
	for (int i = moveItems.size()-1; i >= 0; i--) {
		send += "move ";
		send += QByteArray::number(size-1-i);
		send += " ";
		send += QByteArray::number(pos-posOffset);
		send += "\n";
	}

	send += "command_list_end";

	sendCommand(send);

	if (!commandOk()) {
		qDebug("Couldn't move songs around in playlist");
	}
}

void MPDConnectionPlayback::shuffle()
{
	sendCommand("shuffle");
	
	if (!commandOk()) {
		qDebug("Couldn't shuffle playlist");
	}
}

void MPDConnectionPlayback::shuffle(const quint32 from, const quint32 to)
{
	QByteArray command = "shuffle ";
	command += QByteArray::number(from);
	command += ":";
	command += QByteArray::number(to+1);

	sendCommand(command);
	
	if (!commandOk()) {
		qDebug("Couldn't shuffle playlist");
	}
}

void MPDConnectionPlayback::currentSong()
{
	QByteArray *data;

	sendCommand("currentsong");
	data = readFromSocket();

	emit currentSongUpdated(MPDParseUtils::parseSong(data));

	delete data;
}

void MPDConnectionPlayback::playListInfo()
{
	QByteArray *data;

	sendCommand("playlistinfo");
	data = readFromSocket();

	emit playlistUpdated(MPDParseUtils::parseSongs(data));

	delete data;
}

/*
 * Playback commands
 */

void MPDConnectionPlayback::setCrossfade(const quint8 secs)
{
	QByteArray data = "crossfade ";
	data += QByteArray::number(secs);
	sendCommand(data);

	if(!commandOk()) {
		qDebug("Couldn't go to next track");
	}
}

void MPDConnectionPlayback::goToNext()
{
	sendCommand("next");

	if(!commandOk()) {
		qDebug("Couldn't go to next track");
	}
}

void MPDConnectionPlayback::setPause(const bool toggle)
{
	QByteArray data = "pause ";
	if(toggle == true)
		data += "1";
	else
		data += "0";

	sendCommand(data);

	if(!commandOk()) {
		qDebug("Couldn't set pause");
	}
}

void MPDConnectionPlayback::startPlayingSong(const quint32 song)
{
	QByteArray data = "play ";
	data += QByteArray::number(song);
	sendCommand(data);

	if(!commandOk()) {
		qDebug("Couldn't start playing song");
	}
}

void MPDConnectionPlayback::startPlayingSongId(const quint32 song_id)
{
	QByteArray data = "playid ";
	data += QByteArray::number(song_id);
	sendCommand(data);

	if(!commandOk()) {
		qDebug("Couldn't start playing song id");
	}
}

void MPDConnectionPlayback::goToPrevious()
{
	sendCommand("previous");

	if(!commandOk()) {
		qDebug("Couldn't go to previous track");
	}
}

void MPDConnectionPlayback::setConsume(const bool toggle)
{
	QByteArray data = "consume ";
	if(toggle == true)
		data += "1";
	else
		data += "0";

	sendCommand(data);

	if(!commandOk()) {
		qDebug("Couldn't toggle consume");
	}
}

void MPDConnectionPlayback::setRandom(const bool toggle)
{
	QByteArray data = "random ";
	if(toggle == true)
		data += "1";
	else
		data += "0";

	sendCommand(data);

	if(!commandOk()) {
		qDebug("Couldn't toggle random");
	}
}

void MPDConnectionPlayback::setRepeat(const bool toggle)
{
	QByteArray data = "repeat ";
	if(toggle == true)
		data += "1";
	else
		data += "0";

	sendCommand(data);

	if(!commandOk()) {
		qDebug("Couldn't toggle repeat");
	}
}

void MPDConnectionPlayback::setSeek(const quint32 song, const quint32 time)
{
	QByteArray data = "seek ";
	data += QByteArray::number(song);
	data += " ";
	data += QByteArray::number(time);

	sendCommand(data);

	if(!commandOk()) {
		qDebug("Couldn't set seek position");
	}
}

void MPDConnectionPlayback::setSeekId(const quint32 song_id, const quint32 time)
{
	QByteArray data = "seekid ";
	data += QByteArray::number(song_id);
	data += " ";
	data += QByteArray::number(time);
	sendCommand(data);

	if(!commandOk()) {
		qDebug("Couldn't set seek position");
	}
}

void MPDConnectionPlayback::setVolume(const quint8 vol)
{
	QByteArray data = "setvol ";
	data += QByteArray::number(vol);
	sendCommand(data);

	if(!commandOk()) {
		qDebug("Couldn't set volume");
	}
}

void MPDConnectionPlayback::stopPlaying()
{
	sendCommand("stop");

	if(!commandOk()) {
		qDebug("Couldn't stop playing");
	}
}

void MPDConnectionPlayback::getStats()
{
	QByteArray *data;

	sendCommand("stats");
	data = readFromSocket();
	MPDParseUtils::parseStats(data);

	emit statsUpdated();

	delete data;
}

void MPDConnectionPlayback::getStatus()
{
	QByteArray *data;

	sendCommand("status");
	data = readFromSocket();
	MPDParseUtils::parseStatus(data);

	emit statusUpdated();

	delete data;
}

/*
 * Data is written during idle.
 * Retrieve it and parse it
 */
void MPDConnectionPlayback::idleDataReady()
{
	QByteArray data;
	
	mutex.lock();
	if (sock.bytesAvailable() == 0) {
		mutex.unlock();
		return;
	}
	disconnect(&sock, SIGNAL(readyRead()), this, SLOT(idleDataReady()));
	while(true) {
		while(sock.bytesAvailable() == 0) {
			qDebug("MPDConnectionPlayback: waiting for data.");
			if(sock.waitForReadyRead(5000)) {
				break;
			}
		}

		data.append(sock.readAll());

		if(data.endsWith("OK\n") || data.startsWith("OK")) {
			break;
		} else {
			qDebug() << "WARNING!";
		}
	}
	isIdle = false;
	
	mutex.unlock();
	parseIdleReturn(data);
}

/*
 * Socket state has changed, currently we only use this to gracefully
 * handle disconnects.
 */
void MPDConnectionPlayback::onSocketStateChanged(QAbstractSocket::SocketState socketState)
{
	if(socketState == QAbstractSocket::ClosingState)
		emit mpdConnectionDied();
}

/*
 * Parse data returned by the mpd deamon on an idle commond.
 */
void MPDConnectionPlayback::parseIdleReturn(const QByteArray &data)
{
	QList<QByteArray> lines = data.split('\n');
	
	QByteArray line;

	/*
	 * See http://www.musicpd.org/doc/protocol/ch02.html
	 */
	foreach(line, lines) {
		if (line == "changed: database") {
			/*
			 * Temp solution
			 */
			getStats();
		} else if (line == "changed: update") {
			qDebug() << "IdleReturn: fix: database_update";
		} else if (line == "changed: stored_playlist") {
			/*
			 * Emit signal to update playlists.
			 * Since this is done in an other thread re-enable idle mode
			 */
			emit storedPlayListUpdated();
			if (enableIdle == true) {
				mutex.lock();
				connect(&sock, SIGNAL(readyRead()), this, SLOT(idleDataReady()));
				qDebug() << "Enabling idle";
				sock.write("idle\n");
				sock.waitForBytesWritten();
				isIdle = true;
				mutex.unlock();
			}
		} else if (line == "changed: playlist") {
			playListInfo();
		} else if (line == "changed: player") {
			getStatus();
		} else if (line == "changed: mixer") {
			getStatus();
		} else if (line == "changed: output") {
			outputs();
		} else if (line == "changed: options") {
			getStatus();
		} else if (line == "OK") {
			break;
		} else {
			qWarning() << "Unkown command in idle return: " << line;
		}
	}
}

void MPDConnectionPlayback::outputs()
{
	QByteArray *data;
	
	sendCommand("outputs");
	data = readFromSocket();
	QList<Output *> outputs = MPDParseUtils::parseOuputs(data);

	emit outputsUpdated(outputs);
	
	delete data;
}

void MPDConnectionPlayback::enableOutput(const quint8 id)
{
	QByteArray data = "enableoutput ";
	data += QByteArray::number(id);
	sendCommand(data);

	if(!commandOk()) {
		qDebug("Couldn't enable output");
	}
}

void MPDConnectionPlayback::disableOutput(const quint8 id)
{
	QByteArray data = "disableoutput ";
	data += QByteArray::number(id);
	sendCommand(data);

	if(!commandOk()) {
		qDebug("Couldn't disable output");
	}
}
