#ifndef RECORDER_H
#define RECORDER_H

#include <QThread>
#include <QMutex>
#include "mainwindow.h"
#include "replay.h"

class Recorder : public QThread
{
    Q_OBJECT

    private:
        QString m_serverid;
        QString m_serveraddress;
        QString m_gameid;
        QString m_encryptionkey;
        QJsonDocument m_gameinfo;
        QString m_replaydirectory;
        QString m_endstartupchunkid;
        QString m_startgamechunkid;

    public:
        Recorder(MainWindow *window, QString serverid, QString serveraddress, QString gameid, QString encryptionkey, QJsonDocument gameinfo, QString replaydirectory);
        QByteArray getFileFromUrl(QString url);
        QJsonDocument getJsonFromUrl(QString url);

    public slots:
        void run();

    signals:
        void toLog(QString logstring);
        void end(QString serverid, QString gameid);
        void toShowmessage(QString message);
};

#endif // RECORDER_H
