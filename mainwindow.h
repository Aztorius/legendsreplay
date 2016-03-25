#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>

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

private:
    Ui::MainWindow *ui;
    QNetworkAccessManager *networkManager_status;
    QNetworkAccessManager *networkManager_featured;
    QList <QJsonObject> json_status;
    QList <QJsonObject> json_featured;

private slots:
    void slot_networkResult_status(QNetworkReply* reply);
    void slot_networkResult_featured(QNetworkReply *reply);
    void slot_doubleclick_featured(int row,int column);
    void slot_click_featured(int row,int column);
    void slot_featuredRefresh();
};

#endif // MAINWINDOW_H
