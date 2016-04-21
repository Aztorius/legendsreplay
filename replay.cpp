#include "replay.h"

Replay::Replay(QString filepath)
{
    QFile file(filepath);
    m_filepath = filepath;

    if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
        QTextStream in(&file);
        in.setCodec("UTF-8");
        while (!in.atEnd()) {
            QString line = in.readLine();

            if(line == "::OREnd::"){
                break;
            }
            else if(line.left(11) == "::ORHeader:"){
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
            else if(line.left(14) == "::ORGameInfos:"){
                line.remove(0,14);

                QString gameinfos64 = line.left(line.indexOf(":"));
                QByteArray gameinfosba = QByteArray::fromBase64(gameinfos64.toLocal8Bit());

                QJsonDocument jd_gameinfos = QJsonDocument::fromJson(gameinfosba);
                m_gameinfos = jd_gameinfos;
            }
            else if(line.left(13) == "::ORKeyFrame:"){
                line.remove(0,13);

                m_keyframesid.append(line.left(line.indexOf(":")).toInt());
                line.remove(0,line.indexOf(":")+1);

                QString keyframe = line.left(line.indexOf(":"));
                QByteArray ba_keyframe = QByteArray::fromBase64(keyframe.toLocal8Bit());
                m_keyframes.append(ba_keyframe);
            }
            else if(line.left(10) == "::ORChunk:"){
                line.remove(0,10);

                m_chunksid.append(line.left(line.indexOf(":")).toInt());
                line.remove(0,line.indexOf(":")+1);

                QString chunk = line.left(line.indexOf(":"));
                QByteArray ba_chunk = QByteArray::fromBase64(chunk.toLocal8Bit());
                m_chunks.append(ba_chunk);
            }
        }
    }
}

QString Replay::getGameid(){
    return m_gameid;
}

QString Replay::getServerid(){
    return m_serverid;
}

QString Replay::getEncryptionkey(){
    return m_encryptionkey;
}

QList<QByteArray> Replay::getKeyFrames(){
    return m_keyframes;
}

QList<QByteArray> Replay::getChunks(){
    return m_chunks;
}

QList<int> Replay::getKeyFramesid(){
    return m_keyframesid;
}

QList<int> Replay::getChunksid(){
    return m_chunksid;
}

QJsonDocument Replay::getGameinfos(){
    return m_gameinfos;
}

QString Replay::getServerversion(){
    return m_serverversion;
}

QString Replay::getEndstartupchunkid(){
    return m_endstartupchunkid;
}

QString Replay::getStartgamechunkid(){
    return m_startgamechunkid;
}
