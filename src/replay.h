#ifndef REPLAY_H
#define REPLAY_H

#include <QMainWindow>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>

class Chunk
{
public:
    Chunk();
    Chunk(int id, QByteArray data, int keyframeid, int duration = 30000);
    ~Chunk();
    int getId() const;
    int getKeyframeId() const;
    QByteArray getData() const;
    int getDuration() const;
    int getSize() const;

private:
    int m_id;
    int m_keyframeid;
    QByteArray m_data;
    int m_duration;
};

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

private:
    int m_id;
    int m_nextchunkid;
    QByteArray m_data;
};

class Replay
{
public:
    Replay(QString filepath);

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
