#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle(tr("OpenReplay Alpha 3.0"));

    log(QString("OpenReplay Alpha 3.0 Started"));

    loldirectory = "C:\\Program Files\\Riot\\League of Legends";

    servers.append(QStringList() << "EU West" << "EUW1" << "spectator.euw1.lol.riotgames.com:80");
    servers.append(QStringList() << "North America" << "NA1" << "spectator.na.lol.riotgames.com:80");

    connect(ui->pushButton_2, SIGNAL(released()), this, SLOT(slot_featuredRefresh()));
    connect(ui->tableWidget_featured, SIGNAL(cellClicked(int,int)), this, SLOT(slot_click_featured(int,int)));
    connect(ui->tableWidget_featured, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(slot_doubleclick_featured(int,int)));
    connect(ui->toolButton, SIGNAL(released()), this, SLOT(slot_setdirectory()));
    connect(ui->pushButton_featured_spectate, SIGNAL(released()), this, SLOT(slot_featuredLaunch()));

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
    QString serverid = ui->tableWidget_featured->item(row,0)->text();
    QString key = ui->tableWidget_featured->item(row,2)->text();
    QString matchid = ui->tableWidget_featured->item(row,1)->text();

    lol_launch(serverid,key,matchid);
}

void MainWindow::lol_launch(QString serverid, QString key, QString matchid){
    QString path;
    path = loldirectory + "\\RADS\\solutions\\lol_game_client_sln\\releases\\0.0.1.125\\deploy\\";

    if(!check_path(path)){
        QMessageBox::information(this, "OpenReplay", "Invalid League of Legends directory.\nPlease set a valid one.");
        return;
    }

    QString address;
    for(int i = 0; i < servers.size(); i++){
        if(servers.at(i).at(1) == serverid){
            address = servers.at(i).at(2);
            break;
        }
    }
    if(address.isEmpty()){
        //Server address not found
        return;
    }
    QProcess *process = new QProcess;
    process->setWorkingDirectory(path);
    process->startDetached("\"" + path + "League of Legends.exe\" \"8394\" \"LoLLauncher.exe\" \"\" \"spectator " + address + " " + key + " " + matchid + " " + serverid + "\"", QStringList(), path);
    log("\"" + path + "\\League of Legends.exe\" \"8394\" \"LoLLauncher.exe\" \"\" \"spectator " + address + " " + key + " " + matchid + " " + serverid + "\"");
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

    for(int i = 0; i < servers.size(); i++){
        if(jsonObject.value(tr("name")).toString() == servers.at(i).at(0)){
            for(int j = 0; j < 4; j++){
                ui->tableWidget_status->setItem(i,j,new QTableWidgetItem(services[j].toObject().value(tr("status")).toString()));
            }
            break;
        }
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
        ui->tableWidget_featured->insertRow(ui->tableWidget_featured->rowCount());

        QTableWidgetItem* item = new QTableWidgetItem();
        item->setText(gamelist[i].toObject().value(tr("platformId")).toString());

        ui->tableWidget_featured->setItem(ui->tableWidget_featured->rowCount()-1, 0, item);

        QTableWidgetItem* item2 = new QTableWidgetItem();

        item2->setText(QString::number(gamelist[i].toObject().value("gameId").toVariant().toULongLong()));

        ui->tableWidget_featured->setItem(ui->tableWidget_featured->rowCount()-1, 1, item2);

        QTableWidgetItem* item3 = new QTableWidgetItem();

        item3->setText(gamelist[i].toObject().value("observers").toObject().value("encryptionKey").toString());

        ui->tableWidget_featured->setItem(ui->tableWidget_featured->rowCount()-1, 2, item3);
    }

}

void MainWindow::slot_featuredRefresh(){
    while(ui->tableWidget_featured->rowCount() > 0){
        ui->tableWidget_featured->removeRow(0);
    }
    json_featured.clear();

    networkManager_featured->get(QNetworkRequest(QUrl(tr("http://spectator.euw1.lol.riotgames.com/observer-mode/rest/featured"))));  // GET EUW FEATURED GAMES

    networkManager_featured->get(QNetworkRequest(QUrl(tr("http://spectator.na.lol.riotgames.com/observer-mode/rest/featured"))));  // GET NA FEATURED GAMES
}

void MainWindow::slot_click_featured(int row, int column){
    ui->lineEdit_2->setText(ui->tableWidget_featured->item(row, 0)->text());
    ui->lineEdit->setText(ui->tableWidget_featured->item(row,1)->text());
    ui->lineEdit_3->setText(ui->tableWidget_featured->item(row,2)->text());
}

void MainWindow::slot_setdirectory(){
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),"/home",QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    loldirectory = dir;
    ui->lineEdit_4->setText(dir);
}

void MainWindow::slot_featuredLaunch(){
    if(ui->tableWidget_featured->selectedItems().size() == 0){
        return;
    }
    int row = ui->tableWidget_featured->currentRow();
    lol_launch(ui->tableWidget_featured->item(row,0)->text(),ui->tableWidget_featured->item(row,2)->text(),ui->tableWidget_featured->item(row,1)->text());
}

bool MainWindow::check_path(QString path){
    QFileInfo checkFile(path);
    return(checkFile.exists());
}
