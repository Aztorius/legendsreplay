#include "mainwindow.h"
#include "ui_mainwindow.h"

QString GLOBAL_VERSION = "1.4.3";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle(tr("LegendsReplay ") + GLOBAL_VERSION);
    setWindowIcon(QIcon(":/icons/logo.png"));

    log(QString(tr("LegendsReplay ") + GLOBAL_VERSION + tr(" Started")));

    qsrand(1);

    // Adding the official local servers

    QFile localserversfile(":/data/LegendsReplayServers.txt");

    if(localserversfile.open(QIODevice::ReadOnly)){
        QTextStream in(&localserversfile);

        while(!in.atEnd()){
            QString line = in.readLine();
            if(!line.isEmpty()){
                lrservers.append(line);
                ui->tableWidget_replayservers->insertRow(ui->tableWidget_replayservers->rowCount());
                ui->tableWidget_replayservers->setItem(ui->tableWidget_replayservers->rowCount()-1, 0, new QTableWidgetItem(line));
            }
        }

        localserversfile.close();
    }
    else{
        log(tr("[ERROR] Unable to open internal servers file"));
        lrservers.append("informaticien77.serveminecraft.net/legendsreplay.php");
        ui->tableWidget_replayservers->insertRow(ui->tableWidget_replayservers->rowCount());
        ui->tableWidget_replayservers->setItem(ui->tableWidget_replayservers->rowCount()-1, 0, new QTableWidgetItem("informaticien77.serveminecraft.net/legendsreplay.php"));
    }

    // Adding the unofficial servers

    QFile serversfile(QStandardPaths::standardLocations(QStandardPaths::DataLocation).first() + "/LegendsReplayServers.txt");

    if(!serversfile.open(QIODevice::ReadOnly)){
        log(tr("[WARN] Opening servers file : ") + serversfile.errorString());

        log(tr("Creating LegendsReplayServers.txt file"));
        QDir filedir(QStandardPaths::standardLocations(QStandardPaths::DataLocation).first());

        if(!filedir.exists()){
            filedir.mkpath(".");
        }

        if(!serversfile.open(QIODevice::WriteOnly | QIODevice::Text)){
            log(tr("[ERROR] Creating servers file : ") + serversfile.errorString());
        }

        serversfile.close();
    }
    else{
        QTextStream in(&serversfile);

        while(!in.atEnd()){
            QString line = in.readLine();
            if(!line.isEmpty() && !lrservers.contains(line)){
                lrservers.append(line);
                ui->tableWidget_replayservers->insertRow(ui->tableWidget_replayservers->rowCount());
                ui->tableWidget_replayservers->setItem(ui->tableWidget_replayservers->rowCount()-1, 0, new QTableWidgetItem(line));
            }
        }

        serversfile.close();
    }

    if(lrservers.isEmpty()){
        // Critical
        QMessageBox::critical(this, tr("LegendsReplay"), tr("Unable to find a Legends Replay server"));
        this->close();
        return;
    }

    m_currentLegendsReplayServer = lrservers.at(qrand()%lrservers.size());

    orsettings = new QSettings("LegendsReplay", "Local");

    if(!orsettings->value("SummonerName").toString().isEmpty()){
        m_summonername = orsettings->value("SummonerName").toString();
        ui->lineEdit_summonername->setText(m_summonername);
    }
    else{
        QMessageBox::information(this, tr("LegendsReplay"), tr("Please set your summoner name and then keep the software open to record your games."));
        log(tr("[WARN] Please set your summoner name and then keep the software open to record your games."));

        ui->tabWidget->setCurrentIndex(6);
    }

    if(!orsettings->value("SummonerId").toString().isEmpty()){
        m_summonerid = orsettings->value("SummonerId").toString();
        ui->lineEdit_summonerid->setText(m_summonerid);
    }

    if(!orsettings->value("SummonerServer").toString().isEmpty()){
        m_summonerserver = orsettings->value("SummonerServer").toString();
        ui->comboBox_summonerserver->setCurrentText(m_summonerserver);
    }

    if(!orsettings->value("PBEName").toString().isEmpty()){
        m_PBEname = orsettings->value("PBEName").toString();
        ui->lineEdit_pbename->setText(m_PBEname);
    }

    if(!orsettings->value("PBEId").toString().isEmpty()){
        m_PBEid = orsettings->value("PBEId").toString();
        ui->lineEdit_pbeid->setText(m_PBEid);
    }

    if(!orsettings->value("Language").toString().isEmpty()){
        if(translator.load(QString(":/translation/legendsreplay_") + orsettings->value("Language").toString().toLower())){
            qApp->installTranslator(&translator);
        }
    }

    if(!orsettings->value("LoLDirectory").toString().isEmpty()){
        loldirectory = orsettings->value("LoLDirectory").toString();
    }
    else{
        QSettings lolsettings("Riot Games", "RADS");
        QString rootfolder = lolsettings.value("LocalRootFolder").toString();

        if(rootfolder.isEmpty()){
            loldirectory = "C:/Program Files/Riot Games/League of Legends/RADS";
        }
        else{
            loldirectory = rootfolder;
        }
    }

    ui->lineEdit_lolfolder->setText(loldirectory);

    if(!orsettings->value("LoLPBEDirectory").toString().isEmpty()){
        lolpbedirectory = orsettings->value("LoLPBEDirectory").toString();
        ui->lineEdit_pbefolder->setText(lolpbedirectory);
    }

    QString docfolder;
    if(orsettings->value("ReplayDirectory").toString().isEmpty()){
        QStringList folders = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
        if(folders.isEmpty()){
            log(tr("[CRITIC] no documents location found on the system"));
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

    replaying = false;
    playing = false;

    replay = NULL;

    serverChunkCount = 0;
    serverKeyframeCount = 1;

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slot_refreshPlayingStatus()));
    m_timer->start(60000);

    //Add servers

    servers.append(Server("EU West", "EUW", "EUW1", "spectator.euw1.lol.riotgames.com", 80));
    servers.append(Server("EU Nordic & East", "EUNE", "EUN1", "spectator.eu.lol.riotgames.com", 8088));
    servers.append(Server("North America", "NA", "NA1", "spectator.na.lol.riotgames.com", 80));
    servers.append(Server("Japan", "JP", "JP1", "spectator.jp1.lol.riotgames.com", 80));
    servers.append(Server("Republic of Korea", "KR", "KR", "spectator.kr.lol.riotgames.com", 80));
    servers.append(Server("Oceania", "OCE", "OC1", "spectator.oc1.lol.riotgames.com", 80));
    servers.append(Server("Brazil", "BR", "BR1", "spectator.br.lol.riotgames.com", 80));
    servers.append(Server("Latin America North", "LAN", "LA1", "spectator.la1.lol.riotgames.com", 80));
    servers.append(Server("Latin America South", "LAS", "LA2", "spectator.la2.lol.riotgames.com", 80));
    servers.append(Server("Russia", "RU", "RU", "spectator.ru.lol.riotgames.com", 80));
    servers.append(Server("Turkey", "TR", "TR1", "spectator.tr.lol.riotgames.com", 80));
    servers.append(Server("PBE", "PBE", "PBE1", "spectator.pbe1.lol.riotgames.com", 8088));

    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(slot_changedTab(int)));
    connect(ui->pushButton_2, SIGNAL(released()), this, SLOT(slot_featuredRefresh()));
    connect(ui->listWidget_featured, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(slot_doubleclick_featured(QListWidgetItem*)));
    connect(ui->toolButton, SIGNAL(released()), this, SLOT(slot_setdirectory()));
    connect(ui->toolButton_2, SIGNAL(released()), this, SLOT(slot_setreplaydirectory()));
    connect(ui->toolButton_pbefolder, SIGNAL(released()), this, SLOT(slot_setpbedirectory()));
    connect(ui->pushButton_add_replayserver, SIGNAL(released()), this, SLOT(slot_replayserversAdd()));
    connect(ui->pushButton_pbesave, SIGNAL(released()), this, SLOT(slot_pbeinfos_save()));
    connect(ui->pushButton_summonersave, SIGNAL(released()), this, SLOT(slot_summonerinfos_save()));
    connect(ui->pushButton_searchsummoner, SIGNAL(released()), this, SLOT(slot_searchsummoner()));
    connect(ui->pushButton_searchsummoner_spectate, SIGNAL(released()), this, SLOT(slot_click_searchsummoner_spectate()));
    connect(ui->pushButton_searchsummoner_record, SIGNAL(released()), this, SLOT(slot_click_searchsummoner_record()));
    connect(ui->tableWidget_replayservers, SIGNAL(itemSelectionChanged()), this, SLOT(slot_click_replayservers()));
    connect(ui->actionRepair_tool, SIGNAL(triggered()), this, SLOT(slot_checkandrepair()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionReport_an_issue, SIGNAL(triggered()), this, SLOT(slot_reportAnIssue()));
    connect(ui->actionAbout_LegendsReplay, SIGNAL(triggered()), this, SLOT(slot_aboutLegendsReplay()));

    networkManager_status = new QNetworkAccessManager(this);
    connect(networkManager_status, SIGNAL(finished(QNetworkReply*)), this, SLOT(slot_networkResult_status(QNetworkReply*)));
    connect(ui->pushButton_statusRefresh, SIGNAL(pressed()), this, SLOT(slot_statusRefresh()));
    connect(this, SIGNAL(signal_refreshStatusServers()), this, SLOT(slot_statusRefresh()), Qt::QueuedConnection);

    emit signal_refreshStatusServers();

    connect(this, SIGNAL(signal_refresh_recordedGames()), this, SLOT(slot_refresh_recordedGames()), Qt::QueuedConnection);

    networkManager_featured = new QNetworkAccessManager(this);
    connect(networkManager_featured, SIGNAL(finished(QNetworkReply*)), this, SLOT(slot_networkResult_featured(QNetworkReply*)));
    connect(ui->tableWidget_recordedgames, SIGNAL(itemSelectionChanged()), this, SLOT(slot_click_allgames()));
    connect(ui->tableWidget_recordedgames, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slot_customcontextmenu(QPoint)));
    connect(ui->tableWidget_yourgames, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slot_customcontextmenu(QPoint)));
    connect(ui->listWidget_featured, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slot_customcontextmenu(QPoint)));
    connect(ui->tableWidget_recordingGames, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slot_customcontextmenu(QPoint)));

    httpserver = new QHttpServer(this);
    connect(ui->tableWidget_recordedgames, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(slot_doubleclick_savedgames(int,int)));
    connect(ui->tableWidget_yourgames, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(slot_doubleclick_mygames(int,int)));

    connect(ui->actionOpen, SIGNAL(triggered(bool)), this, SLOT(slot_open_replay(bool)));
    connect(ui->actionAdvanced_Recorder, SIGNAL(triggered()), this, SLOT(slot_openAdvancedRecorder()));
    connect(ui->pushButton_saveLanguage, SIGNAL(released()), this, SLOT(slot_setLanguage()));

    m_directory_watcher = new QFileSystemWatcher;
    m_directory_watcher->addPath(replaydirectory);
    connect(m_directory_watcher, SIGNAL(directoryChanged(QString)), this, SLOT(slot_directoryChanged(QString)));

    emit signal_refresh_recordedGames();

    systemtrayavailable = QSystemTrayIcon::isSystemTrayAvailable();

    if(systemtrayavailable){
        systemtrayicon = new QSystemTrayIcon;
        systemtrayicon->setIcon(QIcon(":/icons/logo.png"));

        QMenu* menu = new QMenu;
        menu->addAction(QIcon(":/icons/exit.png"), tr("Exit"));
        systemtrayicon->setContextMenu(menu);
        systemtrayicon->show();

        connect(systemtrayicon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(systemtrayiconActivated(QSystemTrayIcon::ActivationReason)));
        connect(systemtrayicon, SIGNAL(messageClicked()), this, SLOT(slot_messageclicked()));
        connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(slot_customMenuTriggeredSystemTrayIcon(QAction*)));
    }

    connect(this, SIGNAL(signal_checkSoftwareVersion()), this, SLOT(slot_checkSoftwareVersion()), Qt::QueuedConnection);

    emit signal_checkSoftwareVersion();
}

MainWindow::~MainWindow()
{
    for(int i = 0; i < recordingThreads.size(); i++){
        recordingThreads.at(i)->exit(0);
    }

    if(systemtrayavailable)
    {
        systemtrayicon->hide();
        systemtrayicon->deleteLater();
    }

    m_directory_watcher->deleteLater();
    httpserver->deleteLater();
    networkManager_featured->deleteLater();
    networkManager_status->deleteLater();
    m_timer->deleteLater();
    orsettings->deleteLater();    

    delete replay;
    delete ui;
}

void MainWindow::slot_checkSoftwareVersion(){
    QJsonDocument updatejson = getJsonFromUrl("http://aztorius.github.io/legendsreplay/version.json");

    if(!updatejson.isEmpty() && updatejson.object().value("version").toString() != GLOBAL_VERSION)
    {
        showmessage(tr("New version ") + updatejson.object().value("version").toString() + tr(" available !"));
        ui->statusBar->showMessage(QTime::currentTime().toString() + " | " + tr("New version ") + updatejson.object().value("version").toString() + tr(" available !"));
        ui->textBrowser->append(QTime::currentTime().toString() + " | <a href='http://aztorius.github.io/legendsreplay/'>" + tr("New version ") + updatejson.object().value("version").toString() + tr(" available !") + "</a>");
    }
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

        if(std::string(argv[1]) == "record" && argc > 6){ //command : record serverid serveraddress gameid encryptionkey forceCompleteDownload
            slot_customGameRecord(QString(argv[3]), QString(argv[2]), QString(argv[4]), QString(argv[5]), QString(argv[6]) == "true", false, false);
        }

        Replay replay(argv[1]);
        if(!replay.getGameId().isEmpty()){
            replay_launch(argv[1]);
            return;
        }
    }
}

void MainWindow::slot_statusRefresh()
{
    ui->tableWidget_status->clearContents();

    networkManager_status->get(QNetworkRequest(QUrl("http://status.leagueoflegends.com/shards/euw")));  // GET EUW SERVERS STATUS
    networkManager_status->get(QNetworkRequest(QUrl("http://status.leagueoflegends.com/shards/eune")));  // GET EUNE SERVERS STATUS
    networkManager_status->get(QNetworkRequest(QUrl("http://status.leagueoflegends.com/shards/na")));  // GET NA SERVERS STATUS
    networkManager_status->get(QNetworkRequest(QUrl("http://status.leagueoflegends.com/shards/jp")));  // GET JP SERVERS STATUS
    networkManager_status->get(QNetworkRequest(QUrl("http://status.leagueoflegends.com/shards/kr")));  // GET KR SERVERS STATUS
    networkManager_status->get(QNetworkRequest(QUrl("http://status.leagueoflegends.com/shards/oce")));  // GET OCE SERVERS STATUS
    networkManager_status->get(QNetworkRequest(QUrl("http://status.leagueoflegends.com/shards/br")));  // GET BR SERVERS STATUS
    networkManager_status->get(QNetworkRequest(QUrl("http://status.leagueoflegends.com/shards/lan")));  // GET LAN SERVERS STATUS
    networkManager_status->get(QNetworkRequest(QUrl("http://status.leagueoflegends.com/shards/las")));  // GET LAS SERVERS STATUS
    networkManager_status->get(QNetworkRequest(QUrl("http://status.leagueoflegends.com/shards/ru")));  // GET RU SERVERS STATUS
    networkManager_status->get(QNetworkRequest(QUrl("http://status.leagueoflegends.com/shards/tr")));  // GET TR SERVERS STATUS
    networkManager_status->get(QNetworkRequest(QUrl("http://status.pbe.leagueoflegends.com/shards/pbe")));  // GET PBE SERVERS STATUS

    for(int i = 0; i < servers.size(); i++){
        networkManager_status->get(QNetworkRequest(QUrl("http://" + servers.at(i).getUrl() + "/observer-mode/rest/consumer/version")));
    }
}

void MainWindow::slot_doubleclick_featured(QListWidgetItem *item)
{
    GameInfosWidget *widget = dynamic_cast<GameInfosWidget*>(ui->listWidget_featured->itemWidget(item));

    QString serverid = widget->getServerId();
    QString key = widget->getEncryptionkey();
    QString gameid = widget->getGameId();

    if(game_ended(serverid, gameid)){
        log(tr("Game ") + serverid + "/" + gameid + tr(" has already ended"));
        return;
    }

    slot_refreshPlayingStatus();

    if(playing){
        log(tr("[WARN] Replay aborted. LoL is currently running."));
        return;
    }

    replaying = true;

    lol_launch(serverid, key, gameid);
}

void MainWindow::lol_launch(QString serverid, QString key, QString matchid, bool local)
{
    if(serverid.isEmpty() || key.isEmpty() || matchid.isEmpty()){
        log(tr("[ERROR] Invalid game parameters."));
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
        QMessageBox::information(this, tr("LegendsReplay"), tr("Invalid League of Legends (or PBE) directory.\nPlease set a valid one."));
        log(tr("[WARN] Invalid League of Legends (or PBE) directory. No releases folder found."));
        return;
    }

    if(serverid == "PBE1"){
        path = lolpbedirectory + "/solutions/lol_game_client_sln/releases/" + list.at(0).fileName() + "/deploy/";
    }
    else{
        path = loldirectory + "/solutions/lol_game_client_sln/releases/" + list.at(0).fileName() + "/deploy/";
    }

    if(!check_path(path)){
        QMessageBox::information(this, tr("LegendsReplay"), tr("Invalid League of Legends (or PBE) directory.\nPlease set a valid one."));
        log(tr("[WARN] Invalid League of Legends (or PBE) directory. Invalid path."));
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
        log(tr("LoL Launch isn't available for Linux yet"));

#endif

    }
    else{
        for(int i = 0; i < servers.size(); i++){
            if(servers.at(i).getPlatformId() == serverid){
                address = servers.at(i).getUrl();
                break;
            }
        }

        if(address.isEmpty()){
            //Server address not found
            log(tr("[ERROR] Server address not found"));
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
        QString platformid;
        int row = -1;

        for(int i = 0; i < servers.size(); i++){
            if(servers.at(i).getDomain() == host){
                platformid = servers.at(i).getPlatformId();
                row = i;
                break;
            }
        }

        if(platformid.isEmpty() || row == -1){
            log(tr("[ERROR] Unknown platform id"));
            return;
        }

        log(tr("[ERROR] Status of ") + platformid + ": " + reply->errorString());

        if(reply->url().path() == "/observer-mode/rest/consumer/version"){
            ui->tableWidget_status->setItem(row, ui->tableWidget_status->columnCount()-1, new QTableWidgetItem(tr("offline")));
            ui->tableWidget_status->item(row, ui->tableWidget_status->columnCount()-1)->setBackgroundColor(Qt::red);
            ui->tableWidget_status->item(row, ui->tableWidget_status->columnCount()-1)->setTextColor(Qt::white);
        }
        else{
            for(int i = 0; i < 4; i++){
                ui->tableWidget_status->setItem(row, i, new QTableWidgetItem(tr("offline")));
                ui->tableWidget_status->item(row, i)->setBackgroundColor(Qt::red);
                ui->tableWidget_status->item(row, i)->setTextColor(Qt::white);
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
            QString platformid;
            int row = -1;

            for(int i = 0; i < servers.size(); i++){
                if(servers.at(i).getDomain() == host){
                    platformid = servers.at(i).getPlatformId();
                    row = i;
                    break;
                }
            }

            if(platformid.isEmpty() || row == -1){
                log(tr("[ERROR] Unknown platform id"));
                return;
            }

            if(data.isEmpty()){
                ui->tableWidget_status->setItem(row, ui->tableWidget_status->columnCount()-1, new QTableWidgetItem(tr("offline")));
                ui->tableWidget_status->item(row, ui->tableWidget_status->columnCount()-1)->setBackgroundColor(Qt::red);
                ui->tableWidget_status->item(row, ui->tableWidget_status->columnCount()-1)->setTextColor(Qt::white);
                return;
            }

            ui->tableWidget_status->setItem(row, ui->tableWidget_status->columnCount()-1, new QTableWidgetItem(tr("online")));
            ui->tableWidget_status->item(row, ui->tableWidget_status->columnCount()-1)->setBackgroundColor(QColor(0,160,0));
            ui->tableWidget_status->item(row, ui->tableWidget_status->columnCount()-1)->setTextColor(Qt::white);

            return;
        }
        else{
            log(tr("[ERROR] Retrieving server status"));
            return;
        }
    }

    QJsonObject jsonObject = jsonResponse.object();

    json_status.append(jsonObject);

    QJsonArray services = jsonObject.value("services").toArray();

    log(jsonObject.value("name").toString() + tr(" : Status server infos"));

    for(int i = 0; i < servers.size(); i++)
    {
        if(jsonObject.value("name").toString() == servers.at(i).getName()){
            for(int j = 0; j < qMin(services.size(), 5); j++){
                QJsonArray incidentsArray = services[j].toObject().value("incidents").toArray();

                if(services[j].toObject().value("status").toString() == "offline"){ //Service offline
                    ui->tableWidget_status->setItem(i, j, new QTableWidgetItem(tr("offline")));
                    ui->tableWidget_status->item(i, j)->setBackgroundColor(Qt::red);
                    ui->tableWidget_status->item(i, j)->setTextColor(Qt::white);
                }
                else if(!incidentsArray.isEmpty()){ //Service online with incidents
                    QString incidents = incidentsArray.first().toObject().value("updates").toArray().first().toObject().value("content").toString();

                    for(int k = 1; k < incidentsArray.size(); k++){
                        if(!incidentsArray.at(k).toObject().value("updates").toArray().first().toObject().value("content").toString().isEmpty()){
                            if(!incidents.isEmpty()){
                                incidents.append("\n");
                            }

                            incidents.append(incidentsArray.at(k).toObject().value("updates").toArray().first().toObject().value("content").toString());
                        }
                    }

                    ui->tableWidget_status->setItem(i, j, new QTableWidgetItem(QString::number(incidentsArray.size()) + tr(" incident(s)")));
                    ui->tableWidget_status->item(i, j)->setToolTip(incidents);
                    ui->tableWidget_status->item(i, j)->setTextColor(Qt::white);
                    ui->tableWidget_status->item(i, j)->setBackgroundColor(QColor(255,165,0));
                }
                else if(services[j].toObject().value("status").toString() == "online"){ //Service online
                    ui->tableWidget_status->setItem(i, j, new QTableWidgetItem(tr("online")));
                    ui->tableWidget_status->item(i, j)->setTextColor(Qt::white);
                    ui->tableWidget_status->item(i, j)->setBackgroundColor(QColor(0,160,0));
                }
            }
            break;
        }
    }
}

void MainWindow::slot_networkResult_featured(QNetworkReply *reply)
{
    if(reply->error() != QNetworkReply::NoError){
        log(tr("[ERROR] Network featured games : ") + reply->errorString());
        return;
    }

    QString data = (QString) reply->readAll();

    QJsonDocument jsonResponse = QJsonDocument::fromJson(data.toUtf8());

    if(jsonResponse.isEmpty()){
        log(tr("[ERROR] Featured games empty reponse"));
        return;
    }

    QJsonObject jsonObject = jsonResponse.object();

    json_featured.append(jsonObject);

    QJsonArray gamelist = jsonObject.value("gameList").toArray();

    if(gamelist.size() == 0){
        log(tr("[WARN] Featured games empty response"));
        return;
    }

    log(gamelist[0].toObject().value("platformId").toString() + tr(" : Featured games infos"));

    for(int i = 0; i < gamelist.size(); i++){
        GameInfosWidget *widget = new GameInfosWidget(this);
        widget->setGameHeader(gamelist[i].toObject().value("platformId").toString(), QString::number(gamelist[i].toObject().value("gameId").toVariant().toULongLong()), gamelist[i].toObject().value("observers").toObject().value("encryptionKey").toString());
        widget->setGameInfos(QJsonDocument(gamelist.at(i).toObject()));

        ui->listWidget_featured->addItem("");
        ui->listWidget_featured->item(ui->listWidget_featured->count()-1)->setSizeHint(QSize(600, 200));
        ui->listWidget_featured->setItemWidget(ui->listWidget_featured->item(ui->listWidget_featured->count()-1), widget);
    }

}

void MainWindow::slot_featuredRefresh()
{
    ui->listWidget_featured->clear();
    json_featured.clear();

    networkManager_featured->get(QNetworkRequest(QUrl("http://spectator.euw1.lol.riotgames.com/observer-mode/rest/featured")));  // GET EUW FEATURED GAMES

    networkManager_featured->get(QNetworkRequest(QUrl("http://spectator.eu.lol.riotgames.com:8088/observer-mode/rest/featured")));  // GET EUNE FEATURED GAMES

    networkManager_featured->get(QNetworkRequest(QUrl("http://spectator.na.lol.riotgames.com/observer-mode/rest/featured")));  // GET NA FEATURED GAMES

    networkManager_featured->get(QNetworkRequest(QUrl("http://spectator.jp1.lol.riotgames.com/observer-mode/rest/featured")));  // GET JP FEATURED GAMES

    networkManager_featured->get(QNetworkRequest(QUrl("http://spectator.kr.lol.riotgames.com/observer-mode/rest/featured")));  // GET KR FEATURED GAMES

    networkManager_featured->get(QNetworkRequest(QUrl("http://spectator.oc1.lol.riotgames.com/observer-mode/rest/featured")));  // GET OCE FEATURED GAMES

    networkManager_featured->get(QNetworkRequest(QUrl("http://spectator.br.lol.riotgames.com/observer-mode/rest/featured")));  // GET BR LAN LAS FEATURED GAMES

    networkManager_featured->get(QNetworkRequest(QUrl("http://spectator.ru.lol.riotgames.com/observer-mode/rest/featured")));  // GET RU TR FEATURED GAMES

    networkManager_featured->get(QNetworkRequest(QUrl("http://spectator.pbe1.lol.riotgames.com:8088/observer-mode/rest/featured")));  // GET PBE FEATURED GAMES

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

void MainWindow::slot_setdirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open RADS Directory"), loldirectory, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(dir.isEmpty()){
        return;
    }
    loldirectory = dir;
    orsettings->setValue("LoLDirectory", loldirectory);
    ui->lineEdit_lolfolder->setText(dir);
}

void MainWindow::slot_setpbedirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open RADS Directory"), loldirectory, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(dir.isEmpty()){
        return;
    }
    lolpbedirectory = dir;
    orsettings->setValue("LoLPBEDirectory", lolpbedirectory);
    ui->lineEdit_pbefolder->setText(dir);
}

void MainWindow::slot_setreplaydirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), replaydirectory, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(dir.isEmpty()){
        return;
    }
    replaydirectory = dir;
    orsettings->setValue("ReplayDirectory", replaydirectory);
    ui->lineEdit_replaysFolder->setText(dir);
}

void MainWindow::slot_featuredLaunch()
{
    if(ui->listWidget_featured->selectedItems().isEmpty()){
        return;
    }

    slot_refreshPlayingStatus();

    if(playing){
        log(tr("[WARN] Replay aborted. LoL is currently running."));
        return;
    }

    GameInfosWidget *widget = dynamic_cast<GameInfosWidget*>(ui->listWidget_featured->itemWidget(ui->listWidget_featured->selectedItems().first()));

    QString serverid = widget->getServerId();
    QString key = widget->getEncryptionkey();
    QString gameid = widget->getGameId();

    if(game_ended(serverid, gameid)){
        log(tr("Game ") + serverid + "/" + gameid + tr(" has already ended"));
        return;
    }

    lol_launch(serverid, key, gameid);

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
        if(servers.at(i).getRegion() == serverid){
            serveraddress = servers.at(i).getUrl();
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
        log(tr("GAME : ") + serverid + "/" + gameid + tr(" has already finished. Aborting spectator mode."));
        return true;
    }
    else{
        log(tr("GAME : ") + serverid + "/" + gameid + tr(" is in progress. Launching spectator mode."));
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
        log(tr("[ERROR] Get JSON : ") + reply->errorString());
        return jsonEmpty;
    }

    QString data = (QString) reply->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(data.toUtf8());
    reply->deleteLater();

    return jsonResponse;
}

Server MainWindow::getServerByPlatformId(QString platformid)
{
    for(int i = 0; i < servers.size(); i++){
        if(servers.at(i).getPlatformId() == platformid){
            return servers.at(i);
        }
    }

    return Server();
}

void MainWindow::slot_featuredRecord()
{
    if(ui->listWidget_featured->currentItem() == NULL){
        log(tr("Empty selected featured games"));
        return;
    }

    GameInfosWidget* widget(dynamic_cast<GameInfosWidget*>(ui->listWidget_featured->itemWidget(ui->listWidget_featured->currentItem())));

    QString serverid = widget->getServerId();
    QString gameid = widget->getGameId();

    //Get server address
    QString serveraddress;
    for(int i = 0; i < servers.size(); i++){
        if(servers.at(i).getPlatformId() == serverid){
            serveraddress = servers.at(i).getUrl();
            break;
        }
    }
    if(serveraddress.isEmpty()){
        log(tr("Server address not found"));
        return;
    }

    for(int i = 0; i < recording.size(); i++){
        if(recording.at(i).at(0) == serverid && recording.at(i).at(1) == gameid){
            log(tr("Game is already recording"));
            return;
        }
    }

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

    QString dateTime;

    if(gameinfo.isEmpty()){
        dateTime = QDateTime::currentDateTime().toString();
    }
    else{
        dateTime = QDateTime::fromMSecsSinceEpoch(gameinfo.object().value("gameStartTime").toVariant().toLongLong()).toString();
    }

    recording.append(QStringList() << serverid << gameid << dateTime);

    refreshRecordingGamesWidget();

    QThread *recorderThread = new QThread;
    Recorder *recorder = new Recorder(widget->getServerId(), serveraddress, gameid, widget->getEncryptionkey(), gameinfo, replaydirectory);
    recorder->moveToThread(recorderThread);
    connect(recorderThread, SIGNAL(started()), recorder, SLOT(launch()));
    connect(recorder, SIGNAL(finished()), recorderThread, SLOT(quit()));
    connect(recorder, SIGNAL(finished()), recorder, SLOT(deleteLater()));
    connect(recorderThread, SIGNAL(finished()), recorderThread, SLOT(deleteLater()));
    connect(recorder, SIGNAL(end(QString,QString)), this, SLOT(slot_endRecording(QString,QString)));
    connect(recorder, SIGNAL(toLog(QString)), this, SLOT(log(QString)));
    connect(recorder, SIGNAL(toShowmessage(QString)), this, SLOT(showmessage(QString)));

    recorderThread->start();

    recordingThreads.append(recorderThread);
}

void MainWindow::slot_endRecording(QString serverid, QString gameid)
{
    for(int i = 0; i < recording.size(); i++)
    {
        if(recording.at(i).size() >= 2 && recording.at(i).at(0) == serverid && recording.at(i).at(1) == gameid)
        {
            recording.removeAt(i);
            recordingThreads.removeAt(i);
            break;
        }
    }

    emit signal_refresh_recordedGames();

    refreshRecordingGamesWidget();
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
        log(tr("[ERROR] Get Data : ") + reply->errorString());
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

    Replay game(replaydirectory + "/" + recordedgames_filename.at(ui->tableWidget_recordedgames->currentRow()), true);

    ui->label_allgames_gamemode->setText(game.getGameinfos().object().value("gameMode").toString());

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

    if(game.getGameinfos().isEmpty()){
        log(tr("[ERROR] No game infos found. Aborting."));
        return;
    }

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

    if(leftids.size() >= 1 && leftnames.size() >= 1)
    {
        ui->label_sum1->setAlignment(Qt::AlignCenter);
        ui->label_sum1->setPixmap(getImg(leftids.at(0)));

        ui->label_sum16->setText(leftnames.at(0));

        if(leftids.size() >= 2 && leftnames.size() >= 2)
        {
            ui->label_sum2->setAlignment(Qt::AlignCenter);
            ui->label_sum2->setPixmap(getImg(leftids.at(1)));

            ui->label_sum27->setText(leftnames.at(1));

            if(leftids.size() >= 3 && leftnames.size() >= 3)
            {
                ui->label_sum3->setAlignment(Qt::AlignCenter);
                ui->label_sum3->setPixmap(getImg(leftids.at(2)));

                ui->label_sum38->setText(leftnames.at(2));

                if(leftids.size() >= 4 && leftnames.size() >= 4)
                {
                    ui->label_sum4->setAlignment(Qt::AlignCenter);
                    ui->label_sum4->setPixmap(getImg(leftids.at(3)));

                    ui->label_sum49->setText(leftnames.at(3));

                    if(leftids.size() >= 5 && leftnames.size() >= 5){
                        ui->label_sum5->setAlignment(Qt::AlignCenter);
                        ui->label_sum5->setPixmap(getImg(leftids.at(4)));

                        ui->label_sum510->setText(leftnames.at(4));
                    }
                }
            }
        }
    }

    if(rightids.size() >= 1 && rightnames.size() >= 1)
    {
        ui->label_sum6->setAlignment(Qt::AlignCenter);
        ui->label_sum6->setPixmap(getImg(rightids.at(0)));

        ui->label_sum16->setText(ui->label_sum16->text() + " / " + rightnames.at(0));

        if(rightids.size() >= 2 && rightnames.size() >= 2)
        {
            ui->label_sum7->setAlignment(Qt::AlignCenter);
            ui->label_sum7->setPixmap(getImg(rightids.at(1)));

            ui->label_sum27->setText(ui->label_sum27->text() + " / " + rightnames.at(1));

            if(rightids.size() >= 3 && rightnames.size() >= 3)
            {
                ui->label_sum8->setAlignment(Qt::AlignCenter);
                ui->label_sum8->setPixmap(getImg(rightids.at(2)));

                ui->label_sum38->setText(ui->label_sum38->text() + " / " + rightnames.at(2));

                if(rightids.size() >= 4 && rightnames.size() >= 4)
                {
                    ui->label_sum9->setAlignment(Qt::AlignCenter);
                    ui->label_sum9->setPixmap(getImg(rightids.at(3)));

                    ui->label_sum49->setText(ui->label_sum49->text() + " / " + rightnames.at(3));

                    if(rightids.size() >= 5 && rightnames.size() >= 5){
                        ui->label_sum10->setAlignment(Qt::AlignCenter);
                        ui->label_sum10->setPixmap(getImg(rightids.at(4)));

                        ui->label_sum510->setText(ui->label_sum510->text() + " / " + rightnames.at(4));
                    }
                }
            }
        }
    }
}

void MainWindow::slot_refresh_recordedGames()
{
    //Find saved replays

    recordedgames_filename.clear();
    yourgames_filename.clear();

    QDir dirreplays(replaydirectory);
    dirreplays.setFilter(QDir::Files | QDir::Readable);
    dirreplays.setSorting(QDir::Time);

    QFileInfoList replayslist = dirreplays.entryInfoList();

    ui->tableWidget_recordedgames->clearContents();

    while(ui->tableWidget_recordedgames->rowCount() > 0){
        ui->tableWidget_recordedgames->removeRow(0);
    }

    ui->tableWidget_yourgames->clearContents();

    while(ui->tableWidget_yourgames->rowCount() > 0){
        ui->tableWidget_yourgames->removeRow(0);
    }

    for(int i = 0; i < replayslist.size(); i++){

        QFileInfo fileinfo = replayslist.at(i);

        if(fileinfo.suffix() == "lor"){
            ui->tableWidget_recordedgames->insertRow(ui->tableWidget_recordedgames->rowCount());

            Replay game(fileinfo.filePath(), true);

            QDateTime datetime;
            datetime.setMSecsSinceEpoch(quint64(game.getGameinfos().object().value("gameStartTime").toVariant().toLongLong()));

            ui->tableWidget_recordedgames->setItem(ui->tableWidget_recordedgames->rowCount()-1, 0, new QTableWidgetItem(game.getPlatformId()));
            ui->tableWidget_recordedgames->setItem(ui->tableWidget_recordedgames->rowCount()-1, 1, new QTableWidgetItem(game.getGameId()));
            ui->tableWidget_recordedgames->setItem(ui->tableWidget_recordedgames->rowCount()-1, 2, new QTableWidgetItem(datetime.date().toString()));
            ui->tableWidget_recordedgames->setItem(ui->tableWidget_recordedgames->rowCount()-1, 3, new QTableWidgetItem(fileinfo.fileName()));

            recordedgames_filename.append(fileinfo.fileName());

            //Get platformId of the user
            QString local_platformid;

            for(int k = 0; k < servers.size(); k++){
                if(servers.at(k).getRegion() == m_summonerserver){
                    local_platformid = servers.at(k).getPlatformId();
                    break;
                }
            }

            if(!local_platformid.isEmpty() && game.getPlatformId() == local_platformid)
            {
                if(!m_summonerid.isEmpty() && !game.getGameinfos().isEmpty() && !game.getGameinfos().object().value("participants").toArray().isEmpty())
                {
                    for(int j = 0; j < game.getGameinfos().object().value("participants").toArray().size(); j++)
                    {
                        if(game.getGameinfos().object().value("participants").toArray().at(j).toObject().value("summonerName").toString() == m_summonername){
                            ui->tableWidget_yourgames->insertRow(ui->tableWidget_yourgames->rowCount());

                            QLabel* label = new QLabel;
                            label->setAlignment(Qt::AlignCenter);
                            label->setPixmap(getImg(game.getGameinfos().object().value("participants").toArray().at(j).toObject().value("championId").toInt()));

                            ui->tableWidget_yourgames->setCellWidget(ui->tableWidget_yourgames->rowCount()-1, 0, label);
                            ui->tableWidget_yourgames->setItem(ui->tableWidget_yourgames->rowCount()-1, 1, new QTableWidgetItem(tr("Soon")));
                            ui->tableWidget_yourgames->setItem(ui->tableWidget_yourgames->rowCount()-1, 2, new QTableWidgetItem(datetime.toString(Qt::SystemLocaleLongDate)));
                            ui->tableWidget_yourgames->setItem(ui->tableWidget_yourgames->rowCount()-1, 3, new QTableWidgetItem(game.getPlatformId()));
                            ui->tableWidget_yourgames->setItem(ui->tableWidget_yourgames->rowCount()-1, 4, new QTableWidgetItem(fileinfo.fileName()));

                            yourgames_filename.append(fileinfo.fileName());

                            break;
                        }
                    }
                }
            }
        }
    }
}

void MainWindow::slot_replayserversAdd()
{
    if(ui->lineEdit_replayserver_address->text().isEmpty()){
        return;
    }

    QFile serversfile(QStandardPaths::standardLocations(QStandardPaths::DataLocation).first() + "/LegendsReplayServers.txt");
    QFile localserversfile(":/data/LegendsReplayServers.txt");

    if(!serversfile.open(QIODevice::ReadWrite | QIODevice::Text)){
        log(tr("[WARN] Unable to write to LegendsReplayServers.txt file : ") + serversfile.errorString());
        return;
    }
    else{
        if(!localserversfile.open(QIODevice::ReadOnly | QIODevice::Text)){
            log(tr("[ERROR] Unable to open internal servers file : ") + serversfile.errorString());
            return;
        }

        QTextStream file_stream(&serversfile);
        QTextStream local_in(&localserversfile);

        lrservers.clear();

        while(!local_in.atEnd()){
            QString line = local_in.readLine();
            if(!line.isEmpty()){
                lrservers.append(line);
            }
        }

        while(!file_stream.atEnd()){
            QString line = file_stream.readLine();
            if(!line.isEmpty() && !lrservers.contains(line)){
                lrservers.append(line);
            }
        }

        file_stream << "\n" << ui->lineEdit_replayserver_address->text();

        serversfile.close();
        localserversfile.close();
    }

    lrservers.append(ui->lineEdit_replayserver_address->text());

    ui->tableWidget_replayservers->insertRow(ui->tableWidget_replayservers->rowCount());
    ui->tableWidget_replayservers->setItem(ui->tableWidget_replayservers->rowCount()-1,0,new QTableWidgetItem(ui->lineEdit_replayserver_address->text()));

    ui->lineEdit_replayserver_address->clear();
}

void MainWindow::slot_summonerinfos_save()
{
    if(ui->lineEdit_summonername->text().isEmpty()){
        return;
    }

    if(lrservers.isEmpty()){
        QMessageBox::information(this, tr("LegendsReplay"), tr("Please add a LegendsReplay server."));
        log(tr("Please add a LegendsReplay server."));
        return;
    }

    m_summonername = ui->lineEdit_summonername->text();
    m_summonerserver = ui->comboBox_summonerserver->currentText();

    //Retrieving summoner ID

    log("Retrieving summoner ID");

    QJsonDocument suminfos = getJsonFromUrl("http://" + m_currentLegendsReplayServer + "?region=" + m_summonerserver + "&summonername=" + m_summonername);

    if(suminfos.isEmpty()){
        QMessageBox::information(this, tr("LegendsReplay"), tr("Unknown summoner on this server."));
        log(tr("Unknown summoner on this server."));
        return;
    }
    else{
        m_summonerid = QString::number(suminfos.object().value(suminfos.object().keys().first()).toObject().value("id").toVariant().toLongLong());
        ui->lineEdit_summonerid->setText(m_summonerid);
        log(tr("Your summoner ID is ") + m_summonerid);

        orsettings->setValue("SummonerName", m_summonername);
        orsettings->setValue("SummonerId", m_summonerid);
        orsettings->setValue("SummonerServer", m_summonerserver);
    }
}

void MainWindow::slot_pbeinfos_save()
{
    if(ui->lineEdit_pbename->text().isEmpty()){
        return;
    }

    if(lrservers.isEmpty()){
        QMessageBox::information(this, tr("LegendsReplay"), tr("Please add a LegendsReplay server."));
        log(tr("Please add a LegendsReplay server."));
        return;
    }

    m_PBEname = ui->lineEdit_pbename->text();

    //Retrieving PBE ID

    log(tr("Retrieving PBE ID"));

    QJsonDocument suminfos = getJsonFromUrl("http://" + m_currentLegendsReplayServer + "?region=PBE&summonername=" + m_PBEname);

    if(suminfos.isEmpty()){
        QMessageBox::information(this, tr("LegendsReplay"), tr("Unknown summoner on this server.\nPBE is not supported."));
        log(tr("Unknown summoner on this server. PBE is not supported."));
        return;
    }
    else{
        m_PBEid = QString::number(suminfos.object().value(suminfos.object().keys().first()).toObject().value("id").toVariant().toLongLong());
        ui->lineEdit_pbeid->setText(m_PBEid);
        log(tr("Your PBE ID is ") + m_PBEid);

        orsettings->setValue("PBEName", m_PBEname);
        orsettings->setValue("PBEId", m_PBEid);
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
            log(tr("Server: stoped due to inactivity"));
        }
    }

    if(!replaying && islolclientRunning() && islolRunning()){
        if(!playing){
            //Start recording the game
            playing = true;

            log(tr("Game detected : start recording"));

            QJsonDocument gameinfos = getCurrentPlayingGameInfos(m_summonerserver, m_summonerid);

            QTimer timer;
            QEventLoop loop;
            connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
            unsigned int counter = 0;
            timer.start(150000);

            while(gameinfos.isEmpty() || gameinfos.object().value("gameStartTime").toVariant().toLongLong() == 0){
                loop.exec();
                if(counter > 5){
                    log(tr("[WARN] Game not found : API/LegendsReplay servers may be offline or not working correctly"));
                    log(tr("End of recording"));
                    return;
                }
                counter++;
                gameinfos = getCurrentPlayingGameInfos(m_summonerserver, m_summonerid);
                timer.start(45000);
            }

            QString serverid, serveraddress;
            QString gameid = QString::number(gameinfos.object().value("gameId").toVariant().toLongLong());

            for(int i = 0; i < servers.size(); i++){
                if(servers.at(i).getRegion() == m_summonerserver){
                    serverid = servers.at(i).getPlatformId();
                    serveraddress = servers.at(i).getUrl();
                    break;
                }
            }

            if(serverid.isEmpty()){
                log(tr("[ERROR] Server not found"));
                return;
            }

            QString dateTime = QDateTime::fromMSecsSinceEpoch(gameinfos.object().value("gameStartTime").toVariant().toLongLong()).toString();

            recording.append(QStringList() << serverid << gameid << dateTime);

            refreshRecordingGamesWidget();

            Recorder *recorder = new Recorder(serverid, serveraddress, gameid, gameinfos.object().value("observers").toObject().value("encryptionKey").toString(), gameinfos, replaydirectory);
            QThread *recorderThread = new QThread;
            recorder->moveToThread(recorderThread);
            connect(recorderThread, SIGNAL(started()), recorder, SLOT(launch()));
            connect(recorder, SIGNAL(finished()), recorderThread, SLOT(quit()));
            connect(recorder, SIGNAL(finished()), recorder, SLOT(deleteLater()));
            connect(recorderThread, SIGNAL(finished()), recorderThread, SLOT(deleteLater()));
            connect(recorder, SIGNAL(end(QString,QString)), this, SLOT(slot_endRecording(QString,QString)));
            connect(recorder, SIGNAL(toLog(QString)), this, SLOT(log(QString)));
            connect(recorder, SIGNAL(toShowmessage(QString)), this, SLOT(showmessage(QString)));

            recorderThread->start();

            recordingThreads.append(recorderThread);
        }
    }

    m_timer->start(60000);

    refreshRecordingGamesWidget();
}

QJsonDocument MainWindow::getCurrentPlayingGameInfos(QString server, QString summonerid)
{
    QString servertag;

    for(int i = 0; i < servers.size(); i++){
        if(servers.at(i).getRegion() == server){
            servertag = servers.at(i).getPlatformId();
            break;
        }
    }

    if(servertag.isEmpty() || lrservers.isEmpty()){
        QJsonDocument docempty;
        log(tr("[ERROR] Unknown server"));
        return docempty;
    }

    QJsonDocument gameinfos = getJsonFromUrl("http://" + m_currentLegendsReplayServer + "?platformid=" + servertag + "&summonerid=" + summonerid);

    return gameinfos;
}

void MainWindow::slot_open_replay(bool param)
{
    Q_UNUSED(param);

    QString path = QFileDialog::getOpenFileName(this, tr("Select a Replay"), replaydirectory);
    if(!path.isEmpty()){
        replay_launch(path);
    }
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

    replay->repair();

    log(tr("Opening : ") + pathfile);

    log(tr("Server: started"));

    httpserver->stopListening();

    if(replay->getGameId().isEmpty()){
        log(tr("Invalid replay file, aborting."));
        delete replay;
        replay = NULL;
        return;
    }

    httpserver->listen(QHostAddress::Any, 8088, [this](QHttpRequest* req, QHttpResponse* res) {
        QString url = req->url().toString();

        if(url == "/observer-mode/rest/consumer/version"){
            res->setStatusCode(qhttp::ESTATUS_OK);      // http status 200
            res->addHeader("Content-Type", "text/plain");
            res->end(replay->getServerVersion().toLocal8Bit());
            log(tr("Server: send server version"));
        }
        else if(url.contains("/observer-mode/rest/consumer/getGameMetaData/" + replay->getPlatformId() + "/" + replay->getGameId())){
            int firstchunkid = replay->getChunks().first().getId();

            while(replay->findKeyframeByChunkId(firstchunkid).getId() == 0)
            {
                firstchunkid++;
            }

            QString metadata = "{\"gameKey\":{";
            metadata.append("\"gameId\":" + replay->getGameId());
            metadata.append(",\"platformId\":\"" + replay->getPlatformId() + "\"}");
            metadata.append(",\"gameServerAddress\":\"\"");
            metadata.append(",\"port\":0");
            metadata.append(",\"encryptionKey\":\"\"");
            metadata.append(",\"chunkTimeInterval\":30000");
            metadata.append(",\"gameEnded\":false");
            metadata.append(",\"lastChunkId\":" + QString::number(firstchunkid));
            metadata.append(",\"lastKeyFrameId\":" + QString::number(replay->findKeyframeByChunkId(firstchunkid).getId()));
            metadata.append(",\"endStartupChunkId\":" + replay->getEndStartupChunkId());
            metadata.append(",\"delayTime\":150000");
            metadata.append(",\"keyFrameTimeInterval\":60000");
            metadata.append(",\"decodedEncryptionKey\":\"\"");
            metadata.append(",\"startGameChunkId\":" + replay->getStartGameChunkId());
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

            log(tr("Server: send game metadata"));
        }
        else if(url.contains("/observer-mode/rest/consumer/getLastChunkInfo/" + replay->getPlatformId() + "/" + replay->getGameId()))
        {
            int endstartupchunkid = replay->getEndStartupChunkId().toInt();
            int startgamechunkid = replay->getStartGameChunkId().toInt();
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
            }
            else{
                serverKeyframeCount = replay->findKeyframeByChunkId(serverChunkCount).getId();
            }

            int currentChunkid = serverChunkCount;

            Keyframe currentKeyframe = replay->findKeyframeByChunkId(currentChunkid);

            while(currentKeyframe.getId() == 0 && currentChunkid < replay->getChunks().last().getId())
            {
                currentChunkid++;
                serverChunkCount++;
                currentKeyframe = replay->findKeyframeByChunkId(currentChunkid);
                serverKeyframeCount = currentKeyframe.getId();
            }

            if(serverKeyframeCount == replay->getKeyFrames().first().getId()){
                nextavailablechunk = 1000;
            }
            else if(serverKeyframeCount < replay->getKeyFrames().first().getId() + 4){
                nextavailablechunk = 30000;
            }
            else if(serverKeyframeCount < replay->getKeyFrames().first().getId() + 10){
                nextavailablechunk = 2000;
            }
            else{
                nextavailablechunk = 500;
            }

            QString data = "{";
            data.append("\"chunkId\":" + QString::number(currentChunkid));
            data.append(",\"availableSince\":" + QString::number(30000 - nextavailablechunk));
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

            log(tr("Server: send lastChunkInfo"));
        }
        else if(url.contains("/observer-mode/rest/consumer/getGameDataChunk/" + replay->getPlatformId() + "/" + replay->getGameId()))
        {
            int chunkid = -1;

            //Get and send the chunk
            url.remove("/observer-mode/rest/consumer/getGameDataChunk/" + replay->getPlatformId() + "/" + replay->getGameId() + "/");
            chunkid = url.left(url.indexOf("/")).toInt();

            Chunk chunk = replay->getChunk(chunkid);
            Chunk primarychunk = replay->getPrimaryChunk(chunkid);

            if(chunk.getId() > 0){
                QByteArray chunk_ba = chunk.getData();

                res->setStatusCode(qhttp::ESTATUS_OK);
                res->addHeader("Content-Type", "application/octet-stream");
                res->addHeader("Content-Length", QString::number(chunk_ba.size()).toLocal8Bit());
                res->end(chunk_ba);

                if(serverChunkCount >= replay->getEndStartupChunkId().toInt() && chunkid > replay->getEndStartupChunkId().toInt()){
                    serverChunkCount++;
                }

                log(tr("Server: send chunk ") + QString::number(chunkid));
            }
            else if(primarychunk.getId() > 0){
                QByteArray chunk_ba = primarychunk.getData();

                res->setStatusCode(qhttp::ESTATUS_OK);
                res->addHeader("Content-Type", "application/octet-stream");
                res->addHeader("Content-Length", QString::number(chunk_ba.size()).toLocal8Bit());
                res->end(chunk_ba);

                log(tr("Server: send primary chunk ") + QString::number(chunkid));
            }
            else
            {
                res->setStatusCode(qhttp::ESTATUS_NOT_FOUND);
                res->end("");
                log(tr("Server: unknown requested chunk ") + QString::number(chunkid));
            }
        }
        else if(url.contains("/observer-mode/rest/consumer/getKeyFrame/" + replay->getPlatformId() + "/" + replay->getGameId()))
        {
            int keyframeid = -1;

            //Get and send the keyframe
            url.remove("/observer-mode/rest/consumer/getKeyFrame/" + replay->getPlatformId() + "/" + replay->getGameId() + "/");
            keyframeid = url.left(url.indexOf("/")).toInt();

            Keyframe keyframe = replay->getKeyFrame(keyframeid);

            if(keyframe.getId() > 0){
                QByteArray keyframe_ba = keyframe.getData();

                res->setStatusCode(qhttp::ESTATUS_OK);
                res->addHeader("Content-Type", "application/octet-stream");
                res->end(keyframe_ba);

                log(tr("Server: send keyframe ") + QString::number(keyframeid));
            }
            else{
                log(tr("Server: unknown requested keyframe ") + QString::number(keyframeid));
            }
        }
        else if(url.contains("/observer-mode/rest/consumer/endOfGameStats/" + replay->getPlatformId() + "/" + replay->getGameId()))
        {
            //End of game stats requested
            res->setStatusCode(qhttp::ESTATUS_OK);
            res->addHeader("Content-Type", "application/octet-stream");
            res->end(replay->getEndOfGameStats());

            log(tr("Server: End of game stats sent"));
        }
        else if(url.contains("/observer-mode/rest/consumer/end/" + replay->getPlatformId() + "/" + replay->getGameId()))
        {
            //End of replay requested : error while replaying
            res->setStatusCode(qhttp::ESTATUS_OK);
            res->addHeader("Content-Type", "text/plain");
            res->end("");

            log(tr("Server: End of replay requested"));

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

            log(tr("Server: Unknown requested link ") + url);
        }
    });

    //Launch lol client
    lol_launch(replay->getPlatformId(), replay->getEncryptionkey(), replay->getGameId(),true);
}

void MainWindow::showmessage(QString message)
{
    if(systemtrayavailable)
    {
        systemtrayicon->showMessage(tr("LegendsReplay"), message, QSystemTrayIcon::Information);
        m_currentsystemtraymessage = message;
    }
}

void MainWindow::slot_messageclicked()
{
    if(systemtrayavailable){
        if(m_currentsystemtraymessage.contains(tr("New version "))){
            QDesktopServices::openUrl(QUrl("http://aztorius.github.io/legendsreplay/"));
        }
    }
}

void MainWindow::slot_searchsummoner()
{
    ui->label_searchsummoner_status->setText("...");

    QString serverid = ui->comboBox_searchsummoner_platformid->currentText();
    QString summonername = ui->lineEdit_searchsummoner->text();

    QJsonDocument suminfos = getJsonFromUrl("http://" + m_currentLegendsReplayServer + "?region=" + serverid + "&summonername=" + summonername);

    if(suminfos.isEmpty())
    {
        log(tr("Summoner name not found on this server"));
        return;
    }

    QString summonerid = QString::number(suminfos.object().value(suminfos.object().keys().first()).toObject().value("id").toVariant().toLongLong());

    if(summonerid.isEmpty())
    {
        log(tr("[ERROR] Summoner id not valid"));
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
        log(tr("Summoner ") + summonername + tr(" is not in a game"));

        ui->label_searchsummoner_status->setText(tr("not playing"));

        ui->label_searchsummoner_gamemode->setText(tr("Game Mode"));
    }
    else
    {
        log(tr("Summoner ") + summonername + tr(" is in a game"));

        ui->label_searchsummoner_status->setText(tr("playing"));

        ui->label_searchsummoner_gamemode->setText(game.object().value("gameMode").toString());

        QJsonArray array = game.object().value("participants").toArray();

        QList<int> leftids;
        QList<QString> leftnames;
        QList<int> rightids;
        QList<QString> rightnames;

        if(array.isEmpty()){
            return;
        }

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

        if(leftids.size() >= 1 && leftnames.size() >= 1)
        {
            ui->label_sums1->setAlignment(Qt::AlignCenter);
            ui->label_sums1->setPixmap(getImg(leftids.at(0)));

            ui->label_sums16->setText(leftnames.at(0));

            if(leftids.size() >= 2 && leftnames.size() >= 2)
            {
                ui->label_sums2->setAlignment(Qt::AlignCenter);
                ui->label_sums2->setPixmap(getImg(leftids.at(1)));

                ui->label_sums27->setText(leftnames.at(1));

                if(leftids.size() >= 3 && leftnames.size() >= 3)
                {
                    ui->label_sums3->setAlignment(Qt::AlignCenter);
                    ui->label_sums3->setPixmap(getImg(leftids.at(2)));

                    ui->label_sums38->setText(leftnames.at(2));

                    if(leftids.size() >= 4 && leftnames.size() >= 4)
                    {
                        ui->label_sums4->setAlignment(Qt::AlignCenter);
                        ui->label_sums4->setPixmap(getImg(leftids.at(3)));

                        ui->label_sums49->setText(leftnames.at(3));

                        if(leftids.size() >= 5 && leftnames.size() >= 5){
                            ui->label_sums5->setAlignment(Qt::AlignCenter);
                            ui->label_sums5->setPixmap(getImg(leftids.at(4)));

                            ui->label_sums510->setText(leftnames.at(4));
                        }
                    }
                }
            }
        }

        if(rightids.size() >= 1 && rightnames.size() >= 1)
        {
            ui->label_sums6->setAlignment(Qt::AlignCenter);
            ui->label_sums6->setPixmap(getImg(rightids.at(0)));

            ui->label_sums16->setText(ui->label_sums16->text() + " / " + rightnames.at(0));

            if(rightids.size() >= 2 && rightnames.size() >= 2)
            {
                ui->label_sums7->setAlignment(Qt::AlignCenter);
                ui->label_sums7->setPixmap(getImg(rightids.at(1)));

                ui->label_sums27->setText(ui->label_sums27->text() + " / " + rightnames.at(1));

                if(rightids.size() >= 3 && rightnames.size() >= 3)
                {
                    ui->label_sums8->setAlignment(Qt::AlignCenter);
                    ui->label_sums8->setPixmap(getImg(rightids.at(2)));

                    ui->label_sums38->setText(ui->label_sums38->text() + " / " + rightnames.at(2));

                    if(rightids.size() >= 4 && rightnames.size() >= 4)
                    {
                        ui->label_sums9->setAlignment(Qt::AlignCenter);
                        ui->label_sums9->setPixmap(getImg(rightids.at(3)));

                        ui->label_sums49->setText(ui->label_sums49->text() + " / " + rightnames.at(3));

                        if(rightids.size() >= 5 && rightnames.size() >= 5){
                            ui->label_sums10->setAlignment(Qt::AlignCenter);
                            ui->label_sums10->setPixmap(getImg(rightids.at(4)));

                            ui->label_sums510->setText(ui->label_sums510->text() + " / " + rightnames.at(4));
                        }
                    }
                }
            }
        }
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
    QString platformid = m_searchsummoner_game.object().value("platformId").toString();
    QString gameid = QString::number(m_searchsummoner_game.object().value("gameId").toVariant().toULongLong());

    for(int i = 0; i < servers.size(); i++){
        if(servers.at(i).getPlatformId() == platformid){
            serveraddress = servers.at(i).getUrl();
            break;
        }
    }
    if(serveraddress.isEmpty()){
        return;
    }

    for(int i = 0; i < recording.size(); i++){
        if(recording.at(i).at(0) == platformid && recording.at(i).at(1) == gameid){
            log(tr("Game is already recording"));
            return;
        }
    }

    QString dateTime;

    if(m_searchsummoner_game.object().value("gameStartTime").toVariant().toULongLong() > 0){
        dateTime = QDateTime::currentDateTime().toString();
    }
    else{
        dateTime = QDateTime::fromMSecsSinceEpoch(m_searchsummoner_game.object().value("gameStartTime").toVariant().toLongLong()).toString();
    }

    recording.append(QStringList() << platformid << gameid << dateTime);

    refreshRecordingGamesWidget();

    Recorder *recorder = new Recorder(platformid, serveraddress, gameid, m_searchsummoner_game.object().value("observers").toObject().value("encryptionKey").toString(), m_searchsummoner_game, replaydirectory);
    QThread *recorderThread = new QThread;
    recorder->moveToThread(recorderThread);
    connect(recorderThread, SIGNAL(started()), recorder, SLOT(launch()));
    connect(recorder, SIGNAL(finished()), recorderThread, SLOT(quit()));
    connect(recorder, SIGNAL(finished()), recorder, SLOT(deleteLater()));
    connect(recorderThread, SIGNAL(finished()), recorderThread, SLOT(deleteLater()));
    connect(recorder, SIGNAL(end(QString,QString)), this, SLOT(slot_endRecording(QString,QString)));
    connect(recorder, SIGNAL(toLog(QString)), this, SLOT(log(QString)));
    connect(recorder, SIGNAL(toShowmessage(QString)), this, SLOT(showmessage(QString)));

    recorderThread->start();

    recordingThreads.append(recorderThread);
}

void MainWindow::systemtrayiconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::DoubleClick)
    {
        this->showNormal();
    }
}

void MainWindow::slot_click_replayservers()
{
    if(ui->tableWidget_replayservers->selectedItems().size() == 0){
        return;
    }

    m_currentLegendsReplayServer = ui->tableWidget_replayservers->itemAt(ui->tableWidget_replayservers->selectedItems().first()->row(), 0)->text();

    log(tr("Legends Replay switch to server ") + m_currentLegendsReplayServer);
}

void MainWindow::slot_customcontextmenu(QPoint point)
{
    Q_UNUSED(point);

    QMenu *menu = new QMenu(tr("Options"), this);

    if(ui->tabWidget->currentIndex() == 0){
        menu->addAction(QIcon(":/icons/open_replay.png"), tr("Replay"));
        menu->addAction(QIcon(":/icons/stats.png"), tr("Stats"));
        menu->addSeparator();
        menu->addAction(QIcon(":/icons/repair.png"), tr("Repair tool"));
        menu->addSeparator();
        menu->addAction(QIcon(":/icons/delete.png"), tr("Delete"));
    }
    else if(ui->tabWidget->currentIndex() == 2){
        menu->addAction(QIcon(":/icons/open_replay.png"), tr("Spectate"));
        menu->addAction(QIcon(":/icons/record.png"), tr("Record"));
    }
    else if(ui->tabWidget->currentIndex() == 3){
        menu->addAction(QIcon(":/icons/cancel_download.png"), tr("Cancel"));
        menu->addAction(QIcon(":/icons/cancel_delete_download.png"), tr("Cancel and delete"));
    }

    menu->move(QCursor::pos());
    menu->show();
    connect(menu, SIGNAL(triggered(QAction*)), this,  SLOT(slot_custommenutriggered(QAction*)));
}

void MainWindow::slot_custommenutriggered(QAction *action)
{
    if(ui->tabWidget->currentIndex() == 0){
        if(ui->tabWidget_2->currentIndex() == 1){
            if(!ui->tableWidget_recordedgames->selectedItems().isEmpty()){
                QString path = replaydirectory + "/" + recordedgames_filename.at(ui->tableWidget_recordedgames->selectedItems().first()->row());
                if(action->text() == tr("Delete")){
                    if(QMessageBox::question(this, tr("Delete"), tr("Do you really want to delete this replay ?")) != QMessageBox::Yes){
                        return;
                    }

                    if(!QFile::exists(path)){
                        log(tr("Unable to find the file : ") + path);
                    }
                    else if(!QFile::remove(path)){
                        log(tr("Unable to remove the file : ") + path);
                    }
                    else if(QFile::exists(path)){
                        log(tr("Unable to remove the file : ") + path);
                    }
                }
                else if(action->text() == tr("Replay")){
                    replay_launch(path);
                }
                else if(action->text() == tr("Stats")){
                    Replay local_replay(path, true);

                    if(local_replay.getGameId().isEmpty()){
                        return;
                    }

                    QString serverRegion;
                    for(int i = 0; i < servers.size(); i++){
                        if(servers.at(i).getPlatformId() == local_replay.getPlatformId()){
                            serverRegion = servers.at(i).getRegion();
                            break;
                        }
                    }

                    if(serverRegion.isEmpty()){
                        return;
                    }

                    if(serverRegion.toLower() == "kr"){
                        QDesktopServices::openUrl(QUrl("http://matchhistory.leagueoflegends.co.kr/en/#match-details/KR/" + local_replay.getGameId() + "?tab=overview"));
                    }
                    else{
                        QDesktopServices::openUrl(QUrl("http://matchhistory." + serverRegion.toLower() + ".leagueoflegends.com/en/#match-details/" + local_replay.getPlatformId() + "/" + local_replay.getGameId() + "?tab=overview"));
                    }
                }
                else if(action->text() == tr("Repair tool")){
                    RepairToolDialog *newRepairToolDialog = new RepairToolDialog(this);
                    newRepairToolDialog->show();
                    newRepairToolDialog->load(Replay(path));
                }
            }
        }
        else if(ui->tabWidget_2->currentIndex() == 0){
            if(!ui->tableWidget_yourgames->selectedItems().isEmpty()){
                QString path = replaydirectory + "/" + ui->tableWidget_yourgames->item(ui->tableWidget_yourgames->selectedItems().first()->row(), 4)->text();
                if(action->text() == tr("Delete")){
                    if(QMessageBox::question(this, tr("Delete"), tr("Do you really want to delete this replay ?")) != QMessageBox::Yes){
                        return;
                    }

                    if(!QFile::exists(path)){
                        log(tr("Unable to find the file : ") + path);
                    }
                    else if(!QFile::remove(path)){
                        log(tr("Unable to remove the file : ") + path);
                    }
                    else if(QFile::exists(path)){
                        log(tr("Unable to remove the file : ") + path);
                    }
                }
                else if(action->text() == tr("Replay")){
                    replay_launch(path);
                }
                else if(action->text() == tr("Stats")){
                    Replay local_replay(path, true);

                    if(local_replay.getGameId().isEmpty()){
                        return;
                    }

                    QString servername;
                    for(int i = 0; i < servers.size(); i++){
                        if(servers.at(i).getPlatformId() == local_replay.getPlatformId()){
                            servername = servers.at(i).getRegion();
                            break;
                        }
                    }

                    if(servername.isEmpty()){
                        return;
                    }

                    if(servername.toLower() == "kr"){
                        QDesktopServices::openUrl(QUrl("http://matchhistory.leagueoflegends.co.kr/en/#match-details/KR/" + local_replay.getGameId() + "?tab=overview"));
                    }
                    else{
                        QDesktopServices::openUrl(QUrl("http://matchhistory." + servername.toLower() + ".leagueoflegends.com/en/#match-details/" + local_replay.getPlatformId() + "/" + local_replay.getGameId() + "?tab=overview"));
                    }
                }
                else if(action->text() == tr("Repair tool")){
                    RepairToolDialog *newRepairToolDialog = new RepairToolDialog(this);
                    newRepairToolDialog->show();
                    newRepairToolDialog->load(Replay(path));
                }
            }
        }
    }
    else if(ui->tabWidget->currentIndex() == 2){
        if(!ui->listWidget_featured->selectedItems().isEmpty()){
            if(action->text() == tr("Spectate")){
                GameInfosWidget* widget = dynamic_cast<GameInfosWidget*>(ui->listWidget_featured->itemWidget(ui->listWidget_featured->selectedItems().first()));

                lol_launch(widget->getServerId(), widget->getEncryptionkey(), widget->getGameId());
            }
            else if(action->text() == tr("Record")){
                slot_featuredRecord();
            }
        }
    }
    else if(ui->tabWidget->currentIndex() == 3){
        if(!ui->tableWidget_recordingGames->selectedItems().isEmpty()){
            if(action->text() == tr("Cancel")){
                if(QMessageBox::question(this, tr("Cancel"), tr("Do you really want to cancel this record ?")) != QMessageBox::Yes){
                    return;
                }

                int j = -1;
                int row = ui->tableWidget_recordingGames->selectedItems().first()->row();

                for(int i = 0; i < recording.size(); i++){
                    if(recording.at(i).at(0) == ui->tableWidget_recordingGames->item(row, 0)->text() && recording.at(i).at(1) == ui->tableWidget_recordingGames->item(row, 1)->text()){
                        j = i;
                    }
                }

                if(j < 0){
                    return;
                }

                if(recordingThreads.size() > j){
                    recordingThreads.at(j)->exit(0);
                }
            }
            else if(action->text() == tr("Cancel and delete")){
                if(QMessageBox::question(this, tr("Cancel"), tr("Do you really want to cancel and delete this record ?")) != QMessageBox::Yes){
                    return;
                }

                int j = -1;
                int row = ui->tableWidget_recordingGames->selectedItems().first()->row();

                for(int i = 0; i < recording.size(); i++){
                    if(recording.at(i).at(0) == ui->tableWidget_recordingGames->item(row, 0)->text() && recording.at(i).at(1) == ui->tableWidget_recordingGames->item(row, 1)->text()){
                        j = i;
                    }
                }

                if(j < 0){
                    return;
                }

                QString filepath = replaydirectory + "/" + recording.at(j).at(0) + "-" + recording.at(j).at(1) + ".lor";

                if(recordingThreads.size() > j){
                    recordingThreads.at(j)->exit(0);
                    recordingThreads.at(j)->wait(1000);
                }

                QFile file(filepath);

                if(!file.exists()){
                    return;
                }

                if(file.open(QIODevice::WriteOnly) && file.remove()){
                    log(tr("Removed file : ") + filepath);
                }
                else{
                    log(tr("[ERROR] Unable to remove the file : ") + file.errorString());
                }
            }
        }
    }

    emit signal_refresh_recordedGames();
}

void MainWindow::slot_openAdvancedRecorder()
{
    AdvancedRecorderDialog *newAdvancedRecorderDialog = new AdvancedRecorderDialog(this);
    newAdvancedRecorderDialog->show();
    connect(newAdvancedRecorderDialog, SIGNAL(recordGame(QString, QString, QString, QString, bool, bool, bool)), this, SLOT(slot_customGameRecord(QString, QString, QString, QString, bool, bool, bool)));
}

void MainWindow::slot_customGameRecord(QString serverAddress, QString serverRegion, QString gameId, QString encryptionKey, bool forceCompleteDownload, bool downloadInfos, bool downloadStats)
{
    Q_UNUSED(downloadInfos);

    for(int i = 0; i < recording.size(); i++){
        if(recording.at(i).at(0) == serverRegion && recording.at(i).at(1) == gameId){
            log(tr("Game is already recording"));
            return;
        }
    }

    recording.append(QStringList() << serverRegion << gameId << QDateTime::currentDateTime().toString());

    refreshRecordingGamesWidget();

    Recorder *recorder = new Recorder(serverRegion, serverAddress, gameId, encryptionKey, QJsonDocument(), replaydirectory, forceCompleteDownload, downloadStats);
    QThread *recorderThread = new QThread;
    recorder->moveToThread(recorderThread);
    connect(recorderThread, SIGNAL(started()), recorder, SLOT(launch()));
    connect(recorder, SIGNAL(finished()), recorderThread, SLOT(quit()));
    connect(recorder, SIGNAL(finished()), recorder, SLOT(deleteLater()));
    connect(recorderThread, SIGNAL(finished()), recorderThread, SLOT(deleteLater()));
    connect(recorder, SIGNAL(end(QString,QString)), this, SLOT(slot_endRecording(QString,QString)));
    connect(recorder, SIGNAL(toLog(QString)), this, SLOT(log(QString)));
    connect(recorder, SIGNAL(toShowmessage(QString)), this, SLOT(showmessage(QString)));

    recorderThread->start();

    recordingThreads.append(recorderThread);
}

void MainWindow::slot_reportAnIssue()
{
    QDesktopServices::openUrl(QUrl("https://github.com/Aztorius/legendsreplay/issues"));
}

void MainWindow::slot_aboutLegendsReplay()
{
    QMessageBox::information(this, tr("About"), tr("Legends Replay is an open source software (GNU GPL v3).\nThis software use Qt and qHttp.") + "\n\nLegendsReplay isn't endorsed by Riot Games and doesn't reflect the views or opinions of Riot Games or anyone officially involved in producing or managing League of Legends. League of Legends and Riot Games are trademarks or registered trademarks of Riot Games, Inc. League of Legends © Riot Games, Inc.");
}

void MainWindow::refreshRecordingGamesWidget()
{
    ui->tableWidget_recordingGames->clearContents();

    while(ui->tableWidget_recordingGames->rowCount() > 0){
        ui->tableWidget_recordingGames->removeRow(0);
    }

    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();

    for(int i = 0; i < recording.size(); i++){
        ui->tableWidget_recordingGames->insertRow(i);

        if(recording.at(i).size() >= 3){
            ui->tableWidget_recordingGames->setItem(i, 0, new QTableWidgetItem(recording.at(i).at(0)));
            ui->tableWidget_recordingGames->setItem(i, 1, new QTableWidgetItem(recording.at(i).at(1)));

            QDateTime dateTime = QDateTime::fromString(recording.at(i).at(2));

            ui->tableWidget_recordingGames->setItem(i, 2, new QTableWidgetItem(dateTime.toString(Qt::SystemLocaleLongDate)));

            qint64 timeProgress = currentTime - dateTime.toMSecsSinceEpoch();
            timeProgress /= 1000;
            timeProgress /= 60;
            int value = 0;

            if(timeProgress >= 30){
                value = 90;
            }
            else{
                value = int(timeProgress * 3);
            }

            QProgressBar* bar = new QProgressBar();
            bar->setValue(value);

            ui->tableWidget_recordingGames->setCellWidget(i, 3, bar);
        }
    }
}

void MainWindow::slot_setLanguage()
{
    if(orsettings->value("Language").toString() == ui->comboBox_language->currentText()){
        return;
    }

    orsettings->setValue("Language", ui->comboBox_language->currentText());

    if(translator.load(QString(":/translation/legendsreplay_") + orsettings->value("Language").toString().toLower())){
        qApp->installTranslator(&translator);
    }
}

void MainWindow::changeEvent(QEvent* event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        ui->comboBox_language->setCurrentText(orsettings->value("Language").toString());
    }

    QMainWindow::changeEvent(event);
}

void MainWindow::slot_checkandrepair(){
    RepairToolDialog *newRepairToolDialog = new RepairToolDialog(this);
    newRepairToolDialog->show();
}

void MainWindow::slot_directoryChanged(QString path){
    Q_UNUSED(path);

    slot_refresh_recordedGames();
}

void MainWindow::slot_customMenuTriggeredSystemTrayIcon(QAction *action){
    if(action->text() == tr("Exit")){
        qApp->exit(0);
    }
}
