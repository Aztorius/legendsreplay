#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include <QFileDialog>
#include <QMessageBox>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void log(QString);
    void lol_launch(QString serverid, QString key, QString matchid);
    bool check_path(QString path);
    void record_game(QString serverid, QString gameid);
    bool game_ended(QString serverid, QString gameid);

private:
    Ui::MainWindow *ui;
    QNetworkAccessManager *networkManager_status;
    QNetworkAccessManager *networkManager_featured;
    QNetworkAccessManager *networkManager_record;
    bool recording;
    QList <QJsonObject> json_status;
    QList <QJsonObject> json_featured;
    //QStringList<QString name, QString slug, QString address>
    QList <QStringList> servers;
    QString loldirectory;

private slots:
    void slot_networkResult_status(QNetworkReply* reply);
    void slot_networkResult_featured(QNetworkReply *reply);
    void slot_networkResult_record(QNetworkReply *reply);
    void slot_doubleclick_featured(int row,int column);
    void slot_click_featured(int row,int column);
    void slot_featuredRefresh();
    void slot_setdirectory();
    void slot_featuredLaunch();
    void slot_changedTab(int index);
};

#endif // MAINWINDOW_H
