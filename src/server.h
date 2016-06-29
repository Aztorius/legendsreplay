#ifndef SERVER_H
#define SERVER_H

#include <QString>

class Server
{
public:
    Server(QString name, QString region, QString platformid, QString domain, unsigned int port = 0);
    QString getName() const;
    QString getRegion() const;
    QString getPlatformId() const;
    QString getDomain() const;
    unsigned int getPort() const;
    QString getUrl() const;

private:
    QString m_name;
    QString m_region;
    QString m_platformid;
    QString m_domain;
    unsigned int m_port;
    QString m_url;
};

#endif // SERVER_H
