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

#include "mpderror.h"

MPDError::MPDError(State state, QString error, QObject *parent)
	: QObject(parent),
		m_state(state),
		m_error(error)
{
}

MPDError::~MPDError()
{
}

MPDError::State MPDError::state() const
{
	return m_state;
}

QString MPDError::errorString() const
{
	return m_error;
}

bool MPDError::isError() const
{
	return m_state == State_Error;
}
