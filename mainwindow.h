#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QtConcurrent/QtConcurrent>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void lol_launch(QString serverid, QString key, QString matchid);
    bool check_path(QString path);
    bool game_ended(QString serverid, QString gameid);

    QJsonDocument getCurrentPlayingGameInfos(QString server, QString summonerid);
    QJsonDocument getJsonFromUrl(QString url);
    QByteArray getFileFromUrl(QString url);

    void refresh_recordedGames();
    bool islolRunning();
    bool islolclientRunning();

private:
    Ui::MainWindow *ui;

    QNetworkAccessManager *networkManager_status;
    QNetworkAccessManager *networkManager_featured;

    QList <QStringList> recording;

    QList <QJsonObject> json_status;
    QList <QJsonObject> json_featured;

    QList <QStringList> servers;
    QList <QString> orservers;

    QString loldirectory;
    QString replaydirectory;

    bool replaying;
    bool playing;

    QString m_summonername;
    QString m_summonerid;
    QString m_summonerserver;

    QTimer *m_timer;

private slots:
    void slot_networkResult_status(QNetworkReply* reply);
    void slot_networkResult_featured(QNetworkReply *reply);

    void slot_refreshPlayingStatus();

    void slot_doubleclick_featured(int row,int column);
    void slot_click_featured(int row,int column);

    void slot_replayserversAdd();
    void slot_summonerinfos_save();

    void slot_featuredRefresh();

    void slot_setdirectory();
    void slot_setreplaydirectory();

    void slot_featuredLaunch();
    void slot_featuredRecord();

    void slot_changedTab(int index);
    void slot_endRecording(QString serverid, QString gameid);
    void log(QString);
};

#endif // MAINWINDOW_H
