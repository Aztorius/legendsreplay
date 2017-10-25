/* ****************************************************************************
 *
 * Copyright 2016-2017 William Bonnaventure
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

#ifndef CHECKANDREPAIRDIALOG_H
#define CHECKANDREPAIRDIALOG_H

#include <QDialog>

#include "replay.h"

namespace Ui {
class RepairToolDialog;
}

class RepairToolDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RepairToolDialog(QWidget *parent = 0);
    ~RepairToolDialog();
    void load(Replay replay);
    void setDirectory(QString replays_directory);

private:
    Ui::RepairToolDialog *ui;

    Replay m_replay;
    bool m_checked;
    QString m_directory;

private slots:
    void repair();
    void check();
    void openFile();
};

#endif // CHECKANDREPAIRDIALOG_H
