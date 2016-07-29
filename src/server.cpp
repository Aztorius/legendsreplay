#include "server.h"

Server::Server()
{
    m_name = "";
    m_region = "";
    m_platformid = "";
    m_domain = "";
    m_port = 0;
    m_url = "";
}

Server::Server(QString name, QString region, QString platformid, QString domain, unsigned int port)
{
    m_name = name;
    m_region = region;
    m_platformid = platformid;
    m_domain = domain;
    m_port = port;
    m_url = m_domain + ":" + QString::number(m_port);
}

QString Server::getName() const
{
    return m_name;
}

QString Server::getRegion() const
{
    return m_region;
}

QString Server::getPlatformId() const
{
    return m_platformid;
}

QString Server::getDomain() const
{
    return m_domain;
}

unsigned int Server::getPort() const
{
    return m_port;
}

QString Server::getUrl() const
{
    return m_url;
}
