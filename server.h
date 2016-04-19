#ifndef SERVER_H
#define SERVER_H

#include <QThread>
#include <QMutex>
#include "mainwindow.h"

class server : public QThread
{
public:
    server();
};

#endif // SERVER_H
