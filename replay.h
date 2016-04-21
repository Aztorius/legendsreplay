#ifndef REPLAY_H
#define REPLAY_H

#include "mainwindow.h"

class Replay
{
public:
    Replay(QString filepath);
    QString getGameid();
    QString getServerid();
    QString getEncryptionkey();
    QList<QByteArray> getKeyFrames();
    QList<QByteArray> getChunks();
    QList<int> getKeyFramesid();
    QList<int> getChunksid();
    QJsonDocument getGameinfos();
    QString getServerversion();
    QString getEndstartupchunkid();
    QString getStartgamechunkid();

private:
    QString m_gameid;
    QString m_serverid;
    QString m_encryptionkey;
    QString m_filepath;
    QString m_serverversion;
    QString m_endstartupchunkid;
    QString m_startgamechunkid;

    QList<QByteArray> m_keyframes;
    QList<int> m_keyframesid;
    QList<QByteArray> m_chunks;
    QList<int> m_chunksduration;
    QList<int> m_chunkskeyframesid;
    QList<int> m_chunksid;

    QJsonDocument m_gameinfos;
};

#endif // REPLAY_H
