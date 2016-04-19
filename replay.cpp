#include "replay.h"

Replay::Replay(QString filepath)
{
    QFile file(filepath);
    m_filepath = filepath;

    if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
        QTextStream in(&file);
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
            }
            else if(line.left(14) == "::ORGameInfos:"){
                line.remove(0,14);

                QString gameinfos = line.left(line.indexOf(":"));
                QByteArray ba_gameinfos = gameinfos.toUtf8();
                ba_gameinfos.fromBase64(ba_gameinfos);
                QJsonDocument jd_gameinfos;
                jd_gameinfos.fromBinaryData(ba_gameinfos);
                m_gameinfos = jd_gameinfos;
            }
            else if(line.left(13) == "::ORKeyFrame:"){
                line.remove(0,13);

                m_keyframesid.append(line.left(line.indexOf(":")).toInt());
                line.remove(0,line.indexOf(":")+1);

                QString keyframe = line.left(line.indexOf(":"));
                QByteArray ba_keyframe = keyframe.toUtf8();
                ba_keyframe.fromBase64(ba_keyframe);
                m_keyframes.append(ba_keyframe);
            }
            else if(line.left(10) == "::ORChunk:"){
                line.remove(0,10);

                m_chunksid.append(line.left(line.indexOf(":")).toInt());
                line.remove(0,line.indexOf(":")+1);

                QString chunk = line.left(line.indexOf(":"));
                QByteArray ba_chunk = chunk.toUtf8();
                ba_chunk.fromBase64(ba_chunk);
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
