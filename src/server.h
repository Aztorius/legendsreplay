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

#ifndef SERVER_H
#define SERVER_H

#include <QString>

class Server
{
public:
    Server();
    Server(QString name, QString region, QString platformid, QString domain, unsigned int port = 0);

    QString getName() const;
    QString getRegion() const;
    QString getPlatformId() const;
    QString getDomain() const;
    unsigned int getPort() const;
    QString getUrl() const;

    bool isEmpty() const;

    bool operator==(Server const& a);

private:
    QString m_name;
    QString m_region;
    QString m_platformid;
    QString m_domain;
    unsigned int m_port;
    QString m_url;
};

#endif // SERVER_H
