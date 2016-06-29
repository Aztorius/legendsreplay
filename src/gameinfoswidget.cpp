#include "gameinfoswidget.h"
#include "ui_gameinfoswidget.h"

GameInfosWidget::GameInfosWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GameInfosWidget)
{
    ui->setupUi(this);
}

GameInfosWidget::~GameInfosWidget()
{
    delete ui;
}

void GameInfosWidget::setGameInfos(QJsonDocument gameinfos)
{
    m_gameinfos = gameinfos;
}

void GameInfosWidget::setGameHeader(QString serverid, QString gameid, QString encryptionkey)
{
    m_serverid = serverid;
    m_gameid = gameid;
    m_encryptionkey = encryptionkey;
}
