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

#ifndef GAMEINFOSWIDGET_H
#define GAMEINFOSWIDGET_H

#include <QWidget>
#include <QJsonDocument>

namespace Ui {
class GameInfosWidget;
}

class GameInfosWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GameInfosWidget(QWidget *parent = 0);
    ~GameInfosWidget();
    void setGameInfos(QJsonDocument gameinfos);
    void setGameHeader(QString platformid, QString gameid, QString encryptionkey);
    QString getPlatformId();
    QString getGameId();
    QString getEncryptionkey();
    QPixmap getImg(int id);

private:
    Ui::GameInfosWidget *ui;
    QJsonDocument m_gameinfos;
    QString m_platformid;
    QString m_gameid;
    QString m_encryptionkey;
};

#endif // GAMEINFOSWIDGET_H
