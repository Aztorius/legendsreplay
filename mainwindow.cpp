#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle(tr("OpenReplay Alpha 1.0"));

    connect(ui->tableWidget, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(slot_doubleclick_featured(int,int)));

    QNetworkAccessManager *networkManager_status = new QNetworkAccessManager(this);

    connect(networkManager_status, SIGNAL(finished(QNetworkReply*)), this, SLOT(slot_networkResult_status(QNetworkReply*)));

    networkManager_status->get(QNetworkRequest(QUrl(tr("http://status.leagueoflegends.com/shards/euw"))));  // GET EUW SERVERS STATUS

    networkManager_status->get(QNetworkRequest(QUrl(tr("http://status.leagueoflegends.com/shards/na"))));  // GET NA SERVERS STATUS

    QNetworkAccessManager *networkManager_featured = new QNetworkAccessManager(this);

    connect(networkManager_featured, SIGNAL(finished(QNetworkReply*)), this, SLOT(slot_networkResult_featured(QNetworkReply*)));

    networkManager_featured->get(QNetworkRequest(QUrl(tr("http://spectator.euw1.lol.riotgames.com/observer-mode/rest/featured"))));  // GET EUW FEATURED GAMES

    networkManager_featured->get(QNetworkRequest(QUrl(tr("http://spectator.na.lol.riotgames.com/observer-mode/rest/featured"))));  // GET EUW FEATURED GAMES

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slot_doubleclick_featured(int row,int column){
    QProcess::startDetached("cmd", QStringList() << "/c start G://GAMES/LoL/RADS/solutions/lol_game_client_sln/releases/0.0.1.123/deploy/League of Legends.exe \"8394\" \"LoLLauncher.exe\" \"\" \"spectator spectator.na.lol.riotgames.com:80\" " + ui->tableWidget->selectedItems()[0]->text() + " " + ui->tableWidget->selectedItems()[0]->text() + " NA1");
}

void MainWindow::slot_networkResult_status(QNetworkReply *reply){

    if (reply->error() != QNetworkReply::NoError)
            return;

    QString data = (QString) reply->readAll();

    QJsonDocument jsonResponse = QJsonDocument::fromJson(data.toUtf8());

    if(jsonResponse.isEmpty()){
        ui->statusBar->showMessage(tr("No Json Content"));
    }

    QJsonObject jsonObject = jsonResponse.object();
    QJsonArray services = jsonObject.value(tr("services")).toArray();

    ui->statusBar->showMessage(jsonObject.value(tr("name")).toString() + tr(" : Status server infos"));

    if(jsonObject.value(tr("slug")).toString() == tr("euw")){
        if(services[0].toObject().value(tr("status")).toString() == tr("online")){
            ui->progressBar->setValue(50);
            if(services[1].toObject().value(tr("status")).toString() == tr("online")){
                ui->progressBar->setValue(100);
            }
        }
    }
    if(jsonObject.value(tr("slug")).toString() == tr("na")){
        if(services[0].toObject().value(tr("status")).toString() == tr("online")){
            ui->progressBar_2->setValue(50);
            if(services[1].toObject().value(tr("status")).toString() == tr("online")){
                ui->progressBar_2->setValue(100);
            }
        }
    }
}

void MainWindow::slot_networkResult_featured(QNetworkReply *reply){

    if (reply->error() != QNetworkReply::NoError)
            return;

    QString data = (QString) reply->readAll();

    QJsonDocument jsonResponse = QJsonDocument::fromJson(data.toUtf8());

    if(jsonResponse.isEmpty()){
        ui->statusBar->showMessage(tr("No Json Content"));
    }

    QJsonObject jsonObject = jsonResponse.object();

    QJsonArray gamelist = jsonObject.value(tr("gameList")).toArray();

    for(int i = 0; i < gamelist.size(); i++){
        ui->tableWidget->insertRow(ui->tableWidget->rowCount());

        QTableWidgetItem* item = new QTableWidgetItem();
        item->setText(gamelist[i].toObject().value(tr("platformId")).toString());

        ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 0, item);

        QTableWidgetItem* item2 = new QTableWidgetItem();

        item2->setText(QString::number(gamelist[i].toObject().value("gameId").toVariant().toULongLong()));

        ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 1, item2);

        QTableWidgetItem* item3 = new QTableWidgetItem();

        item3->setText(gamelist[i].toObject().value("observers").toObject().value("encryptionKey").toString());

        ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 2, item3);
    }

}
