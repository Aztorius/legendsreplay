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

bool Server::isEmpty() const
{
    return m_domain.isEmpty() || m_port == 0 || m_name.isEmpty() || m_region.isEmpty() || m_platformid.isEmpty() || m_url.isEmpty();
}

bool Server::operator ==(Server const& a){
    return (a.getDomain() == m_domain && a.getName() == m_name && a.getPlatformId() == m_platformid && a.getPort() == m_port && a.getRegion() == m_region && a.getUrl() == m_url);
}
