/* ****************************************************************************
 *
 * Copyright 2016-2017 William Bonnaventure
 *
 * This file is part of LegendsReplay.
 *
 * LegendsReplay is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * LegendsReplay is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LegendsReplay.  If not, see <http://www.gnu.org/licenses/>.
 *
 * ****************************************************************************
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

#define GLOBAL_VERSION "1.5.1"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("LegendsReplay " + QString(GLOBAL_VERSION));
    setWindowIcon(QIcon(":/icons/logo.png"));

    log(QString("LegendsReplay " + QString(GLOBAL_VERSION)));

    // Adding the official local servers
    QFile localserversfile(":/data/LegendsReplayServers.txt");

    if (localserversfile.open(QIODevice::ReadOnly)) {
        QTextStream in(&localserversfile);

        while (!in.atEnd()) {
            QString line = in.readLine();
            if (!line.isEmpty()) {
                lrservers.append(line);
                ui->tableWidget_replayservers->insertRow(ui->tableWidget_replayservers->rowCount());
                ui->tableWidget_replayservers->setItem(ui->tableWidget_replayservers->rowCount()-1, 0, new QTableWidgetItem(line));
            }
        }

        localserversfile.close();
    } else {
        log("[ERROR] Unable to open internal servers file");
        lrservers.append("http://legendsreplay.16mb.com/legendsreplay.php");
        ui->tableWidget_replayservers->insertRow(ui->tableWidget_replayservers->rowCount());
        ui->tableWidget_replayservers->setItem(ui->tableWidget_replayservers->rowCount()-1, 0, new QTableWidgetItem(lrservers.last()));
    }

    if (lrservers.isEmpty()) {
        // Critical
        QMessageBox::critical(this, tr("LegendsReplay"), tr("Unable to find a Legends Replay server"));
        close();
        return;
    }

    m_currentLegendsReplayServer = lrservers.first();

    lrsettings = new QSettings("LegendsReplay", "Local", this);

    if (!lrsettings->value("SummonerName").toString().isEmpty()) {
        m_summonername = lrsettings->value("SummonerName").toString();
        ui->lineEdit_summonername->setText(m_summonername);
    } else {
        QMessageBox::information(this, tr("LegendsReplay"), tr("Please set your summoner name and then keep the software open to record your games."));
        log("[WARN] Please set your summoner name and then keep the software open to record your games.");

        ui->tabWidget->setCurrentIndex(6);
    }

    if (!lrsettings->value("SummonerId").toString().isEmpty()) {
        m_summonerid = lrsettings->value("SummonerId").toString();
        ui->lineEdit_summonerid->setText(m_summonerid);
    }

    if (!lrsettings->value("SummonerServer").toString().isEmpty()) {
        m_summonerServerRegion = lrsettings->value("SummonerServer").toString();
        ui->comboBox_summonerserver->setCurrentText(m_summonerServerRegion);
    }

    if (!lrsettings->value("Language").toString().isEmpty() && translator.load(QString(":/translation/legendsreplay_") + lrsettings->value("Language").toString().toLower())) {
        qApp->installTranslator(&translator);
    }

    if (!lrsettings->value("LoLDirectory").toString().isEmpty()) {
        loldirectory = lrsettings->value("LoLDirectory").toString();
    } else {
        QSettings lolsettings("Riot Games", "RADS");
        QString rootfolder = lolsettings.value("LocalRootFolder").toString();

        if (rootfolder.isEmpty()) {
            loldirectory = "C:/Program Files/Riot Games/League of Legends/RADS";
        } else {
            loldirectory = rootfolder;
        }
    }

    ui->lineEdit_lolfolder->setText(loldirectory);

    QString docfolder;
    if (lrsettings->value("ReplayDirectory").toString().isEmpty()) {
        QStringList folders = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
        if (folders.isEmpty()) {
            log("[CRITIC] no documents location found on the system");
            close();
            return;
        }
        docfolder = folders.first();
    } else {
        docfolder = lrsettings->value("ReplayDirectory").toString();
    }

    if (!QDir(docfolder + "/LegendsReplay").exists()) {
        QDir().mkpath(docfolder + "/LegendsReplay");
    }

    replaydirectory = docfolder + "/LegendsReplay";

    ui->lineEdit_replaysFolder->setText(replaydirectory);

    replaying = false;
    playing = false;

    replay = nullptr;

    serverChunkCount = 0;
    serverKeyframeCount = 1;

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slot_refreshPlayingStatus()));
    m_timer->start(60000);

    // Add servers

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
    // PBE Spectator server doesn't exist anymore
    servers.append(Server("PBE", "PBE", "PBE1", QString(), 0));

    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(slot_changedTab(int)));
    connect(ui->pushButton_2, SIGNAL(released()), this, SLOT(slot_featuredRefresh()));
    connect(ui->listWidget_featured, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(slot_doubleclick_featured(QListWidgetItem*)));
    connect(ui->toolButton, SIGNAL(released()), this, SLOT(slot_setdirectory()));
    connect(ui->toolButton_2, SIGNAL(released()), this, SLOT(slot_setreplaydirectory()));
    connect(ui->pushButton_add_replayserver, SIGNAL(released()), this, SLOT(slot_replayserversAdd()));
    connect(ui->pushButton_summonersave, SIGNAL(released()), this, SLOT(slot_summonerinfos_save()));
    connect(ui->pushButton_searchsummoner, SIGNAL(released()), this, SLOT(slot_searchsummoner()));
    connect(ui->pushButton_searchsummoner_spectate, SIGNAL(released()), this, SLOT(slot_click_searchsummoner_spectate()));
    connect(ui->pushButton_searchsummoner_record, SIGNAL(released()), this, SLOT(slot_click_searchsummoner_record()));
    connect(ui->tableWidget_replayservers, SIGNAL(itemSelectionChanged()), this, SLOT(slot_click_replayservers()));
    connect(ui->actionRepair_tool, SIGNAL(triggered()), this, SLOT(slot_checkandrepair()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionReport_an_issue, SIGNAL(triggered()), this, SLOT(slot_reportAnIssue()));
    connect(ui->actionAbout_LegendsReplay, SIGNAL(triggered()), this, SLOT(slot_aboutLegendsReplay()));
    connect(ui->actionLaunch_rofl_file, SIGNAL(triggered()), this, SLOT(slot_launch_rofl_file()));

    networkManager_replayServers = new QNetworkAccessManager(this);
    connect(networkManager_replayServers, SIGNAL(finished(QNetworkReply*)), this, SLOT(slot_networkResult_replayServers(QNetworkReply*)));
    networkManager_replayServers->get(QNetworkRequest(QUrl(m_currentLegendsReplayServer)));

    networkManager_status = new QNetworkAccessManager(this);
    connect(networkManager_status, SIGNAL(finished(QNetworkReply*)), this, SLOT(slot_networkResult_status(QNetworkReply*)));
    connect(ui->pushButton_statusRefresh, SIGNAL(pressed()), this, SLOT(slot_statusRefresh()));
    connect(this, SIGNAL(signal_refreshStatusServers()), this, SLOT(slot_statusRefresh()), Qt::QueuedConnection);

    emit signal_refreshStatusServers();

    connect(this, SIGNAL(signal_refresh_recordedGames()), this, SLOT(slot_refresh_recordedGames()), Qt::QueuedConnection);

    networkManager_featured = new QNetworkAccessManager(this);
    connect(networkManager_featured, SIGNAL(finished(QNetworkReply*)), this, SLOT(slot_networkResult_featured(QNetworkReply*)));
    connect(ui->tableWidget_recordedgames, SIGNAL(itemSelectionChanged()), this, SLOT(slot_click_allgames()));
    connect(ui->tableWidget_recordedgames, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slot_customcontextmenu()));
    connect(ui->tableWidget_yourgames, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slot_customcontextmenu()));
    connect(ui->listWidget_featured, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slot_customcontextmenu()));
    connect(ui->tableWidget_recordingGames, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slot_customcontextmenu()));

    httpserver = new QHttpServer(this);
    connect(ui->tableWidget_recordedgames, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(slot_doubleclick_savedgames(int)));
    connect(ui->tableWidget_yourgames, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(slot_doubleclick_mygames(int)));

    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(slot_open_replay()));
    connect(ui->actionAdvanced_Recorder, SIGNAL(triggered()), this, SLOT(slot_openAdvancedRecorder()));
    connect(ui->pushButton_saveLanguage, SIGNAL(released()), this, SLOT(slot_setLanguage()));

    m_directory_watcher = new QFileSystemWatcher(this);
    m_directory_watcher->addPath(replaydirectory);
    connect(m_directory_watcher, SIGNAL(directoryChanged(QString)), this, SLOT(slot_refresh_recordedGames()));

    emit signal_refresh_recordedGames();

    custommenu = new QMenu(tr("Options"), this);

    //Create and show system tray icon if available
    systemtrayavailable = QSystemTrayIcon::isSystemTrayAvailable();

    if (systemtrayavailable) {
        systemtrayicon = new QSystemTrayIcon(this);
        systemtrayicon->setIcon(QIcon(":/icons/logo.png"));

        systemtraymenu = new QMenu(this);
        systemtraymenu->addAction(QIcon(":/icons/exit.png"), tr("&Exit"), this, SLOT(close()));

        systemtrayicon->setContextMenu(systemtraymenu);
        systemtrayicon->show();

        connect(systemtrayicon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(systemtrayiconActivated(QSystemTrayIcon::ActivationReason)));
        connect(systemtrayicon, SIGNAL(messageClicked()), this, SLOT(slot_messageclicked()));
    }

    connect(this, SIGNAL(signal_checkSoftwareVersion()), this, SLOT(slot_checkSoftwareVersion()), Qt::QueuedConnection);

    emit signal_checkSoftwareVersion();
}

MainWindow::~MainWindow()
{
    //Force stopping all recording threads
    QPair<QString, QThread*> current_record;
    foreach(current_record, recording){
        current_record.second->exit(0);
    }

    //Hide system tray icon if available
    if (systemtrayavailable) {
        systemtrayicon->hide();
    }

    delete replay;
    delete ui;
}

void MainWindow::slot_checkSoftwareVersion()
{
    QJsonDocument updatejson = getJsonFromUrl("https://aztorius.github.io/legendsreplay/version.json");

    if (!updatejson.isEmpty() && updatejson.object().value("version").toString() != QString(GLOBAL_VERSION)) {
        showmessage(tr("New version ") + updatejson.object().value("version").toString() + " available !");
        ui->statusBar->showMessage(QTime::currentTime().toString() + " | " + "New version " + updatejson.object().value("version").toString() + tr(" available !"));
        ui->textBrowser->append(QTime::currentTime().toString() + " | <a href='https://aztorius.github.io/legendsreplay/'>New version " + updatejson.object().value("version").toString() + " available !</a>");
    } else if (updatejson.isEmpty()) {
        ui->textBrowser->append(QTime::currentTime().toString() + " | Unable to retrieve the last version info online");
    }
}

void MainWindow::log(QString s)
{
    ui->statusBar->showMessage(QTime::currentTime().toString() + " | " + s);
    ui->textBrowser->append(QTime::currentTime().toString() + " | " + s);
}

void MainWindow::setArgs(int argc, char *argv[])
{
    if (argc > 1) {
        if (std::string(argv[1]) == "help" || std::string(argv[1]) == "-h") {
            qInfo() << "LegendsReplay -- Help Menu" << endl;
            qInfo() << "help : show this menu" << endl;
            qInfo() << "--silent : hide the main window of LegendsReplay at launch" << endl;
            qInfo() << "record [ServerRegion] [ServerAddress:Port] [GameId] [EncryptionKey] [ForceCompleteDownload]" << endl;
            qInfo() << "    : record the specified game in the spectator server given" << endl;
        } else if (std::string(argv[1]) == "--silent") {
            this->showMinimized();

            return;
        } else if(std::string(argv[1]) == "record" && argc > 6) { //command : record serverregion serveraddress gameid encryptionkey forceCompleteDownload
            slot_customGameRecord(QString(argv[3]), QString(argv[2]), QString(argv[4]), QString(argv[5]), QString(argv[6]) == "true", false, false);

            return;
        }

        Replay replay(argv[1]);
        if (!replay.isEmpty()) {
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

    for(int i = 0; i < servers.size() - 1; i++) {
        networkManager_status->get(QNetworkRequest(QUrl("http://" + servers.at(i).getUrl() + "/observer-mode/rest/consumer/version")));
    }
}

void MainWindow::slot_doubleclick_featured(QListWidgetItem *item)
{
    GameInfosWidget *widget = dynamic_cast<GameInfosWidget*>(ui->listWidget_featured->itemWidget(item));

    QString platformid = widget->getPlatformId();
    QString key = widget->getEncryptionkey();
    QString gameid = widget->getGameId();

    if (game_ended(platformid, gameid)) {
        log("Game " + platformid + "/" + gameid + " has already ended");
        return;
    }

    slot_refreshPlayingStatus();

    if (playing) {
        log("[WARN] Replay aborted. LoL is currently running.");
        return;
    }

    replaying = true;

    lol_launch(platformid, key, gameid);
}

void MainWindow::lol_launch(QString platformid, QString key, QString matchid, bool local)
{
    if (platformid.isEmpty() || key.isEmpty() || matchid.isEmpty()) {
        log("[ERROR] Invalid game parameters.");
        return;
    }

    QString path;

    QDir qd;

    if (platformid == "PBE1") {
        qd.setPath(lolpbedirectory + "/solutions/lol_game_client_sln/releases/");
    } else {
        qd.setPath(loldirectory + "/solutions/lol_game_client_sln/releases/");
    }

    qd.setFilter(QDir::Dirs);
    qd.setSorting(QDir::Name | QDir::Reversed);
    QFileInfoList list = qd.entryInfoList();

    if (list.isEmpty()) {
        QMessageBox::information(this, tr("LegendsReplay"), tr("Invalid League of Legends (or PBE) directory.\nPlease set a valid one."));
        log("[WARN] Invalid League of Legends (or PBE) directory. No releases folder found.");
        return;
    }

    if (platformid == "PBE1") {
        path = lolpbedirectory + "/solutions/lol_game_client_sln/releases/" + list.first().fileName() + "/deploy/";
    } else {
        path = loldirectory + "/solutions/lol_game_client_sln/releases/" + list.first().fileName() + "/deploy/";
    }

    if (!check_path(path)) {
        QMessageBox::information(this, tr("LegendsReplay"), tr("Invalid League of Legends (or PBE) directory.\nPlease set a valid one."));
        log("[WARN] Invalid League of Legends (or PBE) directory. Invalid path.");
        return;
    }

    QString address;

    if (local) {
        address = "127.0.0.1:8088";

#ifdef Q_OS_WIN32

        log("\"" + path + "League of Legends.exe\" \"8394\" \"LoLLauncher.exe\" \"\" \"spectator " + address + " " + key + " " + matchid + " " + platformid + "\"");

        if (!QProcess::startDetached("\"" + path + "League of Legends.exe\"", QStringList() << "\"8394\"" << "\"LoLLauncher.exe\"" << "\"\"" << ("spectator " + address + " " + key + " " + matchid + " " + platformid), path)) {
            log("Unable to launch League of Legends");
            return;
        }

#elif defined(Q_OS_OSX)

        log("\"" + path + "League of Legends\" \"8394\" \"LoLLauncher\" \"\" \"spectator " + address + " " + key + " " + matchid + " " + platformid + "\"");

        if (!QProcess::startDetached("\"" + path + "League of Legends\"", QStringList() << "\"8394\"" << "\"LoLLauncher\"" << "\"\"" << ("spectator " + address + " " + key + " " + matchid + " " + platformid), path)) {
            log("Unable to launch League of Legends");
            return;
        }

#elif defined(Q_OS_UNIX)

        log("LoL Launch isn't available for Linux yet");

#endif

    } else {
        address = getServerByPlatformId(platformid).getUrl();

        if (address.isEmpty()) {
            //Server address not found
            log("[ERROR] Server address not found");
            return;
        }

#ifdef Q_OS_WIN32

        log("\"" + path + "League of Legends.exe\" \"8394\" \"LoLLauncher.exe\" \"\" \"spectator " + address + " " + key + " " + matchid + " " + platformid + "\"");

        if (!QProcess::startDetached("\"" + path + "League of Legends.exe\"", QStringList() << "\"8394\"" << "\"LoLLauncher.exe\"" << "\"\"" << ("spectator " + address + " " + key + " " + matchid + " " + platformid), path)) {
            log("Unable to launch League of Legends");
            return;
        }

#elif defined(Q_OS_OSX)

        log("\"" + path + "League of Legends\" \"8394\" \"LoLLauncher\" \"\" \"spectator " + address + " " + key + " " + matchid + " " + platformid + "\"");

        if (!QProcess::startDetached("\"" + path + "League of Legends\"", QStringList() << "\"8394\"" << "\"LoLLauncher\"" << "\"\"" << ("spectator " + address + " " + key + " " + matchid + " " + platformid), path)) {
            log("Unable to launch League of Legends");
            return;
        }

#elif defined(Q_OS_UNIX)

        log("LoL Launch isn't available for Linux yet");

#endif

    }

    this->showMinimized();
}

void MainWindow::slot_networkResult_status(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        QString host = reply->request().url().host();
        Server server = getServerByDomain(host);

        if (server.getPlatformId().isEmpty()) {
            log("[ERROR] Unknown platform id");
            return;
        }

        int row = servers.indexOf(server);

        log("[ERROR] Status of " + server.getPlatformId() + ": " + reply->errorString());

        if (reply->url().path() == "/observer-mode/rest/consumer/version") {
            ui->tableWidget_status->setItem(row, ui->tableWidget_status->columnCount()-1, new QTableWidgetItem(tr("offline")));
            ui->tableWidget_status->item(row, ui->tableWidget_status->columnCount()-1)->setBackgroundColor(Qt::red);
            ui->tableWidget_status->item(row, ui->tableWidget_status->columnCount()-1)->setTextColor(Qt::white);
        } else {
            for(int i = 0; i < 4; i++) {
                ui->tableWidget_status->setItem(row, i, new QTableWidgetItem(tr("offline")));
                ui->tableWidget_status->item(row, i)->setBackgroundColor(Qt::red);
                ui->tableWidget_status->item(row, i)->setTextColor(Qt::white);
            }
        }
        return;
    }

    QString data = (QString) reply->readAll();

    QJsonDocument jsonResponse = QJsonDocument::fromJson(data.toUtf8());

    if (jsonResponse.isEmpty()) {
        if (reply->url().path() == "/observer-mode/rest/consumer/version") {
            QString host = reply->request().url().host();
            Server server = getServerByDomain(host);

            if (server.isEmpty()) {
                log("[ERROR] Unknown platform id");
                return;
            }

            int row = servers.indexOf(server);

            if (data.isEmpty()) {
                ui->tableWidget_status->setItem(row, ui->tableWidget_status->columnCount()-1, new QTableWidgetItem(tr("offline")));
                ui->tableWidget_status->item(row, ui->tableWidget_status->columnCount()-1)->setBackgroundColor(Qt::red);
                ui->tableWidget_status->item(row, ui->tableWidget_status->columnCount()-1)->setTextColor(Qt::white);
                return;
            }

            ui->tableWidget_status->setItem(row, ui->tableWidget_status->columnCount()-1, new QTableWidgetItem(tr("online")));
            ui->tableWidget_status->item(row, ui->tableWidget_status->columnCount()-1)->setBackgroundColor(QColor(0,160,0));
            ui->tableWidget_status->item(row, ui->tableWidget_status->columnCount()-1)->setTextColor(Qt::white);

            return;
        } else {
            log("[ERROR] Retrieving server status");
            return;
        }
    }

    QJsonObject jsonObject = jsonResponse.object();

    json_status.append(jsonObject);

    QJsonArray services = jsonObject.value("services").toArray();

    log("Server status : " + jsonObject.value("name").toString());

    for(int i = 0; i < servers.size(); i++) {
        if (jsonObject.value("name").toString() == servers.at(i).getName()) {
            for(int j = 0; j < qMin(services.size(), 4); j++) {
                QJsonArray incidentsArray = services[j].toObject().value("incidents").toArray();

                if (services[j].toObject().value("status").toString() == "offline") { //Service offline
                    ui->tableWidget_status->setItem(i, j, new QTableWidgetItem(tr("offline")));
                    ui->tableWidget_status->item(i, j)->setBackgroundColor(Qt::red);
                    ui->tableWidget_status->item(i, j)->setTextColor(Qt::white);

                    if (!incidentsArray.isEmpty()) {
                        QString incidents = incidentsArray.first().toObject().value("updates").toArray().first().toObject().value("content").toString();

                        for(int k = 1; k < incidentsArray.size(); k++) {
                            if (!incidentsArray.at(k).toObject().value("updates").toArray().first().toObject().value("content").toString().isEmpty()) {
                                if (!incidents.isEmpty()) {
                                    incidents.append("\n");
                                }

                                incidents.append(incidentsArray.at(k).toObject().value("updates").toArray().first().toObject().value("content").toString());
                            }
                        }

                        ui->tableWidget_status->item(i, j)->setToolTip(incidents);
                    }
                } else if (!incidentsArray.isEmpty()) { //Service online with incidents
                    QString incidents = incidentsArray.first().toObject().value("updates").toArray().first().toObject().value("content").toString();

                    for(int k = 1; k < incidentsArray.size(); k++) {
                        if (!incidentsArray.at(k).toObject().value("updates").toArray().first().toObject().value("content").toString().isEmpty()) {
                            if (!incidents.isEmpty()) {
                                incidents.append("\n");
                            }

                            incidents.append(incidentsArray.at(k).toObject().value("updates").toArray().first().toObject().value("content").toString());
                        }
                    }

                    ui->tableWidget_status->setItem(i, j, new QTableWidgetItem(QString::number(incidentsArray.size()) + tr(" incident(s)")));
                    ui->tableWidget_status->item(i, j)->setToolTip(incidents);
                    ui->tableWidget_status->item(i, j)->setTextColor(Qt::white);
                    ui->tableWidget_status->item(i, j)->setBackgroundColor(QColor(255,165,0));
                } else if (services[j].toObject().value("status").toString() == "online") { //Service online
                    ui->tableWidget_status->setItem(i, j, new QTableWidgetItem(tr("online")));
                    ui->tableWidget_status->item(i, j)->setTextColor(Qt::white);
                    ui->tableWidget_status->item(i, j)->setBackgroundColor(QColor(0,160,0));
                }
            }
            break;
        }
    }
}

void MainWindow::slot_networkResult_replayServers(QNetworkReply *reply){
    if (reply->error() != QNetworkReply::NoError) {
        log("[WARN] Replay Server error : " + reply->url().toString() + " > " + reply->errorString());

        ui->tableWidget_replayservers->item(lrservers.indexOf(reply->url().toString()), 0)->setBackgroundColor(Qt::red);
        ui->tableWidget_replayservers->item(lrservers.indexOf(reply->url().toString()), 0)->setTextColor(Qt::white);

        int i = lrservers.indexOf(reply->url().toString()) + 1;
        if (i < lrservers.size()) {
            networkManager_replayServers->get(QNetworkRequest(QUrl(lrservers.at(i))));
        } else {
            //No online Legends Replay servers found
        }

        return;
    }

    ui->tableWidget_replayservers->item(lrservers.indexOf(reply->url().toString()), 0)->setBackgroundColor(QColor(0, 160, 0));
    ui->tableWidget_replayservers->item(lrservers.indexOf(reply->url().toString()), 0)->setTextColor(Qt::white);

    m_currentLegendsReplayServer = reply->url().toString();

    log("Legends Replay switch to server " + m_currentLegendsReplayServer);
}

void MainWindow::slot_networkResult_featured(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        log("[ERROR] Network featured games : " + reply->errorString());
        return;
    }

    QString data = (QString) reply->readAll();

    QJsonDocument jsonResponse = QJsonDocument::fromJson(data.toUtf8());

    if (jsonResponse.isEmpty()) {
        log("[ERROR] Featured games empty reponse");
        return;
    }

    QJsonObject jsonObject = jsonResponse.object();

    json_featured.append(jsonObject);

    QJsonArray gamelist = jsonObject.value("gameList").toArray();

    if (gamelist.isEmpty()) {
        log("[WARN] Featured games empty response");
        return;
    }

    log(gamelist.first().toObject().value("platformId").toString() + " : Featured games infos");

    for(int i = 0; i < gamelist.size(); i++) {
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
}

QPixmap MainWindow::getImg(int id)
{
    int finalid = 0;

    QFileInfo info(":/img/" + QString::number(id) + ".png");
    if (info.exists() && info.isFile()) {
        finalid = id;
    } else {
        log("Icon not yet available for champion id " + QString::number(id));
    }

    QPixmap img(":/img/" + QString::number(finalid) + ".png");
    return img.scaled(60, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void MainWindow::slot_setdirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open RADS Directory"), loldirectory, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir.isEmpty()) {
        return;
    }

    loldirectory = dir;
    lrsettings->setValue("LoLDirectory", loldirectory);
    ui->lineEdit_lolfolder->setText(dir);
}

void MainWindow::slot_setreplaydirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), replaydirectory, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir.isEmpty()) {
        return;
    }

    replaydirectory = dir;
    lrsettings->setValue("ReplayDirectory", replaydirectory);
    ui->lineEdit_replaysFolder->setText(dir);
}

void MainWindow::slot_featuredLaunch()
{
    if (ui->listWidget_featured->selectedItems().isEmpty()) {
        return;
    }

    slot_refreshPlayingStatus();

    if (playing) {
        log("[WARN] Replay aborted. LoL is currently running.");
        return;
    }

    GameInfosWidget *widget = dynamic_cast<GameInfosWidget*>(ui->listWidget_featured->itemWidget(ui->listWidget_featured->selectedItems().first()));

    QString platformid = widget->getPlatformId();
    QString key = widget->getEncryptionkey();
    QString gameid = widget->getGameId();

    if (game_ended(platformid, gameid)) {
        log("Game " + platformid + "/" + gameid + " has already ended");
        return;
    }

    lol_launch(platformid, key, gameid);

    replaying = true;
}

bool MainWindow::check_path(QString path)
{
    return(QFileInfo::exists(path));
}

void MainWindow::slot_changedTab(int index)
{
    if (index == 2) {
        slot_featuredRefresh();
    }
}

bool MainWindow::game_ended(QString region, QString gameid)
{
    //Get server URL
    QString serveraddress = getServerByRegion(region).getUrl();

    if (serveraddress.isEmpty()) {
        return false;
    }

    QJsonDocument jsonResponse = getJsonFromUrl(QString("http://" + serveraddress + "/observer-mode/rest/consumer/getGameMetaData/" + region + "/" + gameid + "/token"));

    if (jsonResponse.isEmpty()) {
        return false;
    }

    QJsonObject jsonObject = jsonResponse.object();

    if (jsonObject.value("gameEnded").toBool()) {
        log("GAME : " + region + "/" + gameid + " has already finished. Aborting spectator mode.");
        return true;
    } else {
        log("GAME : " + region + "/" + gameid + " is in progress. Launching spectator mode.");
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

    if (reply->error() != QNetworkReply::NoError) {
        QJsonDocument jsonEmpty;
        log("[ERROR] Get JSON : " + reply->errorString());
        return jsonEmpty;
    }

    QString data = (QString) reply->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(data.toUtf8());
    reply->deleteLater();

    return jsonResponse;
}

Server MainWindow::getServerByPlatformId(QString platformid)
{
    for(int i = 0; i < servers.size(); i++) {
        if (servers.at(i).getPlatformId() == platformid) {
            return servers.at(i);
        }
    }

    return Server();
}

Server MainWindow::getServerByRegion(QString region){
    for(int i = 0; i < servers.size(); i++){
        if (servers.at(i).getRegion() == region) {
            return servers.at(i);
        }
    }

    return Server();
}

Server MainWindow::getServerByDomain(QString domain){
    for(int i = 0; i < servers.size(); i++){
        if (servers.at(i).getDomain() == domain) {
            return servers.at(i);
        }
    }

    return Server();
}

void MainWindow::slot_featuredRecord()
{
    if (ui->listWidget_featured->selectedItems().isEmpty()) {
        return;
    }

    GameInfosWidget* widget(dynamic_cast<GameInfosWidget*>(ui->listWidget_featured->itemWidget(ui->listWidget_featured->currentItem())));

    QString platformid = widget->getPlatformId();
    QString gameid = widget->getGameId();

    //Get server address
    QString serveraddress = getServerByPlatformId(platformid).getUrl();

    if (serveraddress.isEmpty()) {
        log("Server address not found");
        return;
    }

    if (recording.contains(QPair<QString, QString>(platformid, gameid))) {
        log("Game is already recording");
        return;
    }

    QJsonDocument gameinfo;

    for(int i = 0; i < json_featured.size(); i++) {
        QJsonArray gamelist = json_featured.at(i).value("gameList").toArray();
        for(int k = 0; k < gamelist.size(); k++) {
            if (gamelist.at(k).toObject().value("platformId").toString() == platformid) {
                 if (QString::number(gamelist.at(k).toObject().value("gameId").toVariant().toULongLong()) == gameid) {
                     gameinfo = QJsonDocument(gamelist.at(k).toObject());
                     break;
                 }
            }
        }
    }

    QString dateTime;

    if (gameinfo.isEmpty()) {
        dateTime = QDateTime::currentDateTime().toString();
    } else {
        dateTime = QDateTime::fromMSecsSinceEpoch(gameinfo.object().value("gameStartTime").toVariant().toLongLong()).toString();
    }

    refreshRecordingGamesWidget();

    QThread *recorderThread = new QThread;
    Recorder *recorder = new Recorder(widget->getPlatformId(), serveraddress, gameid, widget->getEncryptionkey(), gameinfo, replaydirectory);
    recorder->moveToThread(recorderThread);
    connect(recorderThread, SIGNAL(started()), recorder, SLOT(launch()));
    connect(recorder, SIGNAL(finished()), recorderThread, SLOT(quit()));
    connect(recorder, SIGNAL(finished()), recorder, SLOT(deleteLater()));
    connect(recorderThread, SIGNAL(finished()), recorderThread, SLOT(deleteLater()));
    connect(recorder, SIGNAL(end(QString,QString)), this, SLOT(slot_endRecording(QString,QString)));
    connect(recorder, SIGNAL(toLog(QString)), this, SLOT(log(QString)));
    connect(recorder, SIGNAL(toShowmessage(QString)), this, SLOT(showmessage(QString)));

    recorderThread->start();

    recording.insert(QPair<QString, QString>(platformid, gameid), QPair<QString, QThread*>(dateTime, recorderThread));
}

void MainWindow::slot_endRecording(QString platformid, QString gameid)
{
    if (recording.contains(QPair<QString, QString>(platformid, gameid))) {
        recording.remove(QPair<QString, QString>(platformid, gameid));
    }

    refreshRecordingGamesWidget();
}

QByteArray MainWindow::getFileFromUrl(QString url)
{
    QNetworkAccessManager local_networkResult;
    QNetworkReply *reply = local_networkResult.get(QNetworkRequest(QUrl(url)));

    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
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
    if (ui->tableWidget_recordedgames->selectedItems().isEmpty()) {
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

    if (game.getGameinfos().isEmpty()) {
        log("[ERROR] No game infos found. Aborting.");
        return;
    }

    QJsonArray array = game.getGameinfos().object().value("participants").toArray();
    QList<int> leftids;
    QList<QString> leftnames;
    QList<int> rightids;
    QList<QString> rightnames;

    for(int i = 0; i < array.size(); i++) {
        if (array.at(i).toObject().value("teamId").toInt() == 200) {
            rightids.append(array.at(i).toObject().value("championId").toInt());
            rightnames.append(array.at(i).toObject().value("summonerName").toString());
        } else {
            leftids.append(array.at(i).toObject().value("championId").toInt());
            leftnames.append(array.at(i).toObject().value("summonerName").toString());
        }
    }

    if (leftids.size() >= 1 && leftnames.size() >= 1) {
        ui->label_sum1->setAlignment(Qt::AlignCenter);
        ui->label_sum1->setPixmap(getImg(leftids.first()));

        ui->label_sum16->setText(leftnames.first());

        if (leftids.size() >= 2 && leftnames.size() >= 2) {
            ui->label_sum2->setAlignment(Qt::AlignCenter);
            ui->label_sum2->setPixmap(getImg(leftids.at(1)));

            ui->label_sum27->setText(leftnames.at(1));

            if (leftids.size() >= 3 && leftnames.size() >= 3) {
                ui->label_sum3->setAlignment(Qt::AlignCenter);
                ui->label_sum3->setPixmap(getImg(leftids.at(2)));

                ui->label_sum38->setText(leftnames.at(2));

                if (leftids.size() >= 4 && leftnames.size() >= 4) {
                    ui->label_sum4->setAlignment(Qt::AlignCenter);
                    ui->label_sum4->setPixmap(getImg(leftids.at(3)));

                    ui->label_sum49->setText(leftnames.at(3));

                    if (leftids.size() >= 5 && leftnames.size() >= 5) {
                        ui->label_sum5->setAlignment(Qt::AlignCenter);
                        ui->label_sum5->setPixmap(getImg(leftids.at(4)));

                        ui->label_sum510->setText(leftnames.at(4));
                    }
                }
            }
        }
    }

    if (rightids.size() >= 1 && rightnames.size() >= 1) {
        ui->label_sum6->setAlignment(Qt::AlignCenter);
        ui->label_sum6->setPixmap(getImg(rightids.first()));

        ui->label_sum16->setText(ui->label_sum16->text() + " / " + rightnames.first());

        if (rightids.size() >= 2 && rightnames.size() >= 2) {
            ui->label_sum7->setAlignment(Qt::AlignCenter);
            ui->label_sum7->setPixmap(getImg(rightids.at(1)));

            ui->label_sum27->setText(ui->label_sum27->text() + " / " + rightnames.at(1));

            if (rightids.size() >= 3 && rightnames.size() >= 3) {
                ui->label_sum8->setAlignment(Qt::AlignCenter);
                ui->label_sum8->setPixmap(getImg(rightids.at(2)));

                ui->label_sum38->setText(ui->label_sum38->text() + " / " + rightnames.at(2));

                if (rightids.size() >= 4 && rightnames.size() >= 4) {
                    ui->label_sum9->setAlignment(Qt::AlignCenter);
                    ui->label_sum9->setPixmap(getImg(rightids.at(3)));

                    ui->label_sum49->setText(ui->label_sum49->text() + " / " + rightnames.at(3));

                    if (rightids.size() >= 5 && rightnames.size() >= 5) {
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

    while(ui->tableWidget_recordedgames->rowCount() > 0) {
        ui->tableWidget_recordedgames->removeRow(0);
    }

    ui->tableWidget_yourgames->clearContents();

    while(ui->tableWidget_yourgames->rowCount() > 0) {
        ui->tableWidget_yourgames->removeRow(0);
    }

    for(int i = 0; i < replayslist.size(); i++) {

        QFileInfo fileinfo = replayslist.at(i);

        if (fileinfo.suffix() == "lor") {
            ui->tableWidget_recordedgames->insertRow(ui->tableWidget_recordedgames->rowCount());

            Replay game(fileinfo.filePath(), true);

            QDateTime datetime;
            datetime.setMSecsSinceEpoch(quint64(game.getGameinfos().object().value("gameStartTime").toVariant().toLongLong()));

            ui->tableWidget_recordedgames->setItem(ui->tableWidget_recordedgames->rowCount()-1, 0, new QTableWidgetItem(game.getPlatformId()));
            ui->tableWidget_recordedgames->setItem(ui->tableWidget_recordedgames->rowCount()-1, 1, new QTableWidgetItem(game.getGameId()));
            ui->tableWidget_recordedgames->setItem(ui->tableWidget_recordedgames->rowCount()-1, 2, new QTableWidgetItem(datetime.date().toString(Qt::DefaultLocaleShortDate)));
            ui->tableWidget_recordedgames->setItem(ui->tableWidget_recordedgames->rowCount()-1, 3, new QTableWidgetItem(fileinfo.fileName()));

            recordedgames_filename.append(fileinfo.fileName());

            //Get platformId of the user
            QString local_platformid = getServerByRegion(m_summonerServerRegion).getPlatformId();

            if (!local_platformid.isEmpty() && game.getPlatformId() == local_platformid) {
                if (!m_summonerid.isEmpty() && !game.getGameinfos().isEmpty() && !game.getGameinfos().object().value("participants").toArray().isEmpty()) {
                    for(int j = 0; j < game.getGameinfos().object().value("participants").toArray().size(); j++) {
                        if (game.getGameinfos().object().value("participants").toArray().at(j).toObject().value("summonerName").toString() == m_summonername) {
                            ui->tableWidget_yourgames->insertRow(ui->tableWidget_yourgames->rowCount());

                            QLabel* label_img = new QLabel;
                            label_img->setAlignment(Qt::AlignCenter);
                            label_img->setPixmap(getImg(game.getGameinfos().object().value("participants").toArray().at(j).toObject().value("championId").toInt()));

                            ui->tableWidget_yourgames->setCellWidget(ui->tableWidget_yourgames->rowCount()-1, 0, label_img);

                            QTableWidgetItem * item_stats = new QTableWidgetItem(tr("Soon"));
                            QTableWidgetItem * item_dateTime = new QTableWidgetItem(datetime.toString(Qt::DefaultLocaleShortDate));
                            QTableWidgetItem * item_platformId = new QTableWidgetItem(game.getPlatformId());
                            QTableWidgetItem * item_fileName = new QTableWidgetItem(fileinfo.fileName());

                            item_stats->setTextAlignment(Qt::AlignCenter);
                            item_dateTime->setTextAlignment(Qt::AlignCenter);
                            item_platformId->setTextAlignment(Qt::AlignCenter);
                            item_fileName->setTextAlignment(Qt::AlignCenter);

                            ui->tableWidget_yourgames->setItem(ui->tableWidget_yourgames->rowCount()-1, 1, item_stats);
                            ui->tableWidget_yourgames->setItem(ui->tableWidget_yourgames->rowCount()-1, 2, item_dateTime);
                            ui->tableWidget_yourgames->setItem(ui->tableWidget_yourgames->rowCount()-1, 3, item_platformId);
                            ui->tableWidget_yourgames->setItem(ui->tableWidget_yourgames->rowCount()-1, 4, item_fileName);

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
    if (ui->lineEdit_replayserver_address->text().isEmpty()) {
        return;
    }

    if (lrservers.contains(ui->lineEdit_replayserver_address->text())) {
        return;
    }

    lrservers.append(ui->lineEdit_replayserver_address->text());

    ui->tableWidget_replayservers->insertRow(ui->tableWidget_replayservers->rowCount());
    ui->tableWidget_replayservers->setItem(ui->tableWidget_replayservers->rowCount()-1, 0, new QTableWidgetItem(ui->lineEdit_replayserver_address->text()));

    ui->lineEdit_replayserver_address->clear();

    networkManager_replayServers->get(QNetworkRequest(QUrl(lrservers.last())));
}

void MainWindow::slot_summonerinfos_save()
{
    if (ui->lineEdit_summonername->text().isEmpty()) {
        return;
    }

    if (lrservers.isEmpty()) {
        QMessageBox::information(this, tr("LegendsReplay"), tr("Please add a LegendsReplay server."));
        log("Please add a LegendsReplay server.");
        return;
    }

    m_summonername = ui->lineEdit_summonername->text();
    m_summonerServerRegion = ui->comboBox_summonerserver->currentText();

    //Retrieving summoner ID

    log("Retrieving summoner ID");

    QJsonDocument suminfos = getJsonFromUrl(m_currentLegendsReplayServer + "?region=" + m_summonerServerRegion + "&summonername=" + m_summonername);

    if (suminfos.isEmpty()) {
        QMessageBox::information(this, tr("LegendsReplay"), tr("Unknown summoner on this server."));
        log("Unknown summoner on this server.");
        return;
    } else {
        m_summonerid = QString::number(suminfos.object().value(suminfos.object().keys().first()).toObject().value("id").toVariant().toLongLong());
        ui->lineEdit_summonerid->setText(m_summonerid);
        log("Your summoner ID is " + m_summonerid);

        lrsettings->setValue("SummonerName", m_summonername);
        lrsettings->setValue("SummonerId", m_summonerid);
        lrsettings->setValue("SummonerServer", m_summonerServerRegion);
    }
}

bool MainWindow::islolRunning()
{
  QProcess tasklist;
  bool running = false;

#ifdef Q_OS_WIN32

  tasklist.start("tasklist", QStringList() << "/NH" << "/FO" << "CSV" << "/FI" << QString("IMAGENAME eq League of Legends.exe"));
  tasklist.waitForFinished();

  running = QString(tasklist.readAllStandardOutput()).startsWith(QString("\"League of Legends.exe\""));

#elif defined(Q_OS_OSX)

  log("Mac OSX is not yet supported");

#elif defined(Q_OS_LINUX)

  tasklist.start("ps -A | grep \"League of Legends.exe\"");
  tasklist.waitForFinished();

  running = !tasklist.readAllStandardOutput().isEmpty();

#else

  log("OS not supported");

#endif

  return running;
}

bool MainWindow::islolclientRunning()
{
  QProcess tasklist;
  bool running = false;

#ifdef Q_OS_WIN32

  tasklist.start("tasklist", QStringList() << "/NH" << "/FO" << "CSV" << "/FI" << QString("IMAGENAME eq LolClient.exe"));
  tasklist.waitForFinished();

  running = QString(tasklist.readAllStandardOutput()).startsWith(QString("\"LolClient.exe\""));

#elif defined(Q_OS_OSX)

  log("Mac OSX is not yet supported");

#elif defined(Q_OS_LINUX)

  tasklist.start("ps -A | grep \"LolClient.exe\"");
  tasklist.waitForFinished();

  running = !tasklist.readAllStandardOutput().isEmpty();

#else

  log("OS not supported");

#endif

  return running;
}

void MainWindow::slot_refreshPlayingStatus()
{
    if (!islolRunning()) {
        replaying = false;
        playing = false;
        if (httpserver->isListening()) {
            httpserver->stopListening();
            if (replay != nullptr) {
                delete replay;
                replay = nullptr;
            }
            log("Server: stoped due to inactivity");

            this->showNormal();
        }
    }

    if (!replaying && !playing && islolclientRunning() && islolRunning()) {
        //Start recording the game
        playing = true;

        log("Game detected : start recording");

        QJsonDocument gameInfo;

        QTimer timer;
        QEventLoop loop;
        connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
        unsigned int counter = 0;
        timer.start(180000);

        while(gameInfo.isEmpty() || gameInfo.object().value("gameId").toVariant().toLongLong() == 0 || gameInfo.object().value("gameStartTime").toVariant().toLongLong() == 0) {
            loop.exec();

            if (counter > 5) {
                log("[WARN] Game not found : API/LegendsReplay servers may be offline or not working correctly");
                log("End of recording");
                return;
            }

            counter++;

            gameInfo = getCurrentPlayingGameInfos(m_summonerServerRegion, m_summonerid);

            timer.start(60000);
        }

        QString gameId = QString::number(gameInfo.object().value("gameId").toVariant().toLongLong());

        Server server = getServerByRegion(m_summonerServerRegion);
        QString platformId = server.getPlatformId(), serverAddress = server.getUrl();

        if (platformId.isEmpty()) {
            log("[ERROR] Server not found");
            return;
        }

        QString dateTime = QDateTime::fromMSecsSinceEpoch(gameInfo.object().value("gameStartTime").toVariant().toLongLong()).toString();

        refreshRecordingGamesWidget();

        Recorder *recorder = new Recorder(platformId, serverAddress, gameId, gameInfo.object().value("observers").toObject().value("encryptionKey").toString(), gameInfo, replaydirectory);
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

        recording.insert(QPair<QString, QString>(platformId, gameId), QPair<QString, QThread*>(dateTime, recorderThread));
    }

    if (replaying && islolRunning()) {
        m_timer->start(10000);
    } else if(islolclientRunning()){
        m_timer->start(30000);
    } else{
        m_timer->start(60000);
    }

    refreshRecordingGamesWidget();
}

QJsonDocument MainWindow::getCurrentPlayingGameInfos(QString serverRegion, QString summonerId)
{
    QString platformId = getServerByRegion(serverRegion).getPlatformId();

    if (platformId.isEmpty() || m_currentLegendsReplayServer.isEmpty()) {
        QJsonDocument docempty;
        log("[ERROR] Unknown server");
        return docempty;
    }

    QJsonDocument gameinfos = getJsonFromUrl(m_currentLegendsReplayServer + "?platformid=" + platformId + "&summonerid=" + summonerId);

    return gameinfos;
}

void MainWindow::slot_open_replay()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Select a Replay"), replaydirectory, "Replay File (*.lor)");

    if (!path.isEmpty()) {
        replay_launch(path);
    }
}

void MainWindow::slot_doubleclick_savedgames(int row)
{
    replay_launch(replaydirectory + "/" + recordedgames_filename.at(row));
}

void MainWindow::slot_doubleclick_mygames(int row)
{
    replay_launch(replaydirectory + "/" + yourgames_filename.at(row));
}

void MainWindow::replay_launch(QString pathfile)
{
    serverChunkCount = 0;
    serverKeyframeCount = 1;

    replaying = true;

    //Launch spectator server

    if (replay != nullptr) {
        delete replay;
        replay = nullptr;
    }

    replay = new Replay(pathfile);

    replay->repair();

    log("Opening : " + pathfile);

    log("Server: started");

    httpserver->stopListening();

    if (replay->getGameId().isEmpty()) {
        log("Invalid replay file, aborting.");
        delete replay;
        replay = nullptr;
        return;
    }

    httpserver->listen(QHostAddress::Any, 8088, [this](QHttpRequest* req, QHttpResponse* res) {
        QString url = req->url().toString();

        if (url == "/observer-mode/rest/consumer/version") {
            res->setStatusCode(qhttp::ESTATUS_OK);      // http status 200
            res->addHeader("Content-Type", "text/plain");
            res->end(replay->getServerVersion().toLocal8Bit());
            log("Server: send server version");
        } else if(url.contains("/observer-mode/rest/consumer/getGameMetaData/" + replay->getPlatformId() + "/" + replay->getGameId())) {
            int firstchunkid = replay->getChunks().first().getId();

            while(replay->findKeyframeByChunkId(firstchunkid).getId() == 0) {
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

            log("Server: send game metadata");
        } else if (url.contains("/observer-mode/rest/consumer/getLastChunkInfo/" + replay->getPlatformId() + "/" + replay->getGameId())) {
            int endstartupchunkid = replay->getEndStartupChunkId().toInt();
            int startgamechunkid = replay->getStartGameChunkId().toInt();
            int endgamechunkid = 0;
            int nextavailablechunk = 6000;

            if (serverChunkCount < replay->getChunks().first().getId()) {
                serverChunkCount = replay->getChunks().first().getId();
                serverKeyframeCount = replay->getChunks().first().getKeyframeId();
            } else if(serverChunkCount >= replay->getChunks().last().getId()) {
                serverChunkCount = replay->getChunks().last().getId();
                serverKeyframeCount = replay->getKeyFrames().last().getId();
                endgamechunkid = replay->getChunks().last().getId();
            } else {
                serverKeyframeCount = replay->findKeyframeByChunkId(serverChunkCount).getId();
            }

            int currentChunkid = serverChunkCount;

            Keyframe currentKeyframe = replay->findKeyframeByChunkId(currentChunkid);

            while(currentKeyframe.getId() == 0 && currentChunkid < replay->getChunks().last().getId()) {
                currentChunkid++;
                serverChunkCount++;
                currentKeyframe = replay->findKeyframeByChunkId(currentChunkid);
                serverKeyframeCount = currentKeyframe.getId();
            }

            if (serverKeyframeCount == replay->getKeyFrames().first().getId()) {
                nextavailablechunk = 1000;
            } else if(serverKeyframeCount < replay->getKeyFrames().first().getId() + 4) {
                nextavailablechunk = 30000;
            } else if(serverKeyframeCount < replay->getKeyFrames().first().getId() + 10) {
                nextavailablechunk = 2000;
            } else {
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

            log("Server: send lastChunkInfo");
        } else if (url.contains("/observer-mode/rest/consumer/getGameDataChunk/" + replay->getPlatformId() + "/" + replay->getGameId())) {
            int chunkid = -1;

            //Get and send the chunk
            url.remove("/observer-mode/rest/consumer/getGameDataChunk/" + replay->getPlatformId() + "/" + replay->getGameId() + "/");
            chunkid = url.left(url.indexOf("/")).toInt();

            Chunk chunk = replay->getChunk(chunkid);
            Chunk primarychunk = replay->getPrimaryChunk(chunkid);

            if (chunk.getId() > 0) {
                QByteArray chunk_ba = chunk.getData();

                res->setStatusCode(qhttp::ESTATUS_OK);
                res->addHeader("Content-Type", "application/octet-stream");
                res->addHeader("Content-Length", QString::number(chunk_ba.size()).toLocal8Bit());
                res->end(chunk_ba);

                if (serverChunkCount >= replay->getEndStartupChunkId().toInt() && chunkid > replay->getEndStartupChunkId().toInt()) {
                    serverChunkCount++;
                }

                log("Server: send chunk " + QString::number(chunkid));
            } else if (primarychunk.getId() > 0) {
                QByteArray chunk_ba = primarychunk.getData();

                res->setStatusCode(qhttp::ESTATUS_OK);
                res->addHeader("Content-Type", "application/octet-stream");
                res->addHeader("Content-Length", QString::number(chunk_ba.size()).toLocal8Bit());
                res->end(chunk_ba);

                log("Server: send primary chunk " + QString::number(chunkid));
            } else {
                res->setStatusCode(qhttp::ESTATUS_NOT_FOUND);
                res->end("");
                log("Server: unknown requested chunk " + QString::number(chunkid));
            }
        } else if (url.contains("/observer-mode/rest/consumer/getKeyFrame/" + replay->getPlatformId() + "/" + replay->getGameId())) {
            int keyframeid = -1;

            //Get and send the keyframe
            url.remove("/observer-mode/rest/consumer/getKeyFrame/" + replay->getPlatformId() + "/" + replay->getGameId() + "/");
            keyframeid = url.left(url.indexOf("/")).toInt();

            Keyframe keyframe = replay->getKeyFrame(keyframeid);

            if (keyframe.getId() > 0) {
                QByteArray keyframe_ba = keyframe.getData();

                res->setStatusCode(qhttp::ESTATUS_OK);
                res->addHeader("Content-Type", "application/octet-stream");
                res->end(keyframe_ba);

                log("Server: send keyframe " + QString::number(keyframeid));
            } else {
                log("Server: unknown requested keyframe " + QString::number(keyframeid));
            }
        } else if (url.contains("/observer-mode/rest/consumer/endOfGameStats/" + replay->getPlatformId() + "/" + replay->getGameId())) {
            //End of game stats requested
            res->setStatusCode(qhttp::ESTATUS_OK);
            res->addHeader("Content-Type", "application/octet-stream");
            res->end(replay->getEndOfGameStats());

            log("Server: End of game stats sent");
        } else if (url.contains("/observer-mode/rest/consumer/end/" + replay->getPlatformId() + "/" + replay->getGameId())) {
            //End of replay requested : error while replaying
            res->setStatusCode(qhttp::ESTATUS_OK);
            res->addHeader("Content-Type", "text/plain");
            res->end("");

            log("Server: End of replay requested");

            if (replay != nullptr) {
                delete replay;
                replay = nullptr;
            }

            httpserver->stopListening();
        } else {
            res->setStatusCode(qhttp::ESTATUS_OK);
            res->addHeader("Content-Type", "text/plain");
            res->end("");

            log("Server: Unknown requested link " + url);
        }
    });

    //Launch lol client
    lol_launch(replay->getPlatformId(), replay->getEncryptionkey(), replay->getGameId(), true);
}

void MainWindow::rofl_file_launch(QString filepath)
{
    if (filepath.isEmpty()) {
        log("[ERROR] Invalid game parameters.");
        return;
    }

    QDir qd;

    qd.setPath(loldirectory + "/solutions/lol_game_client_sln/releases/");
    qd.setFilter(QDir::Dirs);
    qd.setSorting(QDir::Name | QDir::Reversed);
    QFileInfoList list = qd.entryInfoList();

    if (list.isEmpty()) {
        QMessageBox::information(this, tr("LegendsReplay"), tr("Invalid League of Legends (or PBE) directory.\nPlease set a valid one."));
        log("[WARN] Invalid League of Legends directory. No releases folder found.");
        return;
    }

    QString path = loldirectory + "/solutions/lol_game_client_sln/releases/" + list.first().fileName() + "/deploy/";

    if (!check_path(path)) {
        QMessageBox::information(this, tr("LegendsReplay"), tr("Invalid League of Legends (or PBE) directory.\nPlease set a valid one."));
        log("[WARN] Invalid League of Legends directory. Invalid path.");
        return;
    }

#ifdef Q_OS_WIN32

    log("\"" + path + "League of Legends.exe\" \"" + filepath + "\"");

    QProcess* process = new QProcess();
    connect(process, SIGNAL(finished()), process, SLOT(deleteLater()));
    process->setWorkingDirectory(path);
    process->start("\"" + path + "League of Legends.exe\"", QStringList() << filepath);

#elif defined(Q_OS_OSX)

    log("\"" + path + "League of Legends\" \"" + filepath + "\"");
    log("MacOSX support is experimental");

    QProcess* process = new QProcess();
    connect(process, SIGNAL(finished()), process, SLOT(deleteLater()));
    process->setWorkingDirectory(path);
    process->start("\"" + path + "League of Legends\"", QStringList() << filepath);

#elif defined(Q_OS_UNIX)

    log("This feature isn't available on Linux yet");

#endif

    this->showMinimized();
}

void MainWindow::slot_launch_rofl_file()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Select a Replay File"), replaydirectory, "Replay File (*.rofl)");

    if(!path.isEmpty()){
        rofl_file_launch(path);
    }
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

    QString region = ui->comboBox_searchsummoner_region->currentText();
    QString summonername = ui->lineEdit_searchsummoner->text();

    QJsonDocument suminfos = getJsonFromUrl(m_currentLegendsReplayServer + "?region=" + region + "&summonername=" + summonername);

    if(suminfos.isEmpty()) {
        log("Summoner name not found on this server");
        return;
    }

    QString summonerid = QString::number(suminfos.object().value(suminfos.object().keys().first()).toObject().value("id").toVariant().toLongLong());

    if(summonerid.isEmpty()) {
        log("[ERROR] Summoner id not valid");
        return;
    }

    QJsonDocument game = getCurrentPlayingGameInfos(region, summonerid);

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

    if(game.isEmpty()) {
        log("Summoner " + summonername + " is not in a game");

        ui->label_searchsummoner_status->setText(tr("not playing"));

        ui->label_searchsummoner_gamemode->setText(tr("Game Mode"));
    } else {
        log("Summoner " + summonername + " is in a game");

        ui->label_searchsummoner_status->setText(tr("playing"));

        ui->label_searchsummoner_gamemode->setText(game.object().value("gameMode").toString());

        QJsonArray array = game.object().value("participants").toArray();

        QList<int> leftids;
        QList<QString> leftnames;
        QList<int> rightids;
        QList<QString> rightnames;

        if (array.isEmpty()) {
            return;
        }

        for(int i = 0; i < array.size(); i++){
            if (array.at(i).toObject().value("teamId").toInt() == 200) {
                rightids.append(array.at(i).toObject().value("championId").toInt());
                rightnames.append(array.at(i).toObject().value("summonerName").toString());
            } else {
                leftids.append(array.at(i).toObject().value("championId").toInt());
                leftnames.append(array.at(i).toObject().value("summonerName").toString());
            }
        }

        if (leftids.size() >= 1 && leftnames.size() >= 1) {
            ui->label_sums1->setAlignment(Qt::AlignCenter);
            ui->label_sums1->setPixmap(getImg(leftids.first()));

            ui->label_sums16->setText(leftnames.first());

            if (leftids.size() >= 2 && leftnames.size() >= 2) {
                ui->label_sums2->setAlignment(Qt::AlignCenter);
                ui->label_sums2->setPixmap(getImg(leftids.at(1)));

                ui->label_sums27->setText(leftnames.at(1));

                if (leftids.size() >= 3 && leftnames.size() >= 3) {
                    ui->label_sums3->setAlignment(Qt::AlignCenter);
                    ui->label_sums3->setPixmap(getImg(leftids.at(2)));

                    ui->label_sums38->setText(leftnames.at(2));

                    if (leftids.size() >= 4 && leftnames.size() >= 4) {
                        ui->label_sums4->setAlignment(Qt::AlignCenter);
                        ui->label_sums4->setPixmap(getImg(leftids.at(3)));

                        ui->label_sums49->setText(leftnames.at(3));

                        if (leftids.size() >= 5 && leftnames.size() >= 5) {
                            ui->label_sums5->setAlignment(Qt::AlignCenter);
                            ui->label_sums5->setPixmap(getImg(leftids.at(4)));

                            ui->label_sums510->setText(leftnames.at(4));
                        }
                    }
                }
            }
        }

        if (rightids.size() >= 1 && rightnames.size() >= 1) {
            ui->label_sums6->setAlignment(Qt::AlignCenter);
            ui->label_sums6->setPixmap(getImg(rightids.first()));

            ui->label_sums16->setText(ui->label_sums16->text() + " / " + rightnames.first());

            if(rightids.size() >= 2 && rightnames.size() >= 2) {
                ui->label_sums7->setAlignment(Qt::AlignCenter);
                ui->label_sums7->setPixmap(getImg(rightids.at(1)));

                ui->label_sums27->setText(ui->label_sums27->text() + " / " + rightnames.at(1));

                if (rightids.size() >= 3 && rightnames.size() >= 3) {
                    ui->label_sums8->setAlignment(Qt::AlignCenter);
                    ui->label_sums8->setPixmap(getImg(rightids.at(2)));

                    ui->label_sums38->setText(ui->label_sums38->text() + " / " + rightnames.at(2));

                    if (rightids.size() >= 4 && rightnames.size() >= 4) {
                        ui->label_sums9->setAlignment(Qt::AlignCenter);
                        ui->label_sums9->setPixmap(getImg(rightids.at(3)));

                        ui->label_sums49->setText(ui->label_sums49->text() + " / " + rightnames.at(3));

                        if (rightids.size() >= 5 && rightnames.size() >= 5) {
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
    if (m_searchsummoner_game.isEmpty()) {
        return;
    }

    lol_launch(m_searchsummoner_game.object().value("platformId").toString(), m_searchsummoner_game.object().value("observers").toObject().value("encryptionKey").toString(), QString::number(m_searchsummoner_game.object().value("gameId").toVariant().toULongLong()));
}

void MainWindow::slot_click_searchsummoner_record()
{
    if (m_searchsummoner_game.isEmpty()) {
        return;
    }

    QString platformid = m_searchsummoner_game.object().value("platformId").toString();
    QString gameid = QString::number(m_searchsummoner_game.object().value("gameId").toVariant().toULongLong());
    QString serveraddress = getServerByPlatformId(platformid).getUrl();

    if (serveraddress.isEmpty()) {
        return;
    }

    if (recording.contains(QPair<QString, QString>(platformid, gameid))) {
        log("Game is already recording");
        return;
    }

    QString dateTime;

    if (m_searchsummoner_game.object().value("gameStartTime").toVariant().toULongLong() > 0) {
        dateTime = QDateTime::currentDateTime().toString();
    } else {
        dateTime = QDateTime::fromMSecsSinceEpoch(m_searchsummoner_game.object().value("gameStartTime").toVariant().toLongLong()).toString();
    }

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

    recording.insert(QPair<QString, QString>(platformid, gameid), QPair<QString, QThread*>(dateTime, recorderThread));
}

void MainWindow::systemtrayiconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick) {
        if (this->isMinimized()) {
            this->showNormal();
        } else {
            this->showMinimized();
        }
    }
}

void MainWindow::slot_click_replayservers()
{
    if (ui->tableWidget_replayservers->selectedItems().isEmpty()) {
        return;
    }

    m_currentLegendsReplayServer = ui->tableWidget_replayservers->item(ui->tableWidget_replayservers->selectedItems().first()->row(), 0)->text();

    log("Legends Replay switch to server " + m_currentLegendsReplayServer);
}

void MainWindow::slot_customcontextmenu()
{
    custommenu->clear();

    if (ui->tabWidget->currentIndex() == 0) {
        custommenu->addAction(QIcon(":/icons/open_replay.png"), tr("Re&play"), this, SLOT(slot_menu_replay()));
        custommenu->addAction(QIcon(":/icons/stats.png"), tr("&Stats"), this, SLOT(slot_menu_stats()));
        custommenu->addSeparator();
        custommenu->addAction(QIcon(":/icons/repair.png"), tr("&Repair tool"), this, SLOT(slot_menu_repairtool()));
        custommenu->addSeparator();
        custommenu->addAction(QIcon(":/icons/delete.png"), tr("&Delete"), this, SLOT(slot_menu_delete()));
    } else if (ui->tabWidget->currentIndex() == 2) {
        custommenu->addAction(QIcon(":/icons/open_replay.png"), tr("&Spectate"), this, SLOT(slot_menu_spectate()));
        custommenu->addAction(QIcon(":/icons/record.png"), tr("&Record"), this, SLOT(slot_menu_record()));
    } else if (ui->tabWidget->currentIndex() == 3) {
        custommenu->addAction(QIcon(":/icons/cancel_download.png"), tr("&Cancel"), this, SLOT(slot_menu_cancel()));
        custommenu->addAction(QIcon(":/icons/cancel_delete_download.png"), tr("Cancel and &delete"), this, SLOT(slot_menu_cancelanddelete()));
    } else {
        return;
    }

    custommenu->popup(QCursor::pos());
}

void MainWindow::slot_menu_replay(){
    QString path;

    if (ui->tabWidget_2->currentIndex() == 0 && !ui->tableWidget_yourgames->selectedItems().isEmpty()) {
        path = replaydirectory + "/" + ui->tableWidget_yourgames->item(ui->tableWidget_yourgames->selectedItems().first()->row(), 4)->text();
    } else if(ui->tabWidget_2->currentIndex() == 1 && !ui->tableWidget_recordedgames->selectedItems().isEmpty()){
        path = replaydirectory + "/" + recordedgames_filename.at(ui->tableWidget_recordedgames->selectedItems().first()->row());
    } else {
        return;
    }

    replay_launch(path);
}

void MainWindow::slot_menu_stats(){
    QString path;

    if (ui->tabWidget_2->currentIndex() == 0 && !ui->tableWidget_yourgames->selectedItems().isEmpty()) {
        path = replaydirectory + "/" + ui->tableWidget_yourgames->item(ui->tableWidget_yourgames->selectedItems().first()->row(), 4)->text();
    } else if (ui->tabWidget_2->currentIndex() == 1 && !ui->tableWidget_recordedgames->selectedItems().isEmpty()) {
        path = replaydirectory + "/" + recordedgames_filename.at(ui->tableWidget_recordedgames->selectedItems().first()->row());
    } else {
        return;
    }

    Replay local_replay(path, true);

    if (local_replay.isEmpty()) {
        return;
    }

    QString serverRegion = getServerByPlatformId(local_replay.getPlatformId()).getRegion();

    if (serverRegion.isEmpty()) {
        return;
    }

    if (serverRegion.toLower() == "kr") {
        QDesktopServices::openUrl(QUrl("http://matchhistory.leagueoflegends.co.kr/en/#match-details/KR/" + local_replay.getGameId() + "?tab=overview"));
    } else {
        QDesktopServices::openUrl(QUrl("http://matchhistory." + serverRegion.toLower() + ".leagueoflegends.com/en/#match-details/" + local_replay.getPlatformId() + "/" + local_replay.getGameId() + "?tab=overview"));
    }
}

void MainWindow::slot_menu_repairtool(){
    QString path;

    if (ui->tabWidget_2->currentIndex() == 0 && !ui->tableWidget_yourgames->selectedItems().isEmpty()) {
        path = replaydirectory + "/" + ui->tableWidget_yourgames->item(ui->tableWidget_yourgames->selectedItems().first()->row(), 4)->text();
    } else if (ui->tabWidget_2->currentIndex() == 1 && !ui->tableWidget_recordedgames->selectedItems().isEmpty()) {
        path = replaydirectory + "/" + recordedgames_filename.at(ui->tableWidget_recordedgames->selectedItems().first()->row());
    } else {
        return;
    }

    RepairToolDialog *newRepairToolDialog = new RepairToolDialog(this);
    newRepairToolDialog->show();
    newRepairToolDialog->load(Replay(path));
}

void MainWindow::slot_menu_delete(){
    QString path;

    if (ui->tabWidget_2->currentIndex() == 0 && !ui->tableWidget_yourgames->selectedItems().isEmpty()) {
        path = replaydirectory + "/" + ui->tableWidget_yourgames->item(ui->tableWidget_yourgames->selectedItems().first()->row(), 4)->text();
    } else if (ui->tabWidget_2->currentIndex() == 1 && !ui->tableWidget_recordedgames->selectedItems().isEmpty()) {
        path = replaydirectory + "/" + recordedgames_filename.at(ui->tableWidget_recordedgames->selectedItems().first()->row());
    } else {
        return;
    }

    if (QMessageBox::question(this, tr("Delete"), tr("Do you really want to delete this replay ?")) != QMessageBox::Yes) {
        return;
    }

    if (!QFile::exists(path)) {
        log("Unable to find the file : " + path);
    } else if (!QFile::remove(path)) {
        log("Unable to remove the file : " + path);
    } else if (QFile::exists(path)) {
        log("Unable to remove the file : " + path);
    }
}

void MainWindow::slot_menu_spectate(){
    if (!ui->listWidget_featured->selectedItems().isEmpty()) {
        GameInfosWidget* widget = dynamic_cast<GameInfosWidget*>(ui->listWidget_featured->itemWidget(ui->listWidget_featured->selectedItems().first()));

        lol_launch(widget->getPlatformId(), widget->getEncryptionkey(), widget->getGameId());
    }
}

void MainWindow::slot_menu_record(){
    if (!ui->listWidget_featured->selectedItems().isEmpty()) {
        slot_featuredRecord();
    }
}

void MainWindow::slot_menu_cancel(){
    if (!ui->tableWidget_recordingGames->selectedItems().isEmpty()) {
        if (QMessageBox::question(this, tr("Cancel"), tr("Do you really want to cancel this record ?")) != QMessageBox::Yes) {
            return;
        }

        int row = ui->tableWidget_recordingGames->selectedItems().first()->row();
        QString platformid = ui->tableWidget_recordingGames->item(row, 0)->text(), gameid = ui->tableWidget_recordingGames->item(row, 1)->text();

        if (recording.contains(QPair<QString, QString>(platformid, gameid))) {
            recording.value(QPair<QString, QString>(platformid, gameid)).second->exit(0);
        }
    }
}

void MainWindow::slot_menu_cancelanddelete(){
    if (!ui->tableWidget_recordingGames->selectedItems().isEmpty()) {
        if (QMessageBox::question(this, tr("Cancel"), tr("Do you really want to cancel and delete this record ?")) != QMessageBox::Yes) {
            return;
        }

        int row = ui->tableWidget_recordingGames->selectedItems().first()->row();
        QString platformid = ui->tableWidget_recordingGames->item(row, 0)->text(), gameid = ui->tableWidget_recordingGames->item(row, 0)->text();

        if (recording.contains(QPair<QString, QString>(platformid, gameid))) {
            recording.value(QPair<QString, QString>(platformid, gameid)).second->exit(0);
            recording.value(QPair<QString, QString>(platformid, gameid)).second->wait(1000);
        }

        QString filepath = replaydirectory + "/" + platformid + "-" + gameid + ".lor";

        QFile file(filepath);

        if (!file.exists()) {
            return;
        }

        if (file.open(QIODevice::WriteOnly) && file.remove()) {
            log("Removed file : " + filepath);
        } else {
            log("[ERROR] Unable to remove the file : " + file.errorString());
        }
    }
}

void MainWindow::slot_openAdvancedRecorder()
{
    AdvancedRecorderDialog *newAdvancedRecorderDialog = new AdvancedRecorderDialog(this);
    newAdvancedRecorderDialog->show();
    connect(newAdvancedRecorderDialog, SIGNAL(recordGame(QString, QString, QString, QString, bool, bool, bool)), this, SLOT(slot_customGameRecord(QString, QString, QString, QString, bool, bool, bool)));
}

void MainWindow::slot_customGameRecord(QString serverAddress, QString platformId, QString gameId, QString encryptionKey, bool forceCompleteDownload, bool downloadInfos, bool downloadStats)
{
    Q_UNUSED(downloadInfos);

    if (recording.contains(QPair<QString, QString>(platformId, gameId))) {
        log("Game is already recording");
        return;
    }

    refreshRecordingGamesWidget();

    Recorder *recorder = new Recorder(platformId, serverAddress, gameId, encryptionKey, QJsonDocument(), replaydirectory, forceCompleteDownload, downloadStats);
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

    recording.insert(QPair<QString, QString>(platformId, gameId), QPair <QString, QThread*> (QDateTime::currentDateTime().toString(), recorderThread));
}

void MainWindow::slot_reportAnIssue()
{
    QDesktopServices::openUrl(QUrl("https://github.com/Aztorius/legendsreplay/issues"));
}

void MainWindow::slot_aboutLegendsReplay()
{
    QMessageBox::information(this, tr("About"), "Legends Replay is an open source software (GNU GPL v3).\nThis software use Qt and qHttp.\n\nLegendsReplay isn't endorsed by Riot Games and doesn't reflect the views or opinions of Riot Games or anyone officially involved in producing or managing League of Legends. League of Legends and Riot Games are trademarks or registered trademarks of Riot Games, Inc. League of Legends © Riot Games, Inc.");
}

void MainWindow::refreshRecordingGamesWidget()
{
    ui->tableWidget_recordingGames->clearContents();

    while(ui->tableWidget_recordingGames->rowCount() > 0) {
        ui->tableWidget_recordingGames->removeRow(0);
    }

    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();

    QPair<QString, QThread*> current_record;

    foreach(current_record, recording) {
        ui->tableWidget_recordingGames->insertRow(ui->tableWidget_recordingGames->rowCount());

        ui->tableWidget_recordingGames->setItem(ui->tableWidget_recordingGames->rowCount()-1, 0, new QTableWidgetItem(recording.key(current_record).second));
        ui->tableWidget_recordingGames->setItem(ui->tableWidget_recordingGames->rowCount()-1, 1, new QTableWidgetItem(recording.key(current_record).first));

        QDateTime dateTime = QDateTime::fromString(current_record.first);

        ui->tableWidget_recordingGames->setItem(ui->tableWidget_recordingGames->rowCount()-1, 2, new QTableWidgetItem(dateTime.toString(Qt::DefaultLocaleShortDate)));

        qint64 timeProgress = currentTime - dateTime.toMSecsSinceEpoch();
        timeProgress /= 1000;
        timeProgress /= 60;
        int value = 0;

        if (timeProgress >= 30) {
            value = 90;
        } else {
            value = int(timeProgress * 3);
        }

        QProgressBar* bar = new QProgressBar();
        bar->setValue(value);

        ui->tableWidget_recordingGames->setCellWidget(ui->tableWidget_recordingGames->rowCount()-1, 3, bar);
    }
}

void MainWindow::slot_setLanguage()
{
    if (lrsettings->value("Language").toString() == ui->comboBox_language->currentText()) {
        return;
    }

    lrsettings->setValue("Language", ui->comboBox_language->currentText());

    if (translator.load(QString(":/translation/legendsreplay_") + lrsettings->value("Language").toString().toLower())) {
        qApp->installTranslator(&translator);
    }
}

void MainWindow::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        ui->comboBox_language->setCurrentText(lrsettings->value("Language").toString());
    }

    QMainWindow::changeEvent(event);
}

void MainWindow::slot_checkandrepair(){
    RepairToolDialog *newRepairToolDialog = new RepairToolDialog(this);
    newRepairToolDialog->setDirectory(replaydirectory);
    newRepairToolDialog->show();
}
