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
