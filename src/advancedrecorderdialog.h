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
