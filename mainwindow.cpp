#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "recorder.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle(tr("OpenReplay Alpha 6.0.0"));

    log(QString("OpenReplay Alpha 6.0.0 Started"));

    ui->lineEdit_status->setText("Starting");

    QSettings settings("Riot Games", "RADS");

    QString rootfolder = settings.value("LocalRootFolder").toString();

    if(rootfolder.isEmpty()){
        loldirectory = "C:\\Program Files\\Riot\\League of Legends\\RADS";
    }
    else{
        loldirectory = rootfolder;
    }
    ui->lineEdit_4->setText(loldirectory);

    QStringList folders = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
    if(folders.isEmpty()){
        //Error : no documents location found on the system
        return;
    }

    if(!QDir(folders.first() + "/OpenReplays").exists()){
        QDir().mkpath(folders.first() + "/OpenReplays");
    }

    replaydirectory = folders.first() + "/OpenReplays";

    ui->lineEdit_replaysFolder->setText(replaydirectory);

    refresh_recordedGames();

    replaying = false;
    playing = false;

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slot_refreshPlayingStatus()));
    m_timer->start(60000);

    //Add servers

    servers.append(QStringList() << "EU West" << "EUW1" << "spectator.euw1.lol.riotgames.com:80" << "EUW");
    servers.append(QStringList() << "EU Nordic & East" << "EUN1" << "spectator.eu.lol.riotgames.com:8088" << "EUNE");
    servers.append(QStringList() << "North America" << "NA1" << "spectator.na.lol.riotgames.com:80" << "NA");

    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(slot_changedTab(int)));
    connect(ui->pushButton_2, SIGNAL(released()), this, SLOT(slot_featuredRefresh()));
    connect(ui->tableWidget_featured, SIGNAL(cellClicked(int,int)), this, SLOT(slot_click_featured(int,int)));
    connect(ui->tableWidget_featured, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(slot_doubleclick_featured(int,int)));
    connect(ui->toolButton, SIGNAL(released()), this, SLOT(slot_setdirectory()));
    connect(ui->toolButton_2, SIGNAL(released()), this, SLOT(slot_setreplaydirectory()));
    connect(ui->pushButton_featured_spectate, SIGNAL(released()), this, SLOT(slot_featuredLaunch()));
    connect(ui->pushButton_featured_record, SIGNAL(released()), this, SLOT(slot_featuredRecord()));
    connect(ui->pushButton_add_replayserver, SIGNAL(released()), this, SLOT(slot_replayserversAdd()));
    connect(ui->pushButton_summonersave, SIGNAL(released()), this, SLOT(slot_summonerinfos_save()));

    networkManager_status = new QNetworkAccessManager(this);
    connect(networkManager_status, SIGNAL(finished(QNetworkReply*)), this, SLOT(slot_networkResult_status(QNetworkReply*)));

    networkManager_status->get(QNetworkRequest(QUrl(tr("http://status.leagueoflegends.com/shards/euw"))));  // GET EUW SERVERS STATUS
    networkManager_status->get(QNetworkRequest(QUrl(tr("http://status.leagueoflegends.com/shards/eune"))));  // GET EUNE SERVERS STATUS
    networkManager_status->get(QNetworkRequest(QUrl(tr("http://status.leagueoflegends.com/shards/na"))));  // GET NA SERVERS STATUS

    networkManager_featured = new QNetworkAccessManager(this);
    connect(networkManager_featured, SIGNAL(finished(QNetworkReply*)), this, SLOT(slot_networkResult_featured(QNetworkReply*)));

    slot_featuredRefresh();

    ui->lineEdit_status->setText("Idle");
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
    Q_UNUSED(column);

    QString serverid = ui->tableWidget_featured->item(row,0)->text();
    QString key = ui->tableWidget_featured->item(row,2)->text();
    QString gameid = ui->tableWidget_featured->item(row,1)->text();

    if(game_ended(serverid, gameid)){
        return;
    }

    lol_launch(serverid, key, gameid);
}

void MainWindow::lol_launch(QString serverid, QString key, QString matchid){
    QString path;

    QDir qd;
    qd.setPath(loldirectory + "\\solutions\\lol_game_client_sln\\releases\\");
    qd.setFilter(QDir::Dirs);
    qd.setSorting(QDir::Name | QDir::Reversed);
    QFileInfoList list = qd.entryInfoList();

    if(list.size()==0){
        QMessageBox::information(this, "OpenReplay", "Invalid League of Legends directory.\nPlease set a valid one.");
        return;
    }

    path = loldirectory + "\\solutions\\lol_game_client_sln\\releases\\" + list.at(0).fileName() + "\\deploy\\";

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

    networkManager_featured->get(QNetworkRequest(QUrl(tr("http://spectator.eu.lol.riotgames.com:8088/observer-mode/rest/featured"))));  // GET EUNE FEATURED GAMES

    networkManager_featured->get(QNetworkRequest(QUrl(tr("http://spectator.na.lol.riotgames.com/observer-mode/rest/featured"))));  // GET NA FEATURED GAMES
}

void MainWindow::slot_click_featured(int row, int column){
    Q_UNUSED(column);

    QString gameid = ui->tableWidget_featured->item(row,1)->text();

    ui->lineEdit_featured_serverid->setText(ui->tableWidget_featured->item(row, 0)->text());
    ui->lineEdit_featured_gameid->setText(ui->tableWidget_featured->item(row,1)->text());
    ui->lineEdit_featured_key->setText(ui->tableWidget_featured->item(row,2)->text());

    QJsonObject game;
    for(int i = 0; i < json_featured.size(); i++){
        QJsonArray gamelist = json_featured.at(i).value(tr("gameList")).toArray();
        for(int j = 0; j < gamelist.size(); j++){
            if(QString::number(gamelist.at(j).toObject().value("gameId").toVariant().toULongLong()) == gameid){
                game = gamelist.at(j).toObject();
            }
        }
    }

    if(game.isEmpty()){
        return;
    }

    QJsonArray participants = game.value("participants").toArray();

    if(participants.size()!=10){
        return;
    }

    //Show game infos
}

void MainWindow::slot_setdirectory(){
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open RADS Directory"),loldirectory,QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(dir.isEmpty()){
        return;
    }
    loldirectory = dir;
    ui->lineEdit_4->setText(dir);
}

void MainWindow::slot_setreplaydirectory(){
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),replaydirectory,QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(dir.isEmpty()){
        return;
    }
    replaydirectory = dir;
    ui->lineEdit_replaysFolder->setText(dir);
}

void MainWindow::slot_featuredLaunch(){
    if(ui->tableWidget_featured->selectedItems().size() == 0){
        return;
    }
    int row = ui->tableWidget_featured->currentRow();

    slot_refreshPlayingStatus();

    if(playing){
        log("Replay aborted. LoL is currently running.");
        return;
    }

    lol_launch(ui->tableWidget_featured->item(row,0)->text(),ui->tableWidget_featured->item(row,2)->text(),ui->tableWidget_featured->item(row,1)->text());

    replaying = true;
}

bool MainWindow::check_path(QString path){
    QFileInfo checkFile(path);
    return(checkFile.exists());
}

void MainWindow::slot_changedTab(int index){
    if(index == 2){
        slot_featuredRefresh();
    }
}

bool MainWindow::game_ended(QString serverid, QString gameid){
    //Get serverID
    QString serveraddress;
    for(int i = 0; i < servers.size(); i++){
        if(servers.at(i).at(1) == serverid){
            serveraddress = servers.at(i).at(2);
        }
    }
    if(serveraddress.size() == 0){
        return false;
    }

    QJsonDocument jsonResponse = getJsonFromUrl(QString("http://" + serveraddress + "/observer-mode/rest/consumer/getGameMetaData/" + serverid + "/" + gameid + "/token"));

    if(jsonResponse.isEmpty()){
        return false;
    }

    QJsonObject jsonObject = jsonResponse.object();

    if(jsonObject.value("gameEnded").toBool()){
        log("GAME : " + serverid + "/" + gameid + " has already finished. Aborting spectator mode.");
        return true;
    }
    else{
        log("GAME : " + serverid + "/" + gameid + " is in progress. Launching spectator mode.");
        return false;
    }
}

QJsonDocument MainWindow::getJsonFromUrl(QString url){
    QNetworkAccessManager local_networkResult;
    QNetworkReply *reply = local_networkResult.get(QNetworkRequest(QUrl(url)));

    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if(reply->error() != QNetworkReply::NoError){
        QJsonDocument jsonEmpty;
        log("Error : " + reply->errorString());
        return jsonEmpty;
    }

    QString data = (QString) reply->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(data.toUtf8());
    reply->deleteLater();

    return jsonResponse;
}

void MainWindow::slot_featuredRecord(){
    if(ui->tableWidget_featured->selectedItems().size() == 0){
        return;
    }
    int row = ui->tableWidget_featured->currentRow();

    QString serverid = ui->tableWidget_featured->item(row,0)->text();
    QString gameid = ui->tableWidget_featured->item(row,1)->text();

    //Get server address
    QString serveraddress;
    for(int i = 0; i < servers.size(); i++){
        if(servers.at(i).at(1) == serverid){
            serveraddress = servers.at(i).at(2);
            break;
        }
    }
    if(serveraddress.size() == 0){
        return;
    }

    for(int i = 0; i < recording.size(); i++){
        if(recording.at(i).at(0) == serverid && recording.at(i).at(1) == gameid){
            //Game is already recording
            return;
        }
    }

    recording.append(QStringList() << serverid << gameid);

    ui->lineEdit_status->setText("Recording " + QString::number(recording.size()) + " games");

    QJsonDocument gameinfo;
    for(int i = 0; i < json_featured.size(); i++){
        if(json_featured.at(i).value("gameList").toArray().first().toObject().value("platformId").toString() == serverid){
            QJsonArray gamelist = json_featured.at(i).value("gameList").toArray();
            for(int j = 0; j < gamelist.size(); j++){
                if(QString::number(gamelist.at(j).toObject().value("gameId").toVariant().toULongLong()) == gameid){
                    gameinfo = QJsonDocument(gamelist.at(j).toObject());
                    break;
                }
            }
            break;
        }
    }

    Recorder *recorder = new Recorder(this, serverid, serveraddress, gameid, ui->tableWidget_featured->item(row,2)->text(), gameinfo, replaydirectory);
    connect(recorder, SIGNAL(end(QString,QString)), this, SLOT(slot_endRecording(QString,QString)));
    connect(recorder, SIGNAL(finished()), recorder, SLOT(deleteLater()));
    recorder->start();
}

void MainWindow::slot_endRecording(QString serverid, QString gameid){
    for(int i = 0; i < recording.size(); i++){
        if(recording.at(i).at(0) == serverid && recording.at(i).at(1) == gameid){
            recording.removeAt(i);
        }
    }

    if(recording.isEmpty()){
        ui->lineEdit_status->setText("Idle");
    }
    else{
        ui->lineEdit_status->setText("Recording " + QString::number(recording.size()) + " games");
    }

    refresh_recordedGames();
}

QByteArray MainWindow::getFileFromUrl(QString url){
    QNetworkAccessManager local_networkResult;
    QNetworkReply *reply = local_networkResult.get(QNetworkRequest(QUrl(url)));

    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if(reply->error() != QNetworkReply::NoError){
        QByteArray emptyArray;
        log("Error : " + reply->errorString());
        return emptyArray;
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();
    return data;
}

void MainWindow::refresh_recordedGames(){
    //Find saved replays

    QDir dirreplays(replaydirectory);
    dirreplays.setFilter(QDir::Files);
    dirreplays.setSorting(QDir::Time | QDir::Reversed);

    QFileInfoList replayslist = dirreplays.entryInfoList();

    while(ui->tableWidget_recordedgames->rowCount() > 0){
        ui->tableWidget_recordedgames->removeRow(0);
    }

    for(int i = 0; i < replayslist.size(); i++){
        QFileInfo fileinfo = replayslist.at(i);
        ui->tableWidget_recordedgames->insertRow(ui->tableWidget_recordedgames->rowCount());
        QTableWidgetItem *item = new QTableWidgetItem;
        item->setText(fileinfo.fileName());
        ui->tableWidget_recordedgames->setItem(ui->tableWidget_recordedgames->rowCount()-1, 3, item);
    }
}

void MainWindow::slot_replayserversAdd(){
    if(ui->lineEdit_replayserver_address->text().isEmpty()){
        return;
    }

    orservers.append(ui->lineEdit_replayserver_address->text());

    ui->tableWidget_replayservers->insertRow(ui->tableWidget_replayservers->rowCount());
    ui->tableWidget_replayservers->setItem(ui->tableWidget_replayservers->rowCount()-1,0,new QTableWidgetItem(ui->lineEdit_replayserver_address->text()));
}

void MainWindow::slot_summonerinfos_save(){
    if(ui->lineEdit_summonername->text().isEmpty() || orservers.isEmpty()){
        return;
    }

    m_summonername = ui->lineEdit_summonername->text();
    m_summonerserver = ui->comboBox_summonerserver->currentText();

    //Retrieving summoner ID

    QJsonDocument suminfos = getJsonFromUrl("http://" + orservers.first() + "?region=" + m_summonerserver + "&summonername=" + m_summonername);

    log("Retrieving summoner ID");

    if(suminfos.isEmpty()){
       return;
    }
    else{
        m_summonerid = QString::number(suminfos.object().value(suminfos.object().keys().first()).toObject().value("id").toVariant().toLongLong());
        ui->lineEdit_summonerid->setText(m_summonerid);
        log("Your summoner ID is " + m_summonerid);
    }
}

bool MainWindow::islolRunning() {
  QProcess tasklist;

  tasklist.start("tasklist", QStringList() << "/NH" << "/FO" << "CSV" << "/FI" << QString("IMAGENAME eq League of Legends.exe"));
  tasklist.waitForFinished();

  QString output = tasklist.readAllStandardOutput();
  return output.startsWith(QString("\"League of Legends.exe\""));
}

bool MainWindow::islolclientRunning() {
  QProcess tasklist;

  tasklist.start("tasklist", QStringList() << "/NH" << "/FO" << "CSV" << "/FI" << QString("IMAGENAME eq LolClient.exe"));
  tasklist.waitForFinished();

  QString output = tasklist.readAllStandardOutput();
  return output.startsWith(QString("\"LolClient.exe\""));
}

void MainWindow::slot_refreshPlayingStatus(){
    if(!islolRunning()){
        replaying = false;
        playing = false;
    }

    if(!replaying && islolclientRunning() && islolRunning()){
        if(!playing){
            //Start recording the game
            playing = true;

            log("Game detected : start recording");

            QJsonDocument gameinfos = getCurrentPlayingGameInfos(m_summonerserver, m_summonerid);

            QString serverid, serveraddress;
            QString gameid = QString::number(gameinfos.object().value("gameId").toVariant().toLongLong());

            for(int i = 0; i < servers.size(); i++){
                if(servers.at(i).at(3) == m_summonerserver){
                    serverid = servers.at(i).at(1);
                    serveraddress = servers.at(i).at(2);
                    break;
                }
            }

            if(serverid.isEmpty()){
                return;
            }

            recording.append(QStringList() << serverid << gameid);

            ui->lineEdit_status->setText("Recording " + QString::number(recording.size()) + " games");

            Recorder *recorder = new Recorder(this, serverid, serveraddress, gameid, gameinfos.object().value("observers").toObject().value("encryptionKey").toString(), gameinfos, replaydirectory);
            connect(recorder, SIGNAL(end(QString,QString)), this, SLOT(slot_endRecording(QString,QString)));
            connect(recorder, SIGNAL(finished()), recorder, SLOT(deleteLater()));
            recorder->start();
        }
    }

    m_timer->start(60000);
}

QJsonDocument MainWindow::getCurrentPlayingGameInfos(QString server, QString summonerid){
    QString servertag;

    for(int i = 0; i < servers.size(); i++){
        if(servers.at(i).at(3) == server){
            servertag = servers.at(i).at(1);
            break;
        }
    }

    if(servertag.isEmpty() || orservers.isEmpty()){
        QJsonDocument docempty;
        return docempty;
    }

    QJsonDocument gameinfos = getJsonFromUrl("http://" + orservers.first() + "?platformid=" + servertag + "&summonerid=" + summonerid);

    return gameinfos;
}
