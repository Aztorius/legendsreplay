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

#include "replay.h"

Replay::Replay(){

}

Replay::Replay(QString filepath, bool loadInfosOnly)
{
    QFile file(filepath);
    m_filepath = filepath;

    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        in.setCodec("UTF-8");

        while(!in.atEnd())
        {
            QString line = in.readLine();

            if(!loadInfosOnly && line.startsWith("::ORChunk:"))
            {
                // A Chunk line ::ORChunk:id:keyframeid:duration:data::

                line.remove(0, 10);
                line.chop(2);

                QStringList elements = line.split(':', QString::KeepEmptyParts);

                if(elements.size() >= 4){

                    int chunkid = elements.at(0).toInt();

                    int keyframeid = elements.at(1).toInt();

                    int chunkduration = elements.at(2).toInt();

                    if(chunkid <= m_endstartupchunkid.toInt())
                    {
                        m_primarychunks.append(Chunk(chunkid, QByteArray::fromBase64(elements.at(3).toLocal8Bit()), keyframeid, chunkduration));
                    }
                    else
                    {
                        m_chunks.append(Chunk(chunkid, QByteArray::fromBase64(elements.at(3).toLocal8Bit()), keyframeid, chunkduration));
                    }
                }
            }
            else if(!loadInfosOnly && line.startsWith("::ORKeyFrame:"))
            {
                // A KeyFrame line ::ORKeyFrame:id:nextchunkid:data::

                line.remove(0, 13);
                line.chop(2);

                QStringList elements = line.split(':', QString::KeepEmptyParts);

                if(elements.size() >= 3){

                    int keyframeid = elements.at(0).toInt();

                    int nextchunkid = elements.at(1).toInt();

                    m_keyframes.append(Keyframe(keyframeid, QByteArray::fromBase64(elements.at(2).toLocal8Bit()), nextchunkid));
                }
            }
            else if(line.startsWith("::ORHeader:"))
            {
                // File Header ::ORHeader:platformid:gameid:encryptionkey:serverversion:endstartupchunkid:startgamechunkid::

                line.remove(0, 11);
                line.chop(2);

                QStringList elements = line.split(':', QString::KeepEmptyParts);

                if(elements.size() >= 6){

                    m_platformid = elements.at(0);

                    m_gameid = elements.at(1);

                    m_encryptionkey = elements.at(2);

                    m_serverversion = elements.at(3);

                    m_endstartupchunkid = elements.at(4);

                    m_startgamechunkid = elements.at(5);
                }
            }
            else if(line.startsWith("::ORGameInfos:"))
            {
                line.remove(0, 14);
                line.chop(2);

                m_gameinfos = QJsonDocument::fromJson(QByteArray::fromBase64(line.toLocal8Bit()));
            }
            else if(line.startsWith("::ORGameStats:"))
            {
                line.remove(0, 14);
                line.chop(2);

                m_endofgamestats = QByteArray::fromBase64(line.toLocal8Bit());
            }
            else if(line == "::OREnd::")
            {
                break;
            }

            if(loadInfosOnly && !m_gameinfos.isEmpty() && !m_endofgamestats.isEmpty() && !m_encryptionkey.isEmpty()){
                break;
            }
        }
        file.close();

        if(m_encryptionkey.isEmpty() && !m_gameinfos.isEmpty() && !m_gameinfos.object().value("observers").toObject().value("encryptionKey").toString().isEmpty()){
            m_encryptionkey = m_gameinfos.object().value("observers").toObject().value("encryptionKey").toString();
        }

        if(m_platformid.isEmpty() && !m_gameinfos.isEmpty() && !m_gameinfos.object().value("platformId").toString().isEmpty()){
            m_platformid = m_gameinfos.object().value("platformId").toString();
        }

        if(m_gameid.isEmpty() && !m_gameinfos.isEmpty() && m_gameinfos.object().value("gameId").toVariant().toULongLong() != 0){
            m_gameid = QString::number(m_gameinfos.object().value("gameId").toVariant().toULongLong());
        }
    }
    else{
        //ERROR : cannot open the file
        qDebug() << "[ERROR] Cannot open the file";
    }
}

Replay::~Replay()
{

}

QString Replay::getFilepath() const
{
    return m_filepath;
}

QString Replay::getGameId() const
{
    return m_gameid;
}

QString Replay::getPlatformId() const
{
    return m_platformid;
}

QString Replay::getEncryptionkey() const
{
    return m_encryptionkey;
}

QList<Keyframe> Replay::getKeyFrames() const
{
    return m_keyframes;
}

QList<Chunk> Replay::getChunks() const
{
    return m_chunks;
}

QList<Chunk> Replay::getPrimaryChunks() const
{
    return m_primarychunks;
}

QJsonDocument Replay::getGameinfos() const
{
    return m_gameinfos;
}

QString Replay::getServerVersion() const
{
    return m_serverversion;
}

QString Replay::getEndStartupChunkId() const
{
    return m_endstartupchunkid;
}

QString Replay::getStartGameChunkId() const
{
    return m_startgamechunkid;
}

Chunk Replay::getChunk(int id) const
{
    Chunk chunk;
    for(int i = 0; i < m_chunks.size(); i++)
    {
        if(m_chunks.at(i).getId() == id)
        {
            chunk = m_chunks.at(i);
            break;
        }
    }
    return chunk;
}

Chunk Replay::getPrimaryChunk(int id) const
{
    Chunk chunk;
    for(int i = 0; i < m_primarychunks.size(); i++)
    {
        if(m_primarychunks.at(i).getId() == id)
        {
            chunk = m_primarychunks.at(i);
            break;
        }
    }
    return chunk;
}

Keyframe Replay::getKeyFrame(int id) const
{
    Keyframe keyframe;
    for(int i = 0; i < m_keyframes.size(); i++)
    {
        if(m_keyframes.at(i).getId() == id)
        {
            keyframe = m_keyframes.at(i);
            break;
        }
    }
    return keyframe;
}

Keyframe Replay::findKeyframeByChunkId(int chunkid) const
{
    Keyframe keyframe;
    for(int i = 0; i < m_keyframes.size(); i++)
    {
        if(m_keyframes.at(i).getNextchunkid() == chunkid)
        {
            keyframe = m_keyframes.at(i);
            break;
        }
        else if(m_keyframes.at(i).getNextchunkid() == chunkid - 1)
        {
            keyframe = m_keyframes.at(i);
            break;
        }
    }
    return keyframe;
}

QByteArray Replay::getEndOfGameStats() const
{
    return QByteArray::fromBase64(m_endofgamestats);
}

bool Replay::repair(){
    while(m_keyframes.size() > 0 && this->getChunk(m_keyframes.first().getNextchunkid()).getId() == 0){
        m_keyframes.removeFirst();
    }

    return true;
}

void Replay::setGameId(QString gameId){
    m_gameid = gameId;
}

void Replay::setPlatformId(QString platformId){
    m_platformid = platformId;
}

void Replay::setEncryptionKey(QString encryptionKey){
    m_encryptionkey = encryptionKey;
}

void Replay::setServerVersion(QString serverVersion){
    m_serverversion = serverVersion;
}

void Replay::setEndStartupChunkId(int id){
    m_endstartupchunkid = QString::number(id);
}

void Replay::setStartGameChunkId(int id){
    m_startgamechunkid = QString::number(id);
}

void Replay::removeKeyFrame(int id){
    for(int i = 0; i < m_keyframes.size(); i++){
        if(m_keyframes.at(i).getId() == id){
            m_keyframes.removeAt(i);
            break;
        }
    }
}

void Replay::removeChunk(int id){
    for(int i = 0; i < m_chunks.size(); i++){
        if(m_chunks.at(i).getId() == id){
            m_chunks.removeAt(i);
            break;
        }
    }
}

void Replay::setKeyFrames(QList<Keyframe> keyframes){
    m_keyframes = keyframes;
}

bool Replay::saveAs(QString filepath){
    QFile file(filepath);

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        return false;
    }

    QTextStream stream(&file);

    stream << "::ORHeader:" << m_platformid << ":" << m_gameid << ":" << m_encryptionkey << ":" << m_serverversion << ":" << m_endstartupchunkid << ":" << m_startgamechunkid << "::" << endl;

    if(!m_gameinfos.isEmpty()){
        stream << "::ORGameInfos:" << m_gameinfos.toJson(QJsonDocument::Compact).toBase64() << "::" << endl;
    }

    if(!m_endofgamestats.isEmpty()){
        stream << "::ORGameStats:" << m_endofgamestats.toBase64() << "::" << endl;
    }

    foreach(Keyframe currentKF, m_keyframes){
        stream << "::ORKeyFrame:" << QString::number(currentKF.getId()) << ":" << QString::number(currentKF.getNextchunkid()) << ":" << currentKF.getData().toBase64() << "::" << endl;
    }

    foreach(Chunk currentCK, m_primarychunks){
        stream << "::ORChunk:" << QString::number(currentCK.getId()) << ":" << QString::number(currentCK.getKeyframeId()) << ":" << QString::number(currentCK.getDuration()) << ":" << currentCK.getData().toBase64() << "::" << endl;
    }

    foreach(Chunk currentCK, m_chunks){
        stream << "::ORChunk:" << QString::number(currentCK.getId()) << ":" << QString::number(currentCK.getKeyframeId()) << ":" << QString::number(currentCK.getDuration()) << ":" << currentCK.getData().toBase64() << "::" << endl;
    }

    stream << "::OREnd::" << endl;

    file.close();

    return true;
}

bool Replay::isEmpty(){
    return m_keyframes.isEmpty() && m_chunks.isEmpty() && m_primarychunks.isEmpty() && m_gameid.isEmpty() && m_platformid.isEmpty() && m_encryptionkey.isEmpty() && m_endstartupchunkid.isEmpty() && m_startgamechunkid.isEmpty();
}
