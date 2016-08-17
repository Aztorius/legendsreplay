#ifndef RECORDER_H
#define RECORDER_H

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <QJsonArray>

#include "replay.h"

class Recorder : public QObject
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
        bool m_forceCompleteDownload;
        bool m_downloadStats;

    public:
        Recorder(QString serverid, QString serveraddress, QString gameid, QString encryptionkey, QJsonDocument gameinfo, QString replaydirectory, bool forceCompleteDownload = false, bool downloadStats = true);
        ~Recorder();
        QByteArray getFileFromUrl(QString url);
        QJsonDocument getJsonFromUrl(QString url);

    public slots:
        void launch();

    signals:
        void toLog(QString logstring);
        void end(QString serverid, QString gameid);
        void toShowmessage(QString message);
        void finished();
};

#endif // RECORDER_H
