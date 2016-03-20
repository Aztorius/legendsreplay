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

private:
    Ui::MainWindow *ui;

private slots:
    void slot_networkResult_status(QNetworkReply* reply);
    void slot_networkResult_featured(QNetworkReply *reply);
    void slot_doubleclick_featured(int row,int column);
};

#endif // MAINWINDOW_H
