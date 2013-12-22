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

#ifndef MPDERROR_H
#define MPDERROR_H

#include <QObject>

class MPDError : public QObject
{
	Q_OBJECT

	public:
		enum State {
			State_OK, /* There's no error, everything is fine */
			State_Error
		};

		MPDError(State state, QString error, QObject *parent = 0);
		~MPDError();

		State state() const;
		QString errorString() const;
		bool isError() const;

	private:
		State m_state;
		QString m_error;
};

#endif
