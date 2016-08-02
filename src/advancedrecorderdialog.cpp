#include "advancedrecorderdialog.h"
#include "ui_advancedrecorderdialog.h"

AdvancedRecorderDialog::AdvancedRecorderDialog(QWidget *parent) :
    QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    ui(new Ui::AdvancedRecorderDialog)
{
    ui->setupUi(this);

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(requestRecordingGame()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(close()));
}

AdvancedRecorderDialog::~AdvancedRecorderDialog()
{
    delete ui;
}

void AdvancedRecorderDialog::requestRecordingGame()
{
    emit recordGame(ui->lineEdit_serveraddress->text(), ui->lineEdit_serverregion->text(), ui->lineEdit_gameid->text(), ui->lineEdit_encryptionkey->text(), ui->checkBox_forcecompletedownload->isChecked(), ui->checkBox_downloadGameInfos->isChecked(), ui->checkBox_downloadstats->isChecked());
    emit close();
}
