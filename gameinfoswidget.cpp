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
