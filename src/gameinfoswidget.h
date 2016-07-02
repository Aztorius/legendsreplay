#ifndef GAMEINFOSWIDGET_H
#define GAMEINFOSWIDGET_H

#include <QWidget>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QLabel>
#include <QHBoxLayout>

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
    void setGameHeader(QString serverid, QString gameid, QString encryptionkey);
    QString getServerId();
    QString getGameId();
    QString getEncryptionkey();
    QPixmap getImg(int id);

private:
    Ui::GameInfosWidget *ui;
    QJsonDocument m_gameinfos;
    QString m_serverid;
    QString m_gameid;
    QString m_encryptionkey;
};

#endif // GAMEINFOSWIDGET_H
