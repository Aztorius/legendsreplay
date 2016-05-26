#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "recorder.h"
#include "replay.h"

QString GLOBAL_VERSION = "1.2.0";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle(tr("LegendsReplay ") + GLOBAL_VERSION);
    setWindowIcon(QIcon(":/logo.png"));

    log(QString("LegendsReplay " + GLOBAL_VERSION + " Started"));

    ui->lineEdit_status->setText("Starting");

    orservers.append("informaticien77.serveminecraft.net/legendsreplay.php");

    ui->tableWidget_replayservers->insertRow(ui->tableWidget_replayservers->rowCount());
    ui->tableWidget_replayservers->setItem(ui->tableWidget_replayservers->rowCount()-1,0,new QTableWidgetItem("informaticien77.serveminecraft.net/legendsreplay.php"));

    orsettings = new QSettings("LegendsReplay", "Local");

    if(!orsettings->value("SummonerName").toString().isEmpty()){
        m_summonername = orsettings->value("SummonerName").toString();
        ui->lineEdit_summonername->setText(m_summonername);
    }
    else{
        QMessageBox::information(this, "LegendsReplay", "Please set your summoner name to record your games.");
        log("[WARN] Please set your summoner name to record your games.");
    }

    if(!orsettings->value("SummonerId").toString().isEmpty()){
        m_summonerid = orsettings->value("SummonerId").toString();
        ui->lineEdit_summonerid->setText(m_summonerid);
    }

    if(!orsettings->value("SummonerServer").toString().isEmpty()){
        m_summonerserver = orsettings->value("SummonerServer").toString();
        ui->comboBox_summonerserver->setCurrentText(m_summonerserver);
    }

    QSettings lolsettings("Riot Games", "RADS");

    QString rootfolder = lolsettings.value("LocalRootFolder").toString();

    if(!orsettings->value("LoLDirectory").toString().isEmpty()){
        loldirectory = orsettings->value("LoLDirectory").toString();
    }
    else if(rootfolder.isEmpty()){
        loldirectory = "C:\\Program Files\\Riot Games\\League of Legends\\RADS";
    }
    else{
        loldirectory = rootfolder;
    }
    ui->lineEdit_4->setText(loldirectory);

    if(!orsettings->value("LoLPBEDirectory").toString().isEmpty()){
        lolpbedirectory = orsettings->value("LoLPBEDirectory").toString();
        ui->lineEdit_pbefolder->setText(lolpbedirectory);
    }

    QString docfolder;
    if(orsettings->value("ReplayDirectory").toString().isEmpty()){
        QStringList folders = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
        if(folders.isEmpty()){
           //Error : no documents location found on the system
            log("Critical: no documents location found on the system");
            return;
        }
        docfolder = folders.first();
    }
    else{
        docfolder = orsettings->value("ReplayDirectory").toString();
    }

    if(!QDir(docfolder + "/LegendsReplay").exists()){
        QDir().mkpath(docfolder + "/LegendsReplay");
    }

    replaydirectory = docfolder + "/LegendsReplay";

    ui->lineEdit_replaysFolder->setText(replaydirectory);

    refresh_recordedGames();

    replaying = false;
    playing = false;

    replay = NULL;

    serverChunkCount = 0;
    serverKeyframeCount = 1;

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slot_refreshPlayingStatus()));
    m_timer->start(60000);

    //Add servers

    servers.append(QStringList() << "EU West" << "EUW1" << "spectator.euw1.lol.riotgames.com:80" << "EUW");
    servers.append(QStringList() << "EU Nordic & East" << "EUN1" << "spectator.eu.lol.riotgames.com:8088" << "EUNE");
    servers.append(QStringList() << "North America" << "NA1" << "spectator.na.lol.riotgames.com:80" << "NA");
    servers.append(QStringList() << "Japan" << "JP1" << "spectator.jp1.lol.riotgames.com:80" << "JP");
    servers.append(QStringList() << "Republic of Korea" << "KR" << "spectator.kr.lol.riotgames.com:80" << "KR");
    servers.append(QStringList() << "Oceania" << "OC1" << "spectator.oc1.lol.riotgames.com:80" << "OCE");
    servers.append(QStringList() << "Brazil" << "BR1" << "spectator.br.lol.riotgames.com:80" << "BR");
    servers.append(QStringList() << "Latin America North" << "LA1" << "spectator.la1.lol.riotgames.com:80" << "LAN");
    servers.append(QStringList() << "Latin America South" << "LA2" << "spectator.la2.lol.riotgames.com:80" << "LAS");
    servers.append(QStringList() << "Russia" << "RU" << "spectator.ru.lol.riotgames.com:80" << "RU");
    servers.append(QStringList() << "Turkey" << "TR1" << "spectator.tr.lol.riotgames.com:80" << "TR");
    servers.append(QStringList() << "PBE" << "PBE1" << "spectator.pbe1.lol.riotgames.com:8088" << "PBE");

    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(slot_changedTab(int)));
    connect(ui->pushButton_2, SIGNAL(released()), this, SLOT(slot_featuredRefresh()));
    connect(ui->tableWidget_featured, SIGNAL(itemSelectionChanged()), this, SLOT(slot_click_featured()));
    connect(ui->tableWidget_featured, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(slot_doubleclick_featured(int,int)));
    connect(ui->toolButton, SIGNAL(released()), this, SLOT(slot_setdirectory()));
    connect(ui->toolButton_2, SIGNAL(released()), this, SLOT(slot_setreplaydirectory()));
    connect(ui->toolButton_pbefolder, SIGNAL(released()), this, SLOT(slot_setpbedirectory()));
    connect(ui->pushButton_featured_spectate, SIGNAL(released()), this, SLOT(slot_featuredLaunch()));
    connect(ui->pushButton_featured_record, SIGNAL(released()), this, SLOT(slot_featuredRecord()));
    connect(ui->pushButton_add_replayserver, SIGNAL(released()), this, SLOT(slot_replayserversAdd()));
    connect(ui->pushButton_summonersave, SIGNAL(released()), this, SLOT(slot_summonerinfos_save()));
    connect(ui->pushButton_searchsummoner, SIGNAL(released()), this, SLOT(slot_searchsummoner()));
    connect(ui->pushButton_searchsummoner_spectate, SIGNAL(released()), this, SLOT(slot_click_searchsummoner_spectate()));
    connect(ui->pushButton_searchsummoner_record, SIGNAL(released()), this, SLOT(slot_click_searchsummoner_record()));

    networkManager_status = new QNetworkAccessManager(this);
    connect(networkManager_status, SIGNAL(finished(QNetworkReply*)), this, SLOT(slot_networkResult_status(QNetworkReply*)));

    slot_statusRefresh();
    connect(ui->pushButton_statusRefresh, SIGNAL(pressed()), this, SLOT(slot_statusRefresh()));

    networkManager_featured = new QNetworkAccessManager(this);
    connect(networkManager_featured, SIGNAL(finished(QNetworkReply*)), this, SLOT(slot_networkResult_featured(QNetworkReply*)));

    connect(ui->tableWidget_recordedgames, SIGNAL(itemSelectionChanged()), this, SLOT(slot_click_allgames()));

    httpserver = new QHttpServer(this);
    connect(ui->tableWidget_recordedgames, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(slot_doubleclick_savedgames(int,int)));
    connect(ui->tableWidget_yourgames, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(slot_doubleclick_mygames(int,int)));

    connect(ui->actionOpen, SIGNAL(triggered(bool)), this, SLOT(slot_open_replay(bool)));

    systemtrayavailable = QSystemTrayIcon::isSystemTrayAvailable();

    if(systemtrayavailable){
        systemtrayicon = new QSystemTrayIcon;
        systemtrayicon->setIcon(QIcon(":/logo.png"));
        systemtrayicon->show();
    }

    QJsonDocument updatejson = getJsonFromUrl("http://aztorius.github.io/legendsreplay/version.json");

    if(!updatejson.isEmpty() && updatejson.object().value("version").toString() != GLOBAL_VERSION)
    {
        showmessage("New version "+ updatejson.object().value("version").toString() + " available !");
        log("<a href='http://aztorius.github.io/legendsreplay/'>New version "+ updatejson.object().value("version").toString() + " available !</a>");
    }

    ui->lineEdit_status->setText("Idle");
}

MainWindow::~MainWindow()
{
    if(systemtrayavailable)
    {
        systemtrayicon->hide();
        systemtrayicon->deleteLater();
    }

    httpserver->deleteLater();
    networkManager_featured->deleteLater();
    networkManager_status->deleteLater();
    m_timer->deleteLater();
    orsettings->deleteLater();    

    delete replay;
    delete ui;
}

void MainWindow::log(QString s)
{
    ui->statusBar->showMessage(QTime::currentTime().toString() + " | " + s);
    ui->textBrowser->append(QTime::currentTime().toString() + " | " + s);
}

void MainWindow::setArgs(int argc, char *argv[])
{
    if(argc > 1){
        if(std::string(argv[1]) == "--silent"){
            this->showMinimized();
            return;
        }

        Replay replay(argv[1]);
        if(!replay.getGameid().isEmpty()){
            replay_launch(argv[1]);
            return;
        }
    }
}

void MainWindow::slot_statusRefresh()
{
    ui->tableWidget_status->clearContents();

    networkManager_status->get(QNetworkRequest(QUrl(tr("http://status.leagueoflegends.com/shards/euw"))));  // GET EUW SERVERS STATUS
    networkManager_status->get(QNetworkRequest(QUrl(tr("http://status.leagueoflegends.com/shards/eune"))));  // GET EUNE SERVERS STATUS
    networkManager_status->get(QNetworkRequest(QUrl(tr("http://status.leagueoflegends.com/shards/na"))));  // GET NA SERVERS STATUS
    networkManager_status->get(QNetworkRequest(QUrl(tr("http://status.leagueoflegends.com/shards/jp"))));  // GET JP SERVERS STATUS
    networkManager_status->get(QNetworkRequest(QUrl(tr("http://status.leagueoflegends.com/shards/kr"))));  // GET KR SERVERS STATUS
    networkManager_status->get(QNetworkRequest(QUrl(tr("http://status.leagueoflegends.com/shards/oce"))));  // GET OCE SERVERS STATUS
    networkManager_status->get(QNetworkRequest(QUrl(tr("http://status.leagueoflegends.com/shards/br"))));  // GET BR SERVERS STATUS
    networkManager_status->get(QNetworkRequest(QUrl(tr("http://status.leagueoflegends.com/shards/lan"))));  // GET LAN SERVERS STATUS
    networkManager_status->get(QNetworkRequest(QUrl(tr("http://status.leagueoflegends.com/shards/las"))));  // GET LAS SERVERS STATUS
    networkManager_status->get(QNetworkRequest(QUrl(tr("http://status.leagueoflegends.com/shards/ru"))));  // GET RU SERVERS STATUS
    networkManager_status->get(QNetworkRequest(QUrl(tr("http://status.leagueoflegends.com/shards/tr"))));  // GET TR SERVERS STATUS
    networkManager_status->get(QNetworkRequest(QUrl(tr("http://status.pbe.leagueoflegends.com/shards/pbe"))));  // GET PBE SERVERS STATUS

    for(int i = 0; i < servers.size(); i++){
        networkManager_status->get(QNetworkRequest(QUrl(tr("http://") + servers.at(i).at(2) + tr("/observer-mode/rest/consumer/version"))));
    }
}

void MainWindow::slot_doubleclick_featured(int row,int column)
{
    Q_UNUSED(column);

    QString serverid = ui->tableWidget_featured->item(row,0)->text();
    QString key = ui->tableWidget_featured->item(row,2)->text();
    QString gameid = ui->tableWidget_featured->item(row,1)->text();

    if(game_ended(serverid, gameid)){
        log("Game " + serverid + "/" + gameid + " has already ended");
        return;
    }

    lol_launch(serverid, key, gameid);
}

void MainWindow::lol_launch(QString serverid, QString key, QString matchid, bool local)
{
    if(serverid.isEmpty() || key.isEmpty() || matchid.isEmpty()){
        log("Invalid game parameters.");
        return;
    }

    QString path;

    QDir qd;

    if(serverid == "PBE1"){
        qd.setPath(lolpbedirectory + "/solutions/lol_game_client_sln/releases/");
    }
    else{
        qd.setPath(loldirectory + "/solutions/lol_game_client_sln/releases/");
    }

    qd.setFilter(QDir::Dirs);
    qd.setSorting(QDir::Name | QDir::Reversed);
    QFileInfoList list = qd.entryInfoList();

    if(list.isEmpty()){
        QMessageBox::information(this, "LegendsReplay", "Invalid League of Legends directory.\nPlease set a valid one.");
        log("[WARN] Invalid League of Legends directory. No release folder found.");
        return;
    }

    if(serverid == "PBE1"){
        path = lolpbedirectory + "/solutions/lol_game_client_sln/releases/" + list.at(0).fileName() + "/deploy/";
    }
    else{
        path = loldirectory + "/solutions/lol_game_client_sln/releases/" + list.at(0).fileName() + "/deploy/";
    }

    if(!check_path(path)){
        QMessageBox::information(this, "LegendsReplay", "Invalid League of Legends directory.\nPlease set a valid one.");
        log("[WARN] Invalid League of Legends directory. Invalid path.");
        return;
    }

    QString address;

    if(local){
        address = "127.0.0.1:8088";

#ifdef WIN32

        QProcess *process = new QProcess;
        process->setWorkingDirectory(path);
        process->startDetached("\"" + path + "League of Legends.exe\"", QStringList() << "\"8394\"" << "\"LoLLauncher.exe\"" << "\"\"" << ("spectator " + address + " " + key + " " + matchid + " " + serverid), path);

        log("\"" + path + "League of Legends.exe\" \"8394\" \"LoLLauncher.exe\" \"\" \"spectator " + address + " " + key + " " + matchid + " " + serverid + "\"");

#else
        //QProcess *process = new QProcess;
        //process->setWorkingDirectory(path);
        //process->startDetached("playonlinux \"./solutions/lol_game_client_sln/releases/0.0.1.131/deploy/League of Legends.exe\"", QStringList() << "\"8394\"" << "\"LoLLauncher.exe\"" << "\"\"" << ("spectator " + address + " " + key + " " + matchid + " " + serverid), "/home/informaticien77/.PlayOnLinux/wineprefix/LeagueOfLegends/drive_c/Riot Games/League of Legends/RADS");

        //log("wine \"./solutions/lol_game_client_sln/releases/0.0.1.131/deploy/League of Legends.exe\" \"8394\" \"LoLLauncher.exe\" \"\" \"spectator " + address + " " + key + " " + matchid + " " + serverid + "\"");
        log("LoL Launch isn't available for Linux yet");

#endif

    }
    else{
        for(int i = 0; i < servers.size(); i++){
            if(servers.at(i).at(1) == serverid){
                address = servers.at(i).at(2);
                break;
            }
        }

        if(address.isEmpty()){
            //Server address not found
            log("Error: Server address not found");
            return;
        }

#ifdef WIN32

        QProcess *process = new QProcess;
        process->setWorkingDirectory(path);
        process->startDetached("\"" + path + "League of Legends.exe\"", QStringList() << "\"8394\"" << "\"LoLLauncher.exe\"" << "\"\"" << ("spectator " + address + " " + key + " " + matchid + " " + serverid), path);

        log("\"" + path + "League of Legends.exe\" \"8394\" \"LoLLauncher.exe\" \"\" \"spectator " + address + " " + key + " " + matchid + " " + serverid + "\"");

#else

        //QProcess *process = new QProcess;
        //process->setWorkingDirectory(path);
        //process->startDetached("wine \"" + path + "League of Legends.exe\"", QStringList() << "\"8394\"" << "\"LoLLauncher.exe\"" << "\"\"" << ("spectator " + address + " " + key + " " + matchid + " " + serverid), path);

        //log("wine \"" + path + "League of Legends.exe\" \"8394\" \"LoLLauncher.exe\" \"\" \"spectator " + address + " " + key + " " + matchid + " " + serverid + "\"");
        log("LoL Launch isn't available for Linux yet");

#endif

    }
}

void MainWindow::slot_networkResult_status(QNetworkReply *reply)
{

    if(reply->error() != QNetworkReply::NoError)
    {
        QString host = reply->request().url().host();
        QString port = QString::number(reply->request().url().port());
        QString domain = host + ":" + port;
        QString platformid;
        int row = -1;

        for(int i = 0; i < servers.size(); i++){
            if(servers.at(i).at(2) == domain){
                platformid = servers.at(i).at(3);
                row = i;
                break;
            }
        }

        if(platformid.isEmpty() || row == -1){
            log("[ERROR] Unknown platform id");
            return;
        }

        log("[ERROR] Status of " + platformid + ": " + reply->errorString());

        if(reply->url().path() == "/observer-mode/rest/consumer/version"){
            ui->tableWidget_status->setItem(row,4,new QTableWidgetItem("offline"));
            ui->tableWidget_status->item(row,4)->setBackgroundColor(Qt::red);
            ui->tableWidget_status->item(row,4)->setTextColor(Qt::white);
        }
        else{
            for(int i = 0; i < 4; i++){
                ui->tableWidget_status->setItem(row,i,new QTableWidgetItem("offline"));
                ui->tableWidget_status->item(row,i)->setBackgroundColor(Qt::red);
                ui->tableWidget_status->item(row,i)->setTextColor(Qt::white);
            }
        }
        return;
    }

    QString data = (QString) reply->readAll();

    QJsonDocument jsonResponse = QJsonDocument::fromJson(data.toUtf8());

    if(jsonResponse.isEmpty())
    {
        if(reply->url().path() == "/observer-mode/rest/consumer/version"){
            QString host = reply->request().url().host();
            QString port = QString::number(reply->request().url().port());
            QString domain = host + ":" + port;
            QString platformid;
            int row = -1;

            for(int i = 0; i < servers.size(); i++){
                if(servers.at(i).at(2) == domain){
                    platformid = servers.at(i).at(3);
                    row = i;
                    break;
                }
            }

            if(platformid.isEmpty() || row == -1){
                log("[ERROR] Unknown platform id");
                return;
            }

            if(data.isEmpty()){
                ui->tableWidget_status->setItem(row,4,new QTableWidgetItem("offline"));
                ui->tableWidget_status->item(row,4)->setBackgroundColor(Qt::red);
                ui->tableWidget_status->item(row,4)->setTextColor(Qt::white);
                return;
            }

            ui->tableWidget_status->setItem(row,4,new QTableWidgetItem("online"));
            ui->tableWidget_status->item(row,4)->setBackgroundColor(QColor(0,160,0));
            ui->tableWidget_status->item(row,4)->setTextColor(Qt::white);

            return;
        }
        else{
            log("[ERROR] Status");
            return;
        }
    }

    QJsonObject jsonObject = jsonResponse.object();

    json_status.append(jsonObject);

    QJsonArray services = jsonObject.value(tr("services")).toArray();

    log(jsonObject.value(tr("name")).toString() + tr(" : Status server infos"));

    for(int i = 0; i < servers.size(); i++)
    {
        if(jsonObject.value(tr("name")).toString() == servers.at(i).at(0)){
            for(int j = 0; j < 4; j++){
                ui->tableWidget_status->setItem(i,j,new QTableWidgetItem(services[j].toObject().value(tr("status")).toString()));
                if(services[j].toObject().value(tr("status")).toString() == "offline"){
                    ui->tableWidget_status->item(i,j)->setBackgroundColor(Qt::red);
                    ui->tableWidget_status->item(i,j)->setTextColor(Qt::white);
                }
                else if(services[j].toObject().value(tr("status")).toString() == "online"){
                    ui->tableWidget_status->item(i,j)->setBackgroundColor(QColor(0,160,0));
                    ui->tableWidget_status->item(i,j)->setTextColor(Qt::white);
                }
            }
            break;
        }
    }
}

void MainWindow::slot_networkResult_featured(QNetworkReply *reply)
{
    if(reply->error() != QNetworkReply::NoError){
        log("[ERROR] Network featured games : " + reply->errorString());
        return;
    }

    QString data = (QString) reply->readAll();

    QJsonDocument jsonResponse = QJsonDocument::fromJson(data.toUtf8());

    if(jsonResponse.isEmpty()){
        log("[ERROR] Featured games empty reponse");
        return;
    }

    QJsonObject jsonObject = jsonResponse.object();

    json_featured.append(jsonObject);

    QJsonArray gamelist = jsonObject.value(tr("gameList")).toArray();

    if(gamelist.size() == 0){
        log("[WARN] Featured games empty response");
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

void MainWindow::slot_featuredRefresh()
{
    while(ui->tableWidget_featured->rowCount() > 0){
        ui->tableWidget_featured->removeRow(0);
    }
    json_featured.clear();

    networkManager_featured->get(QNetworkRequest(QUrl(tr("http://spectator.euw1.lol.riotgames.com/observer-mode/rest/featured"))));  // GET EUW FEATURED GAMES

    networkManager_featured->get(QNetworkRequest(QUrl(tr("http://spectator.eu.lol.riotgames.com:8088/observer-mode/rest/featured"))));  // GET EUNE FEATURED GAMES

    networkManager_featured->get(QNetworkRequest(QUrl(tr("http://spectator.na.lol.riotgames.com/observer-mode/rest/featured"))));  // GET NA FEATURED GAMES

    networkManager_featured->get(QNetworkRequest(QUrl(tr("http://spectator.jp1.lol.riotgames.com/observer-mode/rest/featured"))));  // GET JP FEATURED GAMES

    networkManager_featured->get(QNetworkRequest(QUrl(tr("http://spectator.kr.lol.riotgames.com/observer-mode/rest/featured"))));  // GET KR FEATURED GAMES

    networkManager_featured->get(QNetworkRequest(QUrl(tr("http://spectator.oc1.lol.riotgames.com/observer-mode/rest/featured"))));  // GET OCE FEATURED GAMES

    networkManager_featured->get(QNetworkRequest(QUrl(tr("http://spectator.br.lol.riotgames.com/observer-mode/rest/featured"))));  // GET BR LAN LAS FEATURED GAMES

    networkManager_featured->get(QNetworkRequest(QUrl(tr("http://spectator.ru.lol.riotgames.com/observer-mode/rest/featured"))));  // GET RU TR FEATURED GAMES

    networkManager_featured->get(QNetworkRequest(QUrl(tr("http://spectator.pbe1.lol.riotgames.com:8088/observer-mode/rest/featured"))));  // GET PBE FEATURED GAMES

}

QPixmap MainWindow::getImg(int id)
{
    int finalid = 0;

    QFileInfo info(":/img/" + QString::number(id) + ".png");
    if(info.exists() && info.isFile()){
        finalid = id;
    }

    QPixmap img(":/img/" + QString::number(finalid) + ".png");
    return img.scaled(60, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void MainWindow::slot_click_featured()
{
    if(ui->tableWidget_featured->selectedItems().isEmpty()){
        return;
    }

    QString gameid = ui->tableWidget_featured->item(ui->tableWidget_featured->currentRow(),1)->text();

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
        log("[ERROR] Game selected ");
        return;
    }

    ui->label_featuredgames_gamemode->setText(game.value("gameMode").toString());

    ui->label_sumf1->clear();
    ui->label_sumf2->clear();
    ui->label_sumf3->clear();
    ui->label_sumf4->clear();
    ui->label_sumf5->clear();
    ui->label_sumf6->clear();
    ui->label_sumf7->clear();
    ui->label_sumf8->clear();
    ui->label_sumf9->clear();
    ui->label_sumf10->clear();

    ui->label_sumf16->clear();
    ui->label_sumf27->clear();
    ui->label_sumf38->clear();
    ui->label_sumf49->clear();
    ui->label_sumf510->clear();

    QJsonArray array = game.value("participants").toArray();
    QList<int> leftids;
    QList<QString> leftnames;
    QList<int> rightids;
    QList<QString> rightnames;

    for(int i = 0; i < array.size(); i++){
        if(array.at(i).toObject().value("teamId").toInt() == 200)
        {
            rightids.append(array.at(i).toObject().value("championId").toInt());
            rightnames.append(array.at(i).toObject().value("summonerName").toString());
        }
        else{
            leftids.append(array.at(i).toObject().value("championId").toInt());
            leftnames.append(array.at(i).toObject().value("summonerName").toString());
        }
    }

    if(leftids.size() >= 1)
    {
        ui->label_sumf1->setAlignment(Qt::AlignCenter);
        ui->label_sumf1->setPixmap(getImg(leftids.at(0)));

        ui->label_sumf16->setText(leftnames.at(0));

        if(leftids.size() >= 2)
        {
            ui->label_sumf2->setAlignment(Qt::AlignCenter);
            ui->label_sumf2->setPixmap(getImg(leftids.at(1)));

            ui->label_sumf27->setText(leftnames.at(1));

            if(leftids.size() >= 3)
            {
                ui->label_sumf3->setAlignment(Qt::AlignCenter);
                ui->label_sumf3->setPixmap(getImg(leftids.at(2)));

                ui->label_sumf38->setText(leftnames.at(2));

                if(leftids.size() >= 4)
                {
                    ui->label_sumf4->setAlignment(Qt::AlignCenter);
                    ui->label_sumf4->setPixmap(getImg(leftids.at(3)));

                    ui->label_sumf49->setText(leftnames.at(3));

                    if(leftids.size() >= 5){
                        ui->label_sumf5->setAlignment(Qt::AlignCenter);
                        ui->label_sumf5->setPixmap(getImg(leftids.at(4)));

                        ui->label_sumf510->setText(leftnames.at(4));
                    }
                }
            }
        }
    }

    if(rightids.size() >= 1)
    {
        ui->label_sumf6->setAlignment(Qt::AlignCenter);
        ui->label_sumf6->setPixmap(getImg(rightids.at(0)));

        ui->label_sumf16->setText(ui->label_sumf16->text() + " / " + rightnames.at(0));

        if(rightids.size() >= 2)
        {
            ui->label_sumf7->setAlignment(Qt::AlignCenter);
            ui->label_sumf7->setPixmap(getImg(rightids.at(1)));

            ui->label_sumf27->setText(ui->label_sumf27->text() + " / " + rightnames.at(1));

            if(rightids.size() >= 3)
            {
                ui->label_sumf8->setAlignment(Qt::AlignCenter);
                ui->label_sumf8->setPixmap(getImg(rightids.at(2)));

                ui->label_sumf38->setText(ui->label_sumf38->text() + " / " + rightnames.at(2));

                if(rightids.size() >= 4)
                {
                    ui->label_sumf9->setAlignment(Qt::AlignCenter);
                    ui->label_sumf9->setPixmap(getImg(rightids.at(3)));

                    ui->label_sumf49->setText(ui->label_sumf49->text() + " / " + rightnames.at(3));

                    if(rightids.size() >= 5){
                        ui->label_sumf10->setAlignment(Qt::AlignCenter);
                        ui->label_sumf10->setPixmap(getImg(rightids.at(4)));

                        ui->label_sumf510->setText(ui->label_sumf510->text() + " / " + rightnames.at(4));
                    }
                }
            }
        }
    }

    QPixmapCache::clear();
}

void MainWindow::slot_setdirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open RADS Directory"),loldirectory,QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(dir.isEmpty()){
        return;
    }
    loldirectory = dir;
    orsettings->setValue("LoLDirectory", loldirectory);
    ui->lineEdit_4->setText(dir);
}

void MainWindow::slot_setpbedirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open RADS Directory"),loldirectory,QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(dir.isEmpty()){
        return;
    }
    lolpbedirectory = dir;
    orsettings->setValue("LoLPBEDirectory", lolpbedirectory);
    ui->lineEdit_pbefolder->setText(dir);
}

void MainWindow::slot_setreplaydirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),replaydirectory,QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(dir.isEmpty()){
        return;
    }
    replaydirectory = dir;
    orsettings->setValue("ReplayDirectory", replaydirectory);
    ui->lineEdit_replaysFolder->setText(dir);
}

void MainWindow::slot_featuredLaunch()
{
    if(ui->tableWidget_featured->selectedItems().isEmpty()){
        return;
    }
    int row = ui->tableWidget_featured->currentRow();

    slot_refreshPlayingStatus();

    if(playing){
        log("[WARN] Replay aborted. LoL is currently running.");
        return;
    }

    lol_launch(ui->tableWidget_featured->item(row,0)->text(),ui->tableWidget_featured->item(row,2)->text(),ui->tableWidget_featured->item(row,1)->text());

    replaying = true;
}

bool MainWindow::check_path(QString path)
{
    QFileInfo checkFile(path);
    return(checkFile.exists());
}

void MainWindow::slot_changedTab(int index)
{
    if(index == 2){
        slot_featuredRefresh();
    }
}

bool MainWindow::game_ended(QString serverid, QString gameid)
{
    //Get serverID
    QString serveraddress;
    for(int i = 0; i < servers.size(); i++){
        if(servers.at(i).at(1) == serverid){
            serveraddress = servers.at(i).at(2);
        }
    }
    if(serveraddress.isEmpty()){
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

QJsonDocument MainWindow::getJsonFromUrl(QString url)
{
    QString finalurl = url;
    finalurl.replace(" ","");
    QNetworkAccessManager local_networkResult;
    QNetworkReply *reply = local_networkResult.get(QNetworkRequest(QUrl(finalurl)));

    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if(reply->error() != QNetworkReply::NoError)
    {
        QJsonDocument jsonEmpty;
        log("[ERROR] Get JSON : " + reply->errorString());
        return jsonEmpty;
    }

    QString data = (QString) reply->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(data.toUtf8());
    reply->deleteLater();

    return jsonResponse;
}

void MainWindow::slot_featuredRecord()
{
    if(ui->tableWidget_featured->selectedItems().isEmpty()){
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
    if(serveraddress.isEmpty()){
        return;
    }

    for(int i = 0; i < recording.size(); i++){
        if(recording.at(i).at(0) == serverid && recording.at(i).at(1) == gameid){
            //Game is already recording
            log("Game is already recording");
            return;
        }
    }

    recording.append(QStringList() << serverid << gameid);

    ui->lineEdit_status->setText("Recording " + QString::number(recording.size()) + " games");

    QJsonDocument gameinfo;

    for(int i = 0; i < json_featured.size(); i++){
        QJsonArray gamelist = json_featured.at(i).value("gameList").toArray();
        for(int k = 0; k < gamelist.size(); k++){
            if(gamelist.at(k).toObject().value("platformId").toString() == serverid){
                 if(QString::number(gamelist.at(k).toObject().value("gameId").toVariant().toULongLong()) == gameid){
                     gameinfo = QJsonDocument(gamelist.at(k).toObject());
                     break;
                 }
            }
        }
    }

    Recorder *recorder = new Recorder(this, serverid, serveraddress, gameid, ui->tableWidget_featured->item(row,2)->text(), gameinfo, replaydirectory);
    connect(recorder, SIGNAL(end(QString,QString)), this, SLOT(slot_endRecording(QString,QString)));
    connect(recorder, SIGNAL(finished()), recorder, SLOT(deleteLater()));
    recorder->start();
}

void MainWindow::slot_endRecording(QString serverid, QString gameid)
{
    for(int i = 0; i < recording.size(); i++)
    {
        if(recording.at(i).at(0) == serverid && recording.at(i).at(1) == gameid)
        {
            recording.removeAt(i);
            break;
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

QByteArray MainWindow::getFileFromUrl(QString url)
{
    QNetworkAccessManager local_networkResult;
    QNetworkReply *reply = local_networkResult.get(QNetworkRequest(QUrl(url)));

    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if(reply->error() != QNetworkReply::NoError)
    {
        QByteArray emptyArray;
        log("[ERROR] Get Data : " + reply->errorString());
        return emptyArray;
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();
    return data;
}

void MainWindow::slot_click_allgames()
{
    if(ui->tableWidget_recordedgames->selectedItems().isEmpty()){
        return;
    }

    Replay game(replaydirectory + "/" + recordedgames_filename.at(ui->tableWidget_recordedgames->currentRow()));

    ui->label_allgames_gamemode->setText(game.getGameinfos().object().value("gameMode").toString());

    if(game.getGameinfos().isEmpty()){
        log("[ERROR] No game infos found. Aborting.");
        return;
    }

    ui->label_sum1->clear();
    ui->label_sum2->clear();
    ui->label_sum3->clear();
    ui->label_sum4->clear();
    ui->label_sum5->clear();
    ui->label_sum6->clear();
    ui->label_sum7->clear();
    ui->label_sum8->clear();
    ui->label_sum9->clear();
    ui->label_sum10->clear();

    ui->label_sum16->clear();
    ui->label_sum27->clear();
    ui->label_sum38->clear();
    ui->label_sum49->clear();
    ui->label_sum510->clear();

    QJsonArray array = game.getGameinfos().object().value("participants").toArray();
    QList<int> leftids;
    QList<QString> leftnames;
    QList<int> rightids;
    QList<QString> rightnames;

    for(int i = 0; i < array.size(); i++){
        if(array.at(i).toObject().value("teamId").toInt() == 200)
        {
            rightids.append(array.at(i).toObject().value("championId").toInt());
            rightnames.append(array.at(i).toObject().value("summonerName").toString());
        }
        else{
            leftids.append(array.at(i).toObject().value("championId").toInt());
            leftnames.append(array.at(i).toObject().value("summonerName").toString());
        }
    }

    if(leftids.size() >= 1)
    {
        ui->label_sum1->setAlignment(Qt::AlignCenter);
        ui->label_sum1->setPixmap(getImg(leftids.at(0)));

        ui->label_sum16->setText(leftnames.at(0));

        if(leftids.size() >= 2)
        {
            ui->label_sum2->setAlignment(Qt::AlignCenter);
            ui->label_sum2->setPixmap(getImg(leftids.at(1)));

            ui->label_sum27->setText(leftnames.at(1));

            if(leftids.size() >= 3)
            {
                ui->label_sum3->setAlignment(Qt::AlignCenter);
                ui->label_sum3->setPixmap(getImg(leftids.at(2)));

                ui->label_sum38->setText(leftnames.at(2));

                if(leftids.size() >= 4)
                {
                    ui->label_sum4->setAlignment(Qt::AlignCenter);
                    ui->label_sum4->setPixmap(getImg(leftids.at(3)));

                    ui->label_sum49->setText(leftnames.at(3));

                    if(leftids.size() >= 5){
                        ui->label_sum5->setAlignment(Qt::AlignCenter);
                        ui->label_sum5->setPixmap(getImg(leftids.at(4)));

                        ui->label_sum510->setText(leftnames.at(4));
                    }
                }
            }
        }
    }

    if(rightids.size() >= 1)
    {
        ui->label_sum6->setAlignment(Qt::AlignCenter);
        ui->label_sum6->setPixmap(getImg(rightids.at(0)));

        ui->label_sum16->setText(ui->label_sum16->text() + " / " + rightnames.at(0));

        if(rightids.size() >= 2)
        {
            ui->label_sum7->setAlignment(Qt::AlignCenter);
            ui->label_sum7->setPixmap(getImg(rightids.at(1)));

            ui->label_sum27->setText(ui->label_sum27->text() + " / " + rightnames.at(1));

            if(rightids.size() >= 3)
            {
                ui->label_sum8->setAlignment(Qt::AlignCenter);
                ui->label_sum8->setPixmap(getImg(rightids.at(2)));

                ui->label_sum38->setText(ui->label_sum38->text() + " / " + rightnames.at(2));

                if(rightids.size() >= 4)
                {
                    ui->label_sum9->setAlignment(Qt::AlignCenter);
                    ui->label_sum9->setPixmap(getImg(rightids.at(3)));

                    ui->label_sum49->setText(ui->label_sum49->text() + " / " + rightnames.at(3));

                    if(rightids.size() >= 5){
                        ui->label_sum10->setAlignment(Qt::AlignCenter);
                        ui->label_sum10->setPixmap(getImg(rightids.at(4)));

                        ui->label_sum510->setText(ui->label_sum510->text() + " / " + rightnames.at(4));
                    }
                }
            }
        }
    }

    QPixmapCache::clear();
}

void MainWindow::refresh_recordedGames()
{
    //Find saved replays

    recordedgames_filename.clear();
    yourgames_filename.clear();

    QDir dirreplays(replaydirectory);
    dirreplays.setFilter(QDir::Files);
    dirreplays.setSorting(QDir::Time | QDir::Reversed);

    QFileInfoList replayslist = dirreplays.entryInfoList();

    while(ui->tableWidget_recordedgames->rowCount() > 0){
        ui->tableWidget_recordedgames->removeRow(0);
    }

    while(ui->tableWidget_yourgames->rowCount() > 0){
        ui->tableWidget_yourgames->removeRow(0);
    }

    for(int i = 0; i < replayslist.size(); i++){
        QFileInfo fileinfo = replayslist.at(i);
        ui->tableWidget_recordedgames->insertRow(ui->tableWidget_recordedgames->rowCount());

        Replay game(fileinfo.filePath());

        QDateTime datetime;
        datetime.setMSecsSinceEpoch(quint64(game.getGameinfos().object().value("gameStartTime").toVariant().toLongLong()));

        ui->tableWidget_recordedgames->setItem(ui->tableWidget_recordedgames->rowCount()-1, 0, new QTableWidgetItem(game.getServerid()));
        ui->tableWidget_recordedgames->setItem(ui->tableWidget_recordedgames->rowCount()-1, 1, new QTableWidgetItem(game.getGameid()));
        ui->tableWidget_recordedgames->setItem(ui->tableWidget_recordedgames->rowCount()-1, 2, new QTableWidgetItem(datetime.date().toString()));
        ui->tableWidget_recordedgames->setItem(ui->tableWidget_recordedgames->rowCount()-1, 3, new QTableWidgetItem(fileinfo.fileName()));

        recordedgames_filename.append(fileinfo.fileName());

        //TODO: Manage platformId
        if(!m_summonerid.isEmpty() && !game.getGameinfos().object().value("participants").toArray().empty())
        {
            for(int i = 0; i < game.getGameinfos().object().value("participants").toArray().size(); i++)
            {
                if(game.getGameinfos().object().value("participants").toArray().at(i).toObject().value("summonerName").toString() == m_summonername){
                    ui->tableWidget_yourgames->insertRow(ui->tableWidget_yourgames->rowCount());

                    QLabel* label = new QLabel;
                    label->setAlignment(Qt::AlignCenter);
                    label->setPixmap(getImg(game.getGameinfos().object().value("participants").toArray().at(i).toObject().value("championId").toInt()));

                    ui->tableWidget_yourgames->setCellWidget(ui->tableWidget_yourgames->rowCount()-1, 0, label);
                    ui->tableWidget_yourgames->setItem(ui->tableWidget_yourgames->rowCount()-1, 2, new QTableWidgetItem(datetime.toString()));
                    ui->tableWidget_yourgames->setItem(ui->tableWidget_yourgames->rowCount()-1, 3, new QTableWidgetItem(game.getServerid()));
                    ui->tableWidget_yourgames->setItem(ui->tableWidget_yourgames->rowCount()-1, 4, new QTableWidgetItem(fileinfo.fileName()));

                    yourgames_filename.append(fileinfo.fileName());

                    break;
                }
            }
        }
    }

    QPixmapCache::clear();
}

void MainWindow::slot_replayserversAdd()
{
    if(ui->lineEdit_replayserver_address->text().isEmpty()){
        return;
    }

    orservers.append(ui->lineEdit_replayserver_address->text());

    ui->tableWidget_replayservers->insertRow(ui->tableWidget_replayservers->rowCount());
    ui->tableWidget_replayservers->setItem(ui->tableWidget_replayservers->rowCount()-1,0,new QTableWidgetItem(ui->lineEdit_replayserver_address->text()));
}

void MainWindow::slot_summonerinfos_save()
{
    if(ui->lineEdit_summonername->text().isEmpty()){
        return;
    }
    if(orservers.isEmpty()){
        QMessageBox::information(this,"LegendsReplay","Please add a LegendsReplay server.");
        log("Please add a LegendsReplay server.");
        return;
    }

    m_summonername = ui->lineEdit_summonername->text();
    m_summonerserver = ui->comboBox_summonerserver->currentText();

    //Retrieving summoner ID

    log("Retrieving summoner ID");

    QJsonDocument suminfos = getJsonFromUrl("http://" + orservers.first() + "?region=" + m_summonerserver + "&summonername=" + m_summonername);

    if(suminfos.isEmpty()){
        QMessageBox::information(this,"LegendsReplay","Unknown summoner on this server.");
        log("Unknown summoner on this server.");
        return;
    }
    else{
        m_summonerid = QString::number(suminfos.object().value(suminfos.object().keys().first()).toObject().value("id").toVariant().toLongLong());
        ui->lineEdit_summonerid->setText(m_summonerid);
        log("Your summoner ID is " + m_summonerid);

        orsettings->setValue("SummonerName", m_summonername);
        orsettings->setValue("SummonerId", m_summonerid);
        orsettings->setValue("SummonerServer", m_summonerserver);
    }
}

bool MainWindow::islolRunning()
{
  QProcess tasklist;

  tasklist.start("tasklist", QStringList() << "/NH" << "/FO" << "CSV" << "/FI" << QString("IMAGENAME eq League of Legends.exe"));
  tasklist.waitForFinished();

  QString output = tasklist.readAllStandardOutput();
  return output.startsWith(QString("\"League of Legends.exe\""));
}

bool MainWindow::islolclientRunning()
{
  QProcess tasklist;

  tasklist.start("tasklist", QStringList() << "/NH" << "/FO" << "CSV" << "/FI" << QString("IMAGENAME eq LolClient.exe"));
  tasklist.waitForFinished();

  QString output = tasklist.readAllStandardOutput();
  return output.startsWith(QString("\"LolClient.exe\""));
}

void MainWindow::slot_refreshPlayingStatus()
{
    if(!islolRunning()){
        replaying = false;
        playing = false;
        if(httpserver->isListening()){
            httpserver->stopListening();
            if(replay != NULL){
                delete replay;
                replay = NULL;
            }
            log("Server: stoped due to inactivity");
        }
    }

    if(!replaying && islolclientRunning() && islolRunning()){
        if(!playing){
            //Start recording the game
            playing = true;

            log("Game detected : start recording");

            QJsonDocument gameinfos = getCurrentPlayingGameInfos(m_summonerserver, m_summonerid);

            QTimer timer;
            QEventLoop loop;
            connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
            unsigned int counter = 0;
            timer.start(150000);

            while(gameinfos.isEmpty() || gameinfos.object().value("gameStartTime").toVariant().toLongLong() == 0){
                loop.exec();
                if(counter > 5){
                    log("[WARN] Game not found : API/LegendsReplay servers may be offline or not working correctly");
                    log("End of recording");
                    return;
                }
                counter++;
                gameinfos = getCurrentPlayingGameInfos(m_summonerserver, m_summonerid);
                timer.start(45000);
            }

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
                log("[ERROR] Server not found");
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

QJsonDocument MainWindow::getCurrentPlayingGameInfos(QString server, QString summonerid)
{
    QString servertag;

    for(int i = 0; i < servers.size(); i++){
        if(servers.at(i).at(3) == server){
            servertag = servers.at(i).at(1);
            break;
        }
    }

    if(servertag.isEmpty() || orservers.isEmpty()){
        QJsonDocument docempty;
        log("[ERROR] Unknown server");
        return docempty;
    }

    QJsonDocument gameinfos = getJsonFromUrl("http://" + orservers.first() + "?platformid=" + servertag + "&summonerid=" + summonerid);

    return gameinfos;
}

void MainWindow::slot_open_replay(bool param)
{
    Q_UNUSED(param);

    QString path = QFileDialog::getOpenFileName(this, tr("Select a Replay"),replaydirectory);
    replay_launch(path);
}

void MainWindow::slot_doubleclick_savedgames(int row, int column)
{
    Q_UNUSED(column);

    replay_launch(replaydirectory + "/" + recordedgames_filename.at(row));
}

void MainWindow::slot_doubleclick_mygames(int row, int column)
{
    Q_UNUSED(column);

    replay_launch(replaydirectory + "/" + yourgames_filename.at(row));
}

void MainWindow::replay_launch(QString pathfile)
{
    serverChunkCount = 0;
    serverKeyframeCount = 1;

    replaying = true;

    //Launch spectator server

    if(replay != NULL){
        delete replay;
        replay = NULL;
    }

    replay = new Replay(pathfile);

    log("Opening : " + pathfile);

    log("Server: started");

    httpserver->stopListening();

    httpserver->listen(QHostAddress::Any, 8088, [this](QHttpRequest* req, QHttpResponse* res) {
        QString url = req->url().toString();

        if(url == "/observer-mode/rest/consumer/version"){
            res->setStatusCode(qhttp::ESTATUS_OK);      // http status 200
            res->addHeader("Content-Type", "text/plain");
            res->end(replay->getServerversion().toLocal8Bit());
            log("Server: send server version");
        }
        else if(url.contains("/observer-mode/rest/consumer/getGameMetaData/" + replay->getServerid() + "/" + replay->getGameid())){
            int firstchunkid = replay->getChunks().first().getId();

            while(replay->findKeyframeByChunkId(firstchunkid).getId() == 0)
            {
                firstchunkid++;
            }

            QString metadata = "{\"gameKey\":{";
            metadata.append("\"gameId\":" + replay->getGameid());
            metadata.append(",\"platformId\":\"" + replay->getServerid() + "\"}");
            metadata.append(",\"gameServerAddress\":\"\"");
            metadata.append(",\"port\":0");
            metadata.append(",\"encryptionKey\":\"\"");
            metadata.append(",\"chunkTimeInterval\":30000");
            metadata.append(",\"gameEnded\":false");
            metadata.append(",\"lastChunkId\":" + QString::number(firstchunkid));
            metadata.append(",\"lastKeyFrameId\":" + QString::number(replay->findKeyframeByChunkId(firstchunkid).getId()));
            metadata.append(",\"endStartupChunkId\":" + replay->getEndstartupchunkid());
            metadata.append(",\"delayTime\":150000");
            metadata.append(",\"keyFrameTimeInterval\":60000");
            metadata.append(",\"decodedEncryptionKey\":\"\"");
            metadata.append(",\"startGameChunkId\":" + replay->getStartgamechunkid());
            metadata.append(",\"gameLength\":0");
            metadata.append(",\"clientAddedLag\":30000");
            metadata.append(",\"clientBackFetchingEnabled\":false");
            metadata.append(",\"clientBackFetchingFreq\":1000");
            metadata.append(",\"featuredGame\":false");
            metadata.append(",\"endGameChunkId\":-1");
            metadata.append(",\"endGameKeyFrameId\":-1");
            metadata.append("}");

            res->setStatusCode(qhttp::ESTATUS_OK);
            res->addHeader("Content-Type", "application/json; charset=UTF-8");
            res->end(metadata.toUtf8());

            log("Server: send game metadata");
        }
        else if(url.contains("/observer-mode/rest/consumer/getLastChunkInfo/" + replay->getServerid() + "/" + replay->getGameid()))
        {
            int endstartupchunkid = replay->getEndstartupchunkid().toInt();
            int startgamechunkid = replay->getStartgamechunkid().toInt();
            int endgamechunkid = 0;
            int nextavailablechunk = 6000;

            if(serverChunkCount < replay->getChunks().first().getId()){
                serverChunkCount = replay->getChunks().first().getId();
                serverKeyframeCount = replay->getChunks().first().getKeyframeId();
            }
            else if(serverChunkCount >= replay->getChunks().last().getId()){
                serverChunkCount = replay->getChunks().last().getId();
                serverKeyframeCount = replay->getKeyFrames().last().getId();
                endgamechunkid = replay->getChunks().last().getId();
                nextavailablechunk = 90000;
            }
            else{
                serverKeyframeCount = replay->findKeyframeByChunkId(serverChunkCount).getId();
            }

            int currentChunkid = serverChunkCount;

            Keyframe currentKeyframe = replay->findKeyframeByChunkId(currentChunkid);

            while((currentKeyframe.getId() == 0 || replay->findKeyframeByChunkId(currentChunkid).getId() != replay->findKeyframeByChunkId(currentChunkid + 1).getId()) && currentChunkid < replay->getChunks().last().getId())
            {
                currentChunkid++;
                serverChunkCount++;
                currentKeyframe = replay->findKeyframeByChunkId(currentChunkid);
                serverKeyframeCount = currentKeyframe.getId();
            }

            if(currentChunkid <= replay->getChunks().first().getId() + 6){
                nextavailablechunk = 6000;
            }
            else if(currentChunkid < replay->getChunks().last().getId()){
                nextavailablechunk = 1000;
            }

            QString data = "{";
            data.append("\"chunkId\":" + QString::number(currentChunkid));
            data.append(",\"availableSince\":" + QString::number(32000));
            data.append(",\"nextAvailableChunk\":" + QString::number(nextavailablechunk));
            data.append(",\"keyFrameId\":" + QString::number(currentKeyframe.getId()));
            data.append(",\"nextChunkId\":" + QString::number(currentKeyframe.getNextchunkid()));
            data.append(",\"endStartupChunkId\":" + QString::number(endstartupchunkid));
            data.append(",\"startGameChunkId\":" + QString::number(startgamechunkid));
            data.append(",\"endGameChunkId\":" + QString::number(endgamechunkid));
            data.append(",\"duration\":" + QString::number(replay->getChunk(currentChunkid).getDuration()));
            data.append("}");

            res->setStatusCode(qhttp::ESTATUS_OK);
            res->addHeader("Content-Type", "application/json; charset=UTF-8");
            res->end(data.toUtf8());

            log("Server: send lastChunkInfo");
        }
        else if(url.contains("/observer-mode/rest/consumer/getGameDataChunk/" + replay->getServerid() + "/" + replay->getGameid()))
        {
            int chunkid = -1;

            //Get and send the chunk
            url.remove("/observer-mode/rest/consumer/getGameDataChunk/" + replay->getServerid() + "/" + replay->getGameid() + "/");
            chunkid = url.left(url.indexOf("/")).toInt();

            Chunk chunk = replay->getChunk(chunkid);
            Chunk primarychunk = replay->getPrimaryChunk(chunkid);

            if(chunk.getId() > 0){
                QByteArray chunk_ba = chunk.getData();

                res->setStatusCode(qhttp::ESTATUS_OK);
                res->addHeader("Content-Type", "application/octet-stream");
                res->addHeader("Content-Length", QString::number(chunk_ba.size()).toLocal8Bit());
                res->end(chunk_ba);

                if(serverChunkCount >= replay->getEndstartupchunkid().toInt() && chunkid > replay->getEndstartupchunkid().toInt()){
                    serverChunkCount += 1;
                }

                log("Server: send chunk " + QString::number(chunkid));
            }
            else if(primarychunk.getId() > 0){
                QByteArray chunk_ba = primarychunk.getData();

                res->setStatusCode(qhttp::ESTATUS_OK);
                res->addHeader("Content-Type", "application/octet-stream");
                res->addHeader("Content-Length", QString::number(chunk_ba.size()).toLocal8Bit());
                res->end(chunk_ba);

                log("Server: send primary chunk " + QString::number(chunkid));
            }
            else
            {
                res->setStatusCode(qhttp::ESTATUS_NOT_FOUND);
                res->end("");
                log("Server: unknown requested chunk " + QString::number(chunkid));
            }
        }
        else if(url.contains("/observer-mode/rest/consumer/getKeyFrame/" + replay->getServerid() + "/" + replay->getGameid()))
        {
            int keyframeid = -1;

            //Get and send the keyframe
            url.remove("/observer-mode/rest/consumer/getKeyFrame/" + replay->getServerid() + "/" + replay->getGameid() + "/");
            keyframeid = url.left(url.indexOf("/")).toInt();

            Keyframe keyframe = replay->getKeyFrame(keyframeid);

            if(keyframe.getId() > 0){
                QByteArray keyframe_ba = keyframe.getData();

                res->setStatusCode(qhttp::ESTATUS_OK);
                res->addHeader("Content-Type", "application/octet-stream");
                res->end(keyframe_ba);

                serverKeyframeCount += 1;

                log("Server: send keyframe " + QString::number(keyframeid));
            }
            else{
                log("Server: unknown requested keyframe " + QString::number(keyframeid));
            }
        }
        else if(url.contains("/observer-mode/rest/consumer/endOfGameStats/" + replay->getServerid() + "/" + replay->getGameid()))
        {
            //End of game stats requested
            res->setStatusCode(qhttp::ESTATUS_OK);
            res->addHeader("Content-Type", "application/octet-stream");
            res->end(replay->getEndOfGameStats());

            log("Server: End of game stats sent");
        }
        else if(url.contains("/observer-mode/rest/consumer/end/" + replay->getServerid() + "/" + replay->getGameid()))
        {
            //End of replay requested : error while replaying
            res->setStatusCode(qhttp::ESTATUS_OK);
            res->addHeader("Content-Type", "text/plain");
            res->end("");

            log("Server: End of replay requested");

            if(replay != NULL){
                delete replay;
                replay = NULL;
            }

            httpserver->stopListening();
        }
        else
        {
            res->setStatusCode(qhttp::ESTATUS_OK);
            res->addHeader("Content-Type", "text/plain");
            res->end("");

            log("Server: Unknown requested link " + url);
        }
    });

    //Launch lol client
    lol_launch(replay->getServerid(),replay->getEncryptionkey(),replay->getGameid(),true);
}

void MainWindow::showmessage(QString message)
{
    if(systemtrayavailable)
    {
        systemtrayicon->showMessage("LegendsReplay", message, QSystemTrayIcon::Information);
    }
}

void MainWindow::slot_searchsummoner()
{
    ui->label_searchsummoner_status->setText("...");

    QString serverid = ui->comboBox_searchsummoner_platformid->currentText();
    QString summonername = ui->lineEdit_searchsummoner->text();

    QJsonDocument suminfos = getJsonFromUrl("http://" + orservers.first() + "?region=" + serverid + "&summonername=" + summonername);

    if(suminfos.isEmpty())
    {
        log("Summoner name not found on this server");
        return;
    }

    QString summonerid = QString::number(suminfos.object().value(suminfos.object().keys().first()).toObject().value("id").toVariant().toLongLong());

    if(summonerid.isEmpty())
    {
        log("[EROOR] Summoner id not valid");
        return;
    }

    QJsonDocument game = getCurrentPlayingGameInfos(serverid,summonerid);

    ui->label_sums1->clear();
    ui->label_sums2->clear();
    ui->label_sums3->clear();
    ui->label_sums4->clear();
    ui->label_sums5->clear();
    ui->label_sums6->clear();
    ui->label_sums7->clear();
    ui->label_sums8->clear();
    ui->label_sums9->clear();
    ui->label_sums10->clear();

    ui->label_sums16->clear();
    ui->label_sums27->clear();
    ui->label_sums38->clear();
    ui->label_sums49->clear();
    ui->label_sums510->clear();

    m_searchsummoner_game = game;

    if(game.isEmpty())
    {
        log("Summoner " + summonername + " is not in a game");

        ui->label_searchsummoner_status->setText("not playing");

        ui->label_searchsummoner_gamemode->setText("Game Mode");
    }
    else
    {
        log("Summoner " + summonername + " is in a game");

        ui->label_searchsummoner_status->setText("playing");

        ui->label_searchsummoner_gamemode->setText(game.object().value("gameMode").toString());

        QJsonArray array = game.object().value("participants").toArray();

        QList<int> leftids;
        QList<QString> leftnames;
        QList<int> rightids;
        QList<QString> rightnames;

        for(int i = 0; i < array.size(); i++){
            if(array.at(i).toObject().value("teamId").toInt() == 200)
            {
                rightids.append(array.at(i).toObject().value("championId").toInt());
                rightnames.append(array.at(i).toObject().value("summonerName").toString());
            }
            else{
                leftids.append(array.at(i).toObject().value("championId").toInt());
                leftnames.append(array.at(i).toObject().value("summonerName").toString());
            }
        }

        if(leftids.size() >= 1)
        {
            ui->label_sums1->setAlignment(Qt::AlignCenter);
            ui->label_sums1->setPixmap(getImg(leftids.at(0)));

            ui->label_sums16->setText(leftnames.at(0));

            if(leftids.size() >= 2)
            {
                ui->label_sums2->setAlignment(Qt::AlignCenter);
                ui->label_sums2->setPixmap(getImg(leftids.at(1)));

                ui->label_sums27->setText(leftnames.at(1));

                if(leftids.size() >= 3)
                {
                    ui->label_sums3->setAlignment(Qt::AlignCenter);
                    ui->label_sums3->setPixmap(getImg(leftids.at(2)));

                    ui->label_sums38->setText(leftnames.at(2));

                    if(leftids.size() >= 4)
                    {
                        ui->label_sums4->setAlignment(Qt::AlignCenter);
                        ui->label_sums4->setPixmap(getImg(leftids.at(3)));

                        ui->label_sums49->setText(leftnames.at(3));

                        if(leftids.size() >= 5){
                            ui->label_sums5->setAlignment(Qt::AlignCenter);
                            ui->label_sums5->setPixmap(getImg(leftids.at(4)));

                            ui->label_sums510->setText(leftnames.at(4));
                        }
                    }
                }
            }
        }

        if(rightids.size() >= 1)
        {
            ui->label_sums6->setAlignment(Qt::AlignCenter);
            ui->label_sums6->setPixmap(getImg(rightids.at(0)));

            ui->label_sums16->setText(ui->label_sums16->text() + " / " + rightnames.at(0));

            if(rightids.size() >= 2)
            {
                ui->label_sums7->setAlignment(Qt::AlignCenter);
                ui->label_sums7->setPixmap(getImg(rightids.at(1)));

                ui->label_sums27->setText(ui->label_sums27->text() + " / " + rightnames.at(1));

                if(rightids.size() >= 3)
                {
                    ui->label_sums8->setAlignment(Qt::AlignCenter);
                    ui->label_sums8->setPixmap(getImg(rightids.at(2)));

                    ui->label_sums38->setText(ui->label_sums38->text() + " / " + rightnames.at(2));

                    if(rightids.size() >= 4)
                    {
                        ui->label_sums9->setAlignment(Qt::AlignCenter);
                        ui->label_sums9->setPixmap(getImg(rightids.at(3)));

                        ui->label_sums49->setText(ui->label_sums49->text() + " / " + rightnames.at(3));

                        if(rightids.size() >= 5){
                            ui->label_sums10->setAlignment(Qt::AlignCenter);
                            ui->label_sums10->setPixmap(getImg(rightids.at(4)));

                            ui->label_sums510->setText(ui->label_sums510->text() + " / " + rightnames.at(4));
                        }
                    }
                }
            }
        }

        QPixmapCache::clear();
    }
}

void MainWindow::slot_click_searchsummoner_spectate()
{
    if(m_searchsummoner_game.isEmpty()){
        return;
    }

    lol_launch(m_searchsummoner_game.object().value("platformId").toString(), m_searchsummoner_game.object().value("observers").toObject().value("encryptionKey").toString(), QString::number(m_searchsummoner_game.object().value("gameId").toVariant().toULongLong()));
}

void MainWindow::slot_click_searchsummoner_record()
{
    if(m_searchsummoner_game.isEmpty()){
        return;
    }

    QString serveraddress;
    QString serverid = m_searchsummoner_game.object().value("platformId").toString();
    QString gameid = QString::number(m_searchsummoner_game.object().value("gameId").toVariant().toULongLong());

    for(int i = 0; i < servers.size(); i++){
        if(servers.at(i).at(1) == serverid){
            serveraddress = servers.at(i).at(2);
            break;
        }
    }
    if(serveraddress.isEmpty()){
        return;
    }

    for(int i = 0; i < recording.size(); i++){
        if(recording.at(i).at(0) == serverid && recording.at(i).at(1) == gameid){
            //Game is already recording
            log("Game is already recording");
            return;
        }
    }

    recording.append(QStringList() << serverid << gameid);

    ui->lineEdit_status->setText("Recording " + QString::number(recording.size()) + " games");

    Recorder *recorder = new Recorder(this, serverid, serveraddress, gameid, m_searchsummoner_game.object().value("observer").toObject().value("encryptionKey").toString(), m_searchsummoner_game, replaydirectory);
    connect(recorder, SIGNAL(end(QString,QString)), this, SLOT(slot_endRecording(QString,QString)));
    connect(recorder, SIGNAL(finished()), recorder, SLOT(deleteLater()));
    recorder->start();
}
