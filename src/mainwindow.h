#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QtConcurrent/QtConcurrent>
#include <QPainter>
#include <QSystemTrayIcon>
#include <QPixmapCache>

#include "qhttpserver.hpp"
#include "qhttpserverresponse.hpp"
#include "qhttpserverrequest.hpp"

#include "replay.h"

using namespace qhttp::server;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void setArgs(int argc, char *argv[]);

    void lol_launch(QString serverid, QString key, QString matchid, bool local = false);
    bool check_path(QString path);
    bool game_ended(QString serverid, QString gameid);
    void replay_launch(QString pathfile);

    QJsonDocument getCurrentPlayingGameInfos(QString server, QString summonerid);
    QJsonDocument getJsonFromUrl(QString url);
    QByteArray getFileFromUrl(QString url);

    QPixmap getImg(int id);
    void refresh_recordedGames();
    bool islolRunning();
    bool islolclientRunning();

private:
    Ui::MainWindow *ui;

    QSettings *orsettings;

    QNetworkAccessManager *networkManager_status;
    QNetworkAccessManager *networkManager_featured;

    QSystemTrayIcon *systemtrayicon;

    QList <QStringList> recording;
    QList <QString> recordedgames_filename;
    QList <QString> yourgames_filename;

    QList <QJsonObject> json_status;
    QList <QJsonObject> json_featured;

    QList <QStringList> servers;
    QList <QString> orservers;

    QString loldirectory;
    QString lolpbedirectory;
    QString replaydirectory;

    bool replaying;
    bool playing;
    bool systemtrayavailable;

    QString m_summonername;
    QString m_summonerid;
    QString m_summonerserver;

    QTimer *m_timer;

    QHttpServer *httpserver;
    Replay *replay;

    QJsonDocument m_searchsummoner_game;

    int serverChunkCount;
    int serverKeyframeCount;

private slots:
    void slot_networkResult_status(QNetworkReply *reply);
    void slot_networkResult_featured(QNetworkReply *reply);

    void slot_refreshPlayingStatus();

    void slot_doubleclick_savedgames(int row,int column);
    void slot_doubleclick_mygames(int row, int column);
    void slot_doubleclick_featured(int row,int column);
    void slot_click_featured();
    void slot_click_allgames();
    void slot_click_searchsummoner_spectate();
    void slot_click_searchsummoner_record();

    void slot_searchsummoner();

    void slot_open_replay(bool param);

    void slot_replayserversAdd();
    void slot_summonerinfos_save();

    void slot_featuredRefresh();
    void slot_statusRefresh();

    void slot_setdirectory();
    void slot_setpbedirectory();
    void slot_setreplaydirectory();

    void slot_featuredLaunch();
    void slot_featuredRecord();

    void slot_changedTab(int index);
    void slot_endRecording(QString serverid, QString gameid);
    void log(QString);
    void showmessage(QString message);
    void systemtrayiconActivated(QSystemTrayIcon::ActivationReason reason);
};

#endif // MAINWINDOW_H
