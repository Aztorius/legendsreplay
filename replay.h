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

private:
    QString m_gameid;
    QString m_serverid;
    QString m_encryptionkey;
    QString m_filepath;
    QString m_serverversion;

    QList<QByteArray> m_keyframes;
    QList<int> m_keyframesid;
    QList<QByteArray> m_chunks;
    QList<int> m_chunksid;

    QJsonDocument m_gameinfos;
};

#endif // REPLAY_H
