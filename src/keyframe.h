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

#ifndef KEYFRAME_H
#define KEYFRAME_H

#include <QByteArray>

class Keyframe
{
public:
    Keyframe();
    Keyframe(int id, QByteArray data, int nextchunkid);
    ~Keyframe();
    int getId() const;
    int getNextchunkid() const;
    QByteArray getData() const;
    int getSize() const;
    bool isEmpty() const;

private:
    int m_id;
    int m_nextchunkid;
    QByteArray m_data;
};

#endif // KEYFRAME_H
