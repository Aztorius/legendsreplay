#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QApplication>
#include <QtNetwork>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QSystemTrayIcon>
#include <QDesktopServices>
#include <QMenu>
#include <QAction>
#include <QListWidgetItem>
#include <QProgressBar>
#include <QFileSystemWatcher>

#include "qhttpserver.hpp"
#include "qhttpserverresponse.hpp"
#include "qhttpserverrequest.hpp"

#include "replay.h"
#include "recorder.h"
#include "server.h"
#include "gameinfoswidget.h"
#include "advancedrecorderdialog.h"
#include "repairtooldialog.h"

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

    void lol_launch(QString platformid, QString key, QString matchid, bool local = false);
    bool check_path(QString path);
    bool game_ended(QString region, QString gameid);
    void replay_launch(QString pathfile);

    QJsonDocument getCurrentPlayingGameInfos(QString serverRegion, QString summonerId);
    QJsonDocument getJsonFromUrl(QString url);
    QByteArray getFileFromUrl(QString url);

    QPixmap getImg(int id);
    bool islolRunning();
    bool islolclientRunning();

    Server getServerByPlatformId(QString platformid);
    Server getServerByRegion(QString region);
    Server getServerByDomain(QString domain);

    void refreshRecordingGamesWidget();

private:
    Ui::MainWindow *ui;

    QSettings *orsettings;

    QNetworkAccessManager *networkManager_status;
    QNetworkAccessManager *networkManager_featured;
    QNetworkAccessManager *networkManager_replayServers;

    QSystemTrayIcon *systemtrayicon;

    QList <QStringList> recording;
    QList <QThread*> recordingThreads;
    QList <QString> recordedgames_filename;
    QList <QString> yourgames_filename;

    QList <QJsonObject> json_status;
    QList <QJsonObject> json_featured;

    QList <Server> servers;
    QList <QString> lrservers;

    QString loldirectory;
    QString lolpbedirectory;
    QString replaydirectory;

    bool replaying;
    bool playing;
    bool systemtrayavailable;

    QString m_summonername;
    QString m_summonerid;
    QString m_summonerServerRegion;
    QString m_PBEname;
    QString m_PBEid;

    QString m_currentLegendsReplayServer;

    QTimer *m_timer;

    QHttpServer *httpserver;
    Replay *replay;

    QJsonDocument m_searchsummoner_game;

    int serverChunkCount;
    int serverKeyframeCount;

    QString m_currentsystemtraymessage;

    QTranslator translator;

    QFileSystemWatcher* m_directory_watcher;

private slots:
    void slot_checkSoftwareVersion();

    void slot_networkResult_status(QNetworkReply *reply);
    void slot_networkResult_featured(QNetworkReply *reply);
    void slot_networkResult_replayServers(QNetworkReply *reply);

    void slot_refreshPlayingStatus();
    void slot_refresh_recordedGames();

    void slot_doubleclick_savedgames(int row,int column);
    void slot_doubleclick_mygames(int row, int column);
    void slot_doubleclick_featured(QListWidgetItem*item);
    void slot_click_allgames();
    void slot_click_searchsummoner_spectate();
    void slot_click_searchsummoner_record();
    void slot_click_replayservers();

    void slot_searchsummoner();

    void slot_open_replay(bool param);

    void slot_replayserversAdd();

    void slot_summonerinfos_save();
    void slot_pbeinfos_save();

    void slot_featuredRefresh();
    void slot_statusRefresh();

    void slot_setdirectory();
    void slot_setpbedirectory();
    void slot_setreplaydirectory();

    void slot_featuredLaunch();
    void slot_featuredRecord();

    void slot_changedTab(int index);
    void slot_endRecording(QString platformid, QString gameid);
    void log(QString);
    void showmessage(QString message);
    void systemtrayiconActivated(QSystemTrayIcon::ActivationReason reason);
    void slot_messageclicked();

    void slot_openAdvancedRecorder();
    void slot_customGameRecord(QString serverAddress, QString serverRegion, QString gameId, QString encryptionKey, bool forceCompleteDownload = false, bool downloadInfos = false, bool downloadStats = true);

    void slot_customcontextmenu(QPoint point);
    void slot_custommenutriggered(QAction* action);
    void slot_customMenuTriggeredSystemTrayIcon(QAction* action);

    void slot_reportAnIssue();
    void slot_aboutLegendsReplay();
    void slot_checkandrepair();

    void slot_setLanguage();
    void slot_directoryChanged(QString path);

protected:
    void changeEvent(QEvent*);

signals:
    void signal_refresh_recordedGames();
    void signal_checkSoftwareVersion();
    void signal_refreshStatusServers();
};

#endif // MAINWINDOW_H
