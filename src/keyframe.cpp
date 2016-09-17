/* ****************************************************************************
 *
 * Copyright 2016 William Bonnaventure
 *
 * This file is part of LegendsReplay.
 *
 * LegendsReplay is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * LegendsReplay is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LegendsReplay.  If not, see <http://www.gnu.org/licenses/>.
 *
 * ****************************************************************************
 */

#include "keyframe.h"

Keyframe::Keyframe()
{
    m_id = 0;
    m_nextchunkid = 0;
    m_data = "";
}

Keyframe::Keyframe(int id, QByteArray data, int nextchunkid)
{
    m_id = id;
    m_nextchunkid = nextchunkid;
    m_data = data;
}

Keyframe::~Keyframe()
{

}

int Keyframe::getId() const
{
    return m_id;
}

int Keyframe::getNextchunkid() const
{
    return m_nextchunkid;
}

QByteArray Keyframe::getData() const
{
    return m_data;
}

int Keyframe::getSize() const
{
    return m_data.size();
}

bool Keyframe::isEmpty() const
{
    return m_data.isEmpty();
}
