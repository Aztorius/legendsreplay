#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle(tr("OpenReplay Alpha 2.1"));

    log(QString("OpenReplay Alpha 2.1 Started"));

    connect(ui->pushButton_2, SIGNAL(released()), this, SLOT(slot_featuredRefresh()));
    connect(ui->tableWidget, SIGNAL(cellClicked(int,int)), this, SLOT(slot_click_featured(int,int)));
    connect(ui->tableWidget, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(slot_doubleclick_featured(int,int)));

    networkManager_status = new QNetworkAccessManager(this);

    connect(networkManager_status, SIGNAL(finished(QNetworkReply*)), this, SLOT(slot_networkResult_status(QNetworkReply*)));

    networkManager_status->get(QNetworkRequest(QUrl(tr("http://status.leagueoflegends.com/shards/euw"))));  // GET EUW SERVERS STATUS

    networkManager_status->get(QNetworkRequest(QUrl(tr("http://status.leagueoflegends.com/shards/na"))));  // GET NA SERVERS STATUS

    networkManager_featured = new QNetworkAccessManager(this);

    connect(networkManager_featured, SIGNAL(finished(QNetworkReply*)), this, SLOT(slot_networkResult_featured(QNetworkReply*)));

    slot_featuredRefresh();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::log(QString s){

    ui->statusBar->showMessage(QTime::currentTime().toString() + " | " + s);
    ui->textEdit->append(QTime::currentTime().toString() + " | " + s);
}

void MainWindow::slot_doubleclick_featured(int row,int column){
    QProcess::startDetached("cmd", QStringList() << "\"G://GAMES/LoL/RADS/solutions/lol_game_client_sln/releases/0.0.1.123/deploy/League of Legends.exe\" \"8394\" \"LoLLauncher.exe\" \"\" \"spectator spectator.na.lol.riotgames.com:80\" " + ui->tableWidget->selectedItems()[0]->text() + " " + ui->tableWidget->selectedItems()[0]->text() + " NA1");
}

void MainWindow::slot_networkResult_status(QNetworkReply *reply){

    if (reply->error() != QNetworkReply::NoError)
            return;

    QString data = (QString) reply->readAll();

    QJsonDocument jsonResponse = QJsonDocument::fromJson(data.toUtf8());

    if(jsonResponse.isEmpty()){
        ui->statusBar->showMessage(tr("Status : Error"));
        return;
    }

    QJsonObject jsonObject = jsonResponse.object();

    json_status.append(jsonObject);

    QJsonArray services = jsonObject.value(tr("services")).toArray();

    log(jsonObject.value(tr("name")).toString() + tr(" : Status server infos"));

    if(jsonObject.value(tr("slug")).toString() == tr("euw")){
        ui->tableWidget_status->setItem(0,0,new QTableWidgetItem(services[0].toObject().value(tr("status")).toString()));
        ui->tableWidget_status->setItem(0,1,new QTableWidgetItem(services[1].toObject().value(tr("status")).toString()));
        ui->tableWidget_status->setItem(0,2,new QTableWidgetItem(services[2].toObject().value(tr("status")).toString()));
        ui->tableWidget_status->setItem(0,3,new QTableWidgetItem(services[3].toObject().value(tr("status")).toString()));
    }
    if(jsonObject.value(tr("slug")).toString() == tr("na")){
        ui->tableWidget_status->setItem(1,0,new QTableWidgetItem(services[0].toObject().value(tr("status")).toString()));
        ui->tableWidget_status->setItem(1,1,new QTableWidgetItem(services[1].toObject().value(tr("status")).toString()));
        ui->tableWidget_status->setItem(1,2,new QTableWidgetItem(services[2].toObject().value(tr("status")).toString()));
        ui->tableWidget_status->setItem(1,3,new QTableWidgetItem(services[3].toObject().value(tr("status")).toString()));
    }
}

void MainWindow::slot_networkResult_featured(QNetworkReply *reply){

    if (reply->error() != QNetworkReply::NoError)
            return;

    QString data = (QString) reply->readAll();

    QJsonDocument jsonResponse = QJsonDocument::fromJson(data.toUtf8());

    if(jsonResponse.isEmpty()){
        ui->statusBar->showMessage(tr("Featured : Error"));
        return;
    }

    QJsonObject jsonObject = jsonResponse.object();

    json_featured.append(jsonObject);

    QJsonArray gamelist = jsonObject.value(tr("gameList")).toArray();

    if(gamelist.size() == 0){
        return;
    }

    log(gamelist[0].toObject().value(tr("platformId")).toString() + " : Featured games infos");

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

void MainWindow::slot_featuredRefresh(){
    while(ui->tableWidget->rowCount() > 0){
        ui->tableWidget->removeRow(0);
    }
    json_featured.clear();

    networkManager_featured->get(QNetworkRequest(QUrl(tr("http://spectator.euw1.lol.riotgames.com/observer-mode/rest/featured"))));  // GET EUW FEATURED GAMES

    networkManager_featured->get(QNetworkRequest(QUrl(tr("http://spectator.na.lol.riotgames.com/observer-mode/rest/featured"))));  // GET NA FEATURED GAMES
}

void MainWindow::slot_click_featured(int row, int column){
    ui->lineEdit_2->setText(ui->tableWidget->item(row, 0)->text());
    ui->lineEdit->setText(ui->tableWidget->item(row,1)->text());
    ui->lineEdit_3->setText(ui->tableWidget->item(row,2)->text());
}
