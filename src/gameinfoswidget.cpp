#include "gameinfoswidget.h"
#include "ui_gameinfoswidget.h"

GameInfosWidget::GameInfosWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GameInfosWidget)
{
    ui->setupUi(this);
    setFixedSize(600,120);
}

GameInfosWidget::~GameInfosWidget()
{
    delete ui;
}

void GameInfosWidget::setGameInfos(QJsonDocument gameinfos)
{
    m_gameinfos = gameinfos;

    QJsonArray participants = gameinfos.object().value("participants").toArray();

    QGridLayout *layout = new QGridLayout();
    int last100teamindex = 0;

    for(int i = 0; i < participants.size(); i++){
        if(participants.at(i).toObject().value("teamId").toInt() == 100){
            QLabel *label = new QLabel(this);
            QLabel *label2 = new QLabel(this);

            label->setPixmap(getImg(participants.at(i).toObject().value("championId").toInt()));
            label->setAlignment(Qt::AlignCenter);

            label2->setText(participants.at(i).toObject().value("summonerName").toString());
            label2->setAlignment(Qt::AlignCenter);

            layout->addWidget(label, 0, i+1);
            layout->addWidget(label2, 1, i+1);

            last100teamindex = i;
        }
        else{
            QLabel *label = new QLabel(this);
            QLabel *label2 = new QLabel(this);

            label->setPixmap(getImg(participants.at(i).toObject().value("championId").toInt()));
            label->setAlignment(Qt::AlignCenter);

            label2->setText(participants.at(i).toObject().value("summonerName").toString());
            label2->setAlignment(Qt::AlignCenter);

            layout->addWidget(label, 3, i - last100teamindex);
            layout->addWidget(label2, 2, i - last100teamindex);
        }
    }

    QLabel *label_serverregion = new QLabel(m_serverid, this);
    layout->addWidget(label_serverregion, 0, 0);

    setLayout(layout);

    show();
}

void GameInfosWidget::setGameHeader(QString serverid, QString gameid, QString encryptionkey)
{
    m_serverid = serverid;
    m_gameid = gameid;
    m_encryptionkey = encryptionkey;
}

QString GameInfosWidget::getServerId()
{
    return m_serverid;
}

QString GameInfosWidget::getGameId()
{
    return m_gameid;
}

QString GameInfosWidget::getEncryptionkey()
{
    return m_encryptionkey;
}

QPixmap GameInfosWidget::getImg(int id)
{
    int finalid = 0;

    QFileInfo info(":/img/" + QString::number(id) + ".png");
    if(info.exists() && info.isFile()){
        finalid = id;
    }

    QPixmap img(":/img/" + QString::number(finalid) + ".png");
    return img.scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}
