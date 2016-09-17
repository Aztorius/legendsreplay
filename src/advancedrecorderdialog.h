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

#ifndef ADVANCEDRECORDERDIALOG_H
#define ADVANCEDRECORDERDIALOG_H

#include <QDialog>

namespace Ui {
class AdvancedRecorderDialog;
}

class AdvancedRecorderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AdvancedRecorderDialog(QWidget *parent = 0);
    ~AdvancedRecorderDialog();

public slots:
    void requestRecordingGame();

private:
    Ui::AdvancedRecorderDialog *ui;

signals:
    void recordGame(QString, QString, QString, QString, bool, bool, bool);
};

#endif // ADVANCEDRECORDERDIALOG_H
