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

#ifndef REPLAY_H
#define REPLAY_H

#include <QJsonDocument>
#include <QString>
#include <QList>

#include "chunk.h"
#include "keyframe.h"

class Replay
{
public:
    Replay(QString filepath, bool loadInfosOnly = false);
    Replay();
    ~Replay();

    QString getFilepath() const;

    QString getGameId() const;
    QString getPlatformId() const;
    QString getEncryptionkey() const;

    QList<Keyframe> getKeyFrames() const;
    QList<Chunk> getChunks() const;
    QList<Chunk> getPrimaryChunks() const;

    QJsonDocument getGameinfos() const;
    QByteArray getEndOfGameStats() const;

    QString getServerVersion() const;
    QString getEndStartupChunkId() const;
    QString getStartGameChunkId() const;

    Chunk getChunk(int id) const;
    Chunk getPrimaryChunk(int id) const;
    Keyframe getKeyFrame(int id) const;
    Keyframe findKeyframeByChunkId(int chunkid) const;

    bool repair();

    void setGameId(QString gameId);
    void setPlatformId(QString platformId);
    void setEncryptionKey(QString encryptionKey);
    void setServerVersion(QString serverVersion);

    void setEndStartupChunkId(int id);
    void setStartGameChunkId(int id);

    void removeKeyFrame(int id);
    void removeChunk(int id);

    void setKeyFrames(QList<Keyframe> keyframes);

    bool saveAs(QString filepath);

    bool isEmpty();

private:
    QString m_gameid;
    QString m_platformid;
    QString m_encryptionkey;
    QString m_filepath;
    QString m_serverversion;
    QString m_endstartupchunkid;
    QString m_startgamechunkid;

    QList<Keyframe> m_keyframes;
    QList<Chunk> m_chunks;
    QList<Chunk> m_primarychunks;

    QJsonDocument m_gameinfos;
    QByteArray m_endofgamestats;
};

#endif // REPLAY_H
