#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qApp->connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));

    MainWindow w;
    w.setArgs(argc, argv);
    w.show();

    return a.exec();
}
