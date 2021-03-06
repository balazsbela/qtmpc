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

#ifndef MUSIC_LIBRARY_SORT_FILTER_MODEL_H
#define MUSIC_LIBRARY_SORT_FILTER_MODEL_H

#include <QSortFilterProxyModel>

class MusicLibraryItem;

class MusicLibraryProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT

	public:
		MusicLibraryProxyModel(QObject *parent = 0);
		bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
		bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
		void setFilterField(int field);
		int filterField() const;

	private:
		int _filterField;

		bool filterAcceptsArtist(const MusicLibraryItem * const item) const;
		bool filterAcceptsAlbum(const MusicLibraryItem * const item) const;
		bool filterAcceptsSong(const MusicLibraryItem * const item) const;
};

#endif
