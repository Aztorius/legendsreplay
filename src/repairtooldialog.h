#ifndef CHECKANDREPAIRDIALOG_H
#define CHECKANDREPAIRDIALOG_H

#include <QDialog>
#include <QMessageBox>
#include <QFileDialog>

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

private:
    Ui::RepairToolDialog *ui;

    Replay m_replay;
    bool m_checked;

private slots:
    void repair();
    void check();
    void openFile();
};

#endif // CHECKANDREPAIRDIALOG_H
