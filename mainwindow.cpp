#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle(tr("OpenReplay Alpha 4.0"));

    log(QString("OpenReplay Alpha 4.0 Started"));

    QSettings settings("Riot Games", "RADS");

    QString rootfolder = settings.value("LocalRootFolder").toString();

    if(rootfolder.isEmpty()){
        loldirectory = "C:\\Program Files\\Riot\\League of Legends\\RADS";
    }
    else{
        loldirectory = rootfolder;
    }
    ui->lineEdit_4->setText(loldirectory);

    QSettings settings2(QSettings::UserScope, "Microsoft", "Windows");
    settings2.beginGroup("CurrentVersion/Explorer/Shell Folders");
    QString docfolder = settings.value("Personal").toString();

    if(docfolder.isEmpty()){
        replaydirectory = "C:\\";
    }
    else{
        replaydirectory = docfolder;
    }

    ui->lineEdit_replaysFolder->setText(replaydirectory);

    servers.append(QStringList() << "EU West" << "EUW1" << "spectator.euw1.lol.riotgames.com:80");
    servers.append(QStringList() << "EU Nordic & East" << "EUN1" << "spectator.eu.lol.riotgames.com:8088");
    servers.append(QStringList() << "North America" << "NA1" << "spectator.na.lol.riotgames.com:80");

    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(slot_changedTab(int)));
    connect(ui->pushButton_2, SIGNAL(released()), this, SLOT(slot_featuredRefresh()));
    connect(ui->tableWidget_featured, SIGNAL(cellClicked(int,int)), this, SLOT(slot_click_featured(int,int)));
    connect(ui->tableWidget_featured, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(slot_doubleclick_featured(int,int)));
    connect(ui->toolButton, SIGNAL(released()), this, SLOT(slot_setdirectory()));
    connect(ui->toolButton_2, SIGNAL(released()), this, SLOT(slot_setreplaydirectory()));
    connect(ui->pushButton_featured_spectate, SIGNAL(released()), this, SLOT(slot_featuredLaunch()));
    connect(ui->pushButton_featured_record, SIGNAL(released()), this, SLOT(slot_featuredRecord()));

    networkManager_status = new QNetworkAccessManager(this);
    connect(networkManager_status, SIGNAL(finished(QNetworkReply*)), this, SLOT(slot_networkResult_status(QNetworkReply*)));

    networkManager_status->get(QNetworkRequest(QUrl(tr("http://status.leagueoflegends.com/shards/euw"))));  // GET EUW SERVERS STATUS
    networkManager_status->get(QNetworkRequest(QUrl(tr("http://status.leagueoflegends.com/shards/eune"))));  // GET EUNE SERVERS STATUS
    networkManager_status->get(QNetworkRequest(QUrl(tr("http://status.leagueoflegends.com/shards/na"))));  // GET NA SERVERS STATUS

    networkManager_featured = new QNetworkAccessManager(this);
    connect(networkManager_featured, SIGNAL(finished(QNetworkReply*)), this, SLOT(slot_networkResult_featured(QNetworkReply*)));

    recording = false;
    networkManager_record = new QNetworkAccessManager(this);
    connect(networkManager_record, SIGNAL(finished(QNetworkReply*)), this, SLOT(slot_networkResult_record(QNetworkReply*)));

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

    ui->lineEdit_featured_player1->setText(participants.at(0).toObject().value("summonerName").toString());
    ui->lineEdit_featured_player2->setText(participants.at(1).toObject().value("summonerName").toString());
    ui->lineEdit_featured_player3->setText(participants.at(2).toObject().value("summonerName").toString());
    ui->lineEdit_featured_player4->setText(participants.at(3).toObject().value("summonerName").toString());
    ui->lineEdit_featured_player5->setText(participants.at(4).toObject().value("summonerName").toString());
    ui->lineEdit_featured_player6->setText(participants.at(5).toObject().value("summonerName").toString());
    ui->lineEdit_featured_player7->setText(participants.at(6).toObject().value("summonerName").toString());
    ui->lineEdit_featured_player8->setText(participants.at(7).toObject().value("summonerName").toString());
    ui->lineEdit_featured_player9->setText(participants.at(8).toObject().value("summonerName").toString());
    ui->lineEdit_featured_player10->setText(participants.at(9).toObject().value("summonerName").toString());
}

void MainWindow::slot_setdirectory(){
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),loldirectory,QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
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
    lol_launch(ui->tableWidget_featured->item(row,0)->text(),ui->tableWidget_featured->item(row,2)->text(),ui->tableWidget_featured->item(row,1)->text());
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

    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.get(QNetworkRequest(QUrl(QString("http://" + serveraddress + "/observer-mode/rest/consumer/getGameMetaData/" + serverid + "/" + gameid + "/token"))));

    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if(reply->error() != QNetworkReply::NoError){
        return false;
    }

    QString data = (QString) reply->readAll();

    QJsonDocument jsonResponse = QJsonDocument::fromJson(data.toUtf8());

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
        return jsonEmpty;
    }

    QString data = (QString) reply->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(data.toUtf8());

    return jsonResponse;
}

void MainWindow::slot_featuredRecord(){
    if(ui->tableWidget_featured->selectedItems().size() == 0){
        return;
    }
    int row = ui->tableWidget_featured->currentRow();
    record_featured_game(ui->tableWidget_featured->item(row,0)->text(),ui->tableWidget_featured->item(row,1)->text(),ui->tableWidget_featured->item(row,2)->text());
}

QByteArray MainWindow::getFileFromUrl(QString url){
    QNetworkAccessManager local_networkResult;
    QNetworkReply *reply = local_networkResult.get(QNetworkRequest(QUrl(url)));

    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if(reply->error() != QNetworkReply::NoError){
        QByteArray emptyArray;
        return emptyArray;
    }

    QByteArray data = reply->readAll();
    return data;
}

/*    Not complete zone start here    */

void MainWindow::record_featured_game(QString serverid, QString gameid, QString encryptionkey){
    //Get serverID
    QString serveraddress;
    for(int i = 0; i < servers.size(); i++){
        if(servers.at(i).at(1) == serverid){
            serveraddress = servers.at(i).at(2);
        }
    }
    if(serveraddress.size() == 0){
        return;
    }

    if(!recording){
        recording = true;
    }
    else{
        return;
    }

    log("Start recording : " + serverid + "/" + gameid);

    QString version;
    version = getFileFromUrl(QString("http://" + serveraddress + "/observer-mode/rest/consumer/version"));

    QByteArray gameinfo;
    for(int i = 0; i < json_featured.size(); i++){
        if(json_featured.at(i).value("gameList").toArray().first().toObject().value("platformId").toString() == serverid){
            QJsonArray gamelist = json_featured.at(i).value("gameList").toArray();
            for(int j = 0; j < gamelist.size(); j++){
                if(QString::number(gamelist.at(j).toObject().value("gameId").toVariant().toLongLong()) == gameid){
                    gameinfo = gamelist.at(j).toVariant().toByteArray();
                    break;
                }
            }
            break;
        }
    }

    QJsonDocument json_gameMetaData = getJsonFromUrl(QString("http://" + serveraddress + "/observer-mode/rest/consumer/getGameMetaData/" + serverid + "/" + gameid + "/token"));

    QList<QByteArray> list_bytearray_keyframes;
    QList<QByteArray> list_bytearray_gamedatachunks;

    int keyframeid = json_gameMetaData.object().value("pendingAvailableKeyFrameInfo").toArray().first().toObject().value("id").toInt();
    int lastsavedkeyframeid = -1;
    int chunkid = json_gameMetaData.object().value("pendingAvailableChunkInfo").toArray().first().toObject().value("id").toInt();
    int lastsavedchunkid = -1;

    QByteArray bytearray_keyframe, bytearray_chunk;

    QTimer *timer = new QTimer(this);
    QEventLoop loop;
    connect(timer, SIGNAL(timeout()), &loop, SLOT(quit()));

    while(lastsavedchunkid != json_gameMetaData.object().value("endGameChunkId").toInt() || lastsavedkeyframeid != json_gameMetaData.object().value("endGameKeyFrameId").toInt() || json_gameMetaData.object().value("endGameChunkId").toInt() == -1){
        if(json_gameMetaData.object().value("pendingAvailableKeyFrameInfo").toArray().last().toObject().value("id").toInt() > lastsavedkeyframeid){
            //Get a keyframe
            if(lastsavedkeyframeid == keyframeid || keyframeid < json_gameMetaData.object().value("pendingAvailableKeyFrameInfo").toArray().first().toObject().value("id").toInt()){
                keyframeid += 1;
            }
            bytearray_keyframe = getFileFromUrl(QString("http://" + serveraddress + "/observer-mode/rest/consumer/getKeyFrame/" + serverid + "/" + gameid + "/" + QString::number(keyframeid) + "/0"));
            if(!bytearray_keyframe.isEmpty()){
                list_bytearray_keyframes.append(bytearray_keyframe);
                lastsavedkeyframeid = keyframeid;
                log("Keyframe : " + QString::number(keyframeid));
            }
        }

        if(json_gameMetaData.object().value("pendingAvailableChunkInfo").toArray().last().toObject().value("id").toInt() > lastsavedchunkid){
            //Get a chunk
            if(lastsavedchunkid == chunkid || chunkid < json_gameMetaData.object().value("pendingAvailableChunkInfo").toArray().first().toObject().value("id").toInt()){
                chunkid += 1;
            }
            bytearray_chunk = getFileFromUrl(QString("http://" + serveraddress + "/observer-mode/rest/consumer/getGameDataChunk/" + serverid + "/" + gameid + "/" + QString::number(chunkid) + "/0"));
            if(!bytearray_chunk.isEmpty()){
                list_bytearray_gamedatachunks.append(bytearray_chunk);
                lastsavedchunkid = chunkid;
                log("Chunk : " + QString::number(chunkid));
            }
        }

        json_gameMetaData = getJsonFromUrl(QString("http://" + serveraddress + "/observer-mode/rest/consumer/getGameMetaData/" + serverid + "/" + gameid + "/token"));
        if(json_gameMetaData.isEmpty()){
            break;
        }

        //Retry every 5 seconds
        timer->start(5000);
        loop.exec();
    }

    recording = false;
    log("End of recording : " + serverid + "/" + gameid);

    //Save all chunks, infos and keyframes in a file

    QFile file("H:\\OpenReplays\\" + serverid + "-" + gameid + ".lor");

    if(file.open(QIODevice::WriteOnly)){
        QTextStream stream(&file);

        stream << "::OpenReplayInfos:" << serverid << ":" << gameid << ":" << encryptionkey << ":" << version << "::" << endl;

        if(!gameinfo.isEmpty()){
            stream << "::OpenReplayGameInfos::" << gameinfo.toBase64() << endl;
        }

        int first_keyframeid = json_gameMetaData.object().value("endGameKeyFrameId").toInt() - list_bytearray_keyframes.size() + 1;
        int first_chunkid = json_gameMetaData.object().value("endGameChunkId").toInt() - list_bytearray_gamedatachunks.size() + 1;

        for(int i = 0; i < list_bytearray_keyframes.size(); i++){
            stream << "::OpenReplayKeyFrame:" << QString::number(first_keyframeid + i) << "::";
            stream << list_bytearray_keyframes.at(i).toBase64() << endl;
        }
        for(int i = 0; i < list_bytearray_gamedatachunks.size(); i++){
            stream << "::OpenReplayChunk:" << QString::number(first_chunkid + i) << "::";
            stream << list_bytearray_gamedatachunks.at(i).toBase64() << endl;
        }
        stream << "::OpenReplayEnd::";
        file.close();

        log("Replay file created");
    }
    else{
        log("Error saving replay.");
    }

}

void MainWindow::slot_networkResult_record(QNetworkReply *reply){
    if (reply->error() != QNetworkReply::NoError)
            return;

    if(!recording){
        recording = true;

    }
    else{

    }
}
