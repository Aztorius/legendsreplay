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

            if(line == "::OREnd::")
            {
                break;
            }
            else if(line.left(11) == "::ORHeader:")
            {
                line.remove(0,11);

                QString serverid = line.left(line.indexOf(":"));
                m_platformid = serverid;

                line.remove(0,line.indexOf(":")+1);

                QString gameid = line.left(line.indexOf(":"));
                m_gameid = gameid;

                line.remove(0,line.indexOf(":")+1);

                QString encryptionkey = line.left(line.indexOf(":"));
                m_encryptionkey = encryptionkey;

                line.remove(0,line.indexOf(":")+1);

                QString serverversion = line.left(line.indexOf(":"));
                m_serverversion = serverversion;

                line.remove(0,line.indexOf(":")+1);

                QString endstartupchunkid = line.left(line.indexOf(":"));
                m_endstartupchunkid = endstartupchunkid;

                line.remove(0,line.indexOf(":")+1);

                QString startgamechunkid = line.left(line.indexOf(":"));
                m_startgamechunkid = startgamechunkid;
            }
            else if(line.left(14) == "::ORGameInfos:")
            {
                line.remove(0,14);

                QString gameinfos64 = line.left(line.indexOf(":"));
                QByteArray gameinfosba = QByteArray::fromBase64(gameinfos64.toLocal8Bit());

                QJsonDocument jd_gameinfos = QJsonDocument::fromJson(gameinfosba);
                m_gameinfos = jd_gameinfos;
            }
            else if(line.left(14) == "::ORGameStats:")
            {
                line.remove(0,14);

                QString gamestats64 = line.left(line.indexOf(":"));
                QByteArray gamestats = QByteArray::fromBase64(gamestats64.toLocal8Bit());

                m_endofgamestats = gamestats;
            }
            else if(!loadInfosOnly && line.left(13) == "::ORKeyFrame:")
            {
                line.remove(0,13);

                int keyframeid = line.left(line.indexOf(":")).toInt();
                line.remove(0,line.indexOf(":")+1);

                int nextchunkid = line.left(line.indexOf(":")).toInt();
                line.remove(0,line.indexOf(":")+1);

                QString keyframe = line.left(line.indexOf(":"));
                QByteArray ba_keyframe = QByteArray::fromBase64(keyframe.toLocal8Bit());
                m_keyframes.append(Keyframe(keyframeid, ba_keyframe, nextchunkid));
            }
            else if(!loadInfosOnly && line.left(10) == "::ORChunk:")
            {
                line.remove(0,10);

                int chunkid = line.left(line.indexOf(":")).toInt();
                line.remove(0,line.indexOf(":")+1);

                int keyframeid = line.left(line.indexOf(":")).toInt();
                line.remove(0,line.indexOf(":")+1);

                int chunkduration = line.left(line.indexOf(":")).toInt();
                line.remove(0,line.indexOf(":")+1);

                QString chunk = line.left(line.indexOf(":"));
                QByteArray ba_chunk = QByteArray::fromBase64(chunk.toLocal8Bit());

                if(chunkid <= m_endstartupchunkid.toInt())
                {
                    m_primarychunks.append(Chunk(chunkid, ba_chunk, keyframeid, chunkduration));
                }
                else
                {
                    m_chunks.append(Chunk(chunkid, ba_chunk, keyframeid, chunkduration));
                }
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
