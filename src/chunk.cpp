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

#include "chunk.h"

Chunk::Chunk()
{
    m_id = 0;
    m_keyframeid = 0;
    m_data = "";
    m_duration = 30000;
}

Chunk::Chunk(int id, QByteArray data, int keyframeid, int duration)
{
    m_id = id;
    m_keyframeid = keyframeid;
    m_data = data;
    m_duration = duration;
}

Chunk::~Chunk()
{

}

int Chunk::getId() const
{
    return m_id;
}

int Chunk::getKeyframeId() const
{
    return m_keyframeid;
}

QByteArray Chunk::getData() const
{
    return m_data;
}

int Chunk::getDuration() const
{
    return m_duration;
}

int Chunk::getSize() const
{
    return m_data.size();
}

bool Chunk::isEmpty() const
{
    return m_data.isEmpty();
}
