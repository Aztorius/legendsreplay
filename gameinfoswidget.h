#ifndef GAMEINFOSWIDGET_H
#define GAMEINFOSWIDGET_H

#include <QWidget>

namespace Ui {
class GameInfosWidget;
}

class GameInfosWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GameInfosWidget(QWidget *parent = 0);
    ~GameInfosWidget();

private:
    Ui::GameInfosWidget *ui;
};

#endif // GAMEINFOSWIDGET_H
