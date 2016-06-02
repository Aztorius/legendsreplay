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
    ~Replay();

    QString getGameid();
    QString getServerid();
    QString getEncryptionkey();

    QList<Keyframe> getKeyFrames();
    QList<Chunk> getChunks();
    QList<Chunk> getPrimaryChunks();

    QJsonDocument getGameinfos();
    QByteArray getEndOfGameStats();

    QString getServerversion();
    QString getEndstartupchunkid();
    QString getStartgamechunkid();

    Chunk getChunk(int id) const;
    Chunk getPrimaryChunk(int id) const;
    Keyframe getKeyFrame(int id) const;
    Keyframe findKeyframeByChunkId(int chunkid);

    bool repair();

private:
    QString m_gameid;
    QString m_serverid;
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
