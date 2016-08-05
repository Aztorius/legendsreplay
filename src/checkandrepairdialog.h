#ifndef CHECKANDREPAIRDIALOG_H
#define CHECKANDREPAIRDIALOG_H

#include <QDialog>
#include <QMessageBox>
#include <QFileDialog>

#include "replay.h"

namespace Ui {
class CheckAndRepairDialog;
}

class CheckAndRepairDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CheckAndRepairDialog(QWidget *parent = 0);
    ~CheckAndRepairDialog();
    void load(Replay replay);

private:
    Ui::CheckAndRepairDialog *ui;

    Replay m_replay;
    bool m_checked;

private slots:
    void repair();
    void check();
    void openFile();
};

#endif // CHECKANDREPAIRDIALOG_H
