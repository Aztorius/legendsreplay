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

#ifndef RECORDER_H
#define RECORDER_H

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <QJsonArray>
#include <math.h>

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
