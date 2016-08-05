#ifndef REPLAY_H
#define REPLAY_H

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

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
