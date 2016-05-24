#include "replay.h"

Replay::Replay(QString filepath)
{
    QFile file(filepath);
    m_filepath = filepath;

    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        in.setCodec("UTF-8");
        while (!in.atEnd())
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
                m_serverid = serverid;

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
            else if(line.left(13) == "::ORKeyFrame:")
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
            else if(line.left(10) == "::ORChunk:")
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
        }

        file.close();
    }
}

QString Replay::getGameid()
{
    return m_gameid;
}

QString Replay::getServerid()
{
    return m_serverid;
}

QString Replay::getEncryptionkey()
{
    return m_encryptionkey;
}

QList<Keyframe> Replay::getKeyFrames()
{
    return m_keyframes;
}

QList<Chunk> Replay::getChunks()
{
    return m_chunks;
}

QList<Chunk> Replay::getPrimaryChunks()
{
    return m_primarychunks;
}

QJsonDocument Replay::getGameinfos()
{
    return m_gameinfos;
}

QString Replay::getServerversion()
{
    return m_serverversion;
}

QString Replay::getEndstartupchunkid()
{
    return m_endstartupchunkid;
}

QString Replay::getStartgamechunkid()
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

Keyframe Replay::findKeyframeByChunkId(int chunkid)
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

QByteArray Replay::getEndOfGameStats()
{
    return QByteArray::fromBase64(m_endofgamestats);
}

Chunk::Chunk()
{
    m_id = 0;
    m_keyframeid = 0;
    m_data = "";
    m_duration = 30000;
}

Chunk::Chunk(int id, QByteArray data, int keyframeid, int duration)
{
    m_id = id;
    m_keyframeid = keyframeid;
    m_data = data;
    m_duration = duration;
}

Chunk::~Chunk()
{

}

int Chunk::getId() const
{
    return m_id;
}

int Chunk::getKeyframeId() const
{
    return m_keyframeid;
}

QByteArray Chunk::getData() const
{
    return m_data;
}

int Chunk::getDuration() const
{
    return m_duration;
}

int Chunk::getSize() const
{
    return m_data.size();
}

Keyframe::Keyframe()
{
    m_id = 0;
    m_nextchunkid = 0;
    m_data = "";
}

Keyframe::Keyframe(int id, QByteArray data, int nextchunkid)
{
    m_id = id;
    m_nextchunkid = nextchunkid;
    m_data = data;
}

Keyframe::~Keyframe()
{

}

int Keyframe::getId() const
{
    return m_id;
}

int Keyframe::getNextchunkid() const
{
    return m_nextchunkid;
}

QByteArray Keyframe::getData() const
{
    return m_data;
}

int Keyframe::getSize() const
{
    return m_data.size();
}
