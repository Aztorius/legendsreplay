/* ****************************************************************************
 *
 * Copyright 2016 William Bonnaventure
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

#include "gameinfoswidget.h"
#include "ui_gameinfoswidget.h"

GameInfosWidget::GameInfosWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GameInfosWidget)
{
    ui->setupUi(this);
    setFixedSize(600, 200);
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
    layout->setSpacing(0);

    int last100teamindex = 0;

    for(int i = 0; i < participants.size(); i++){
        if(participants.at(i).toObject().value("teamId").toInt() == 100){
            QLabel *label = new QLabel(this);
            QLabel *label2 = new QLabel(this);

            label->setPixmap(getImg(participants.at(i).toObject().value("championId").toInt()));
            label->setAlignment(Qt::AlignCenter);
            label->setStyleSheet("QLabel { background-color : #44BBFF; }");

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
            label->setStyleSheet("QLabel { background-color : #B33AD8; }");

            label2->setText(participants.at(i).toObject().value("summonerName").toString());
            label2->setAlignment(Qt::AlignCenter);

            layout->addWidget(label, 2, i - last100teamindex);
            layout->addWidget(label2, 3, i - last100teamindex);
        }
    }

    QLabel *label_serverregion = new QLabel(m_platformid, this);
    QLabel *label_gamemode = new QLabel(gameinfos.object().value("gameMode").toString(), this);

    QDateTime starttime = QDateTime::fromMSecsSinceEpoch(qint64(gameinfos.object().value("gameStartTime").toVariant().toLongLong()));
    QLabel *label_startTime = new QLabel(starttime.toString(Qt::DefaultLocaleShortDate));

    QTime gameLength(0, 0);
    gameLength = gameLength.addSecs(gameinfos.object().value("gameLength").toVariant().toLongLong());
    QLabel *label_gameLength = new QLabel(gameLength.toString(Qt::TextDate));

    layout->addWidget(label_serverregion, 0, 0);
    layout->addWidget(label_gamemode, 1, 0);
    layout->addWidget(label_startTime, 2, 0);
    layout->addWidget(label_gameLength, 3, 0);

    setLayout(layout);

    show();
}

void GameInfosWidget::setGameHeader(QString platformid, QString gameid, QString encryptionkey)
{
    m_platformid = platformid;
    m_gameid = gameid;
    m_encryptionkey = encryptionkey;
}

QString GameInfosWidget::getPlatformId()
{
    return m_platformid;
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
    return img.scaled(60, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}
