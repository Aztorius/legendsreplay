#include "recorder.h"

Recorder::Recorder(MainWindow *window, QString serverid, QString serveraddress, QString gameid, QString encryptionkey, QJsonDocument gameinfo, QString replaydirectory)
{
    m_serverid = serverid;
    m_serveraddress = serveraddress;
    m_gameid = gameid;
    m_gameinfo = gameinfo;
    m_replaydirectory = replaydirectory;

    if(!encryptionkey.isEmpty()){
        m_encryptionkey = encryptionkey;
    }
    else if(!m_gameinfo.isEmpty()){
        m_encryptionkey = m_gameinfo.object().value("observers").toObject().value("encryptionKey").toString();
    }
    else{
        m_encryptionkey = "";
    }

    connect(this, SIGNAL(toLog(QString)), window, SLOT(log(QString)));
    connect(this, SIGNAL(toShowmessage(QString)), window, SLOT(showmessage(QString)));
}

QByteArray Recorder::getFileFromUrl(QString url){
    QNetworkAccessManager local_networkResult;
    QNetworkReply *reply = local_networkResult.get(QNetworkRequest(QUrl(url)));

    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if(reply->error() != QNetworkReply::NoError)
    {
        QByteArray emptyArray;
        return emptyArray;
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();
    return data;
}

QJsonDocument Recorder::getJsonFromUrl(QString url){
    QNetworkAccessManager local_networkResult;
    QNetworkReply *reply = local_networkResult.get(QNetworkRequest(QUrl(url)));

    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if(reply->error() != QNetworkReply::NoError)
    {
        QJsonDocument jsonEmpty;
        return jsonEmpty;
    }

    QString data = (QString) reply->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(data.toUtf8());
    reply->deleteLater();

    return jsonResponse;
}

void Recorder::run(){

    emit toLog("Start recording : " + m_serverid + "/" + m_gameid);

    QString version;
    version = getFileFromUrl(QString("http://" + m_serveraddress + "/observer-mode/rest/consumer/version"));

    QJsonDocument json_gameMetaData = getJsonFromUrl(QString("http://" + m_serveraddress + "/observer-mode/rest/consumer/getGameMetaData/" + m_serverid + "/" + m_gameid + "/token"));
    QJsonDocument json_lastChunkInfo = getJsonFromUrl(QString("http://" + m_serveraddress + "/observer-mode/rest/consumer/getLastChunkInfo/" + m_serverid + "/" + m_gameid + "/0/token"));

    QTimer timer2;
    QEventLoop loop2;
    connect(&timer2, SIGNAL(timeout()), &loop2, SLOT(quit()));
    unsigned int counter = 0;

    while(json_gameMetaData.isEmpty() || json_lastChunkInfo.isEmpty() || json_lastChunkInfo.object().value("chunkId").toInt() == 0)
    {
        counter++;
        if(counter > 30)
        {
            emit toLog("[ERROR] No valid response from spectator server, aborting recorder : " + m_serverid + "/" + m_gameid);
            return;
        }
        json_gameMetaData = getJsonFromUrl(QString("http://" + m_serveraddress + "/observer-mode/rest/consumer/getGameMetaData/" + m_serverid + "/" + m_gameid + "/token"));
        json_lastChunkInfo = getJsonFromUrl(QString("http://" + m_serveraddress + "/observer-mode/rest/consumer/getLastChunkInfo/" + m_serverid + "/" + m_gameid + "/0/token"));

        //Retry every 28 seconds
        timer2.start(28000);
        loop2.exec();
    }

    QList<Keyframe> list_keyframes;
    QList<Chunk> list_chunks;
    QList<Chunk> list_primarychunks;

    int keyframeid = json_gameMetaData.object().value("pendingAvailableKeyFrameInfo").toArray().first().toObject().value("id").toInt();

    if(keyframeid < 1)
    {
        keyframeid = 0;
    }

    int lastsavedkeyframeid = -1;
    int chunkid = json_gameMetaData.object().value("pendingAvailableChunkInfo").toArray().first().toObject().value("id").toInt();

    if(chunkid <= json_lastChunkInfo.object().value("endStartupChunkId").toInt())
    {
        chunkid = json_lastChunkInfo.object().value("endStartupChunkId").toInt() + 1;
    }

    int lastsavedchunkid = -1;
    int startupchunkid = 1;

    QByteArray bytearray_keyframe, bytearray_chunk;

    QTimer timer, timer3;
    QEventLoop loop, loop3;
    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    connect(&timer3, SIGNAL(timeout()), &loop3, SLOT(quit()));

    while(lastsavedchunkid != json_lastChunkInfo.object().value("endGameChunkId").toInt() || lastsavedkeyframeid != json_lastChunkInfo.object().value("keyFrameId").toInt() || json_lastChunkInfo.object().value("endGameChunkId").toInt() == -1)
    {
        json_lastChunkInfo = getJsonFromUrl(QString("http://" + m_serveraddress + "/observer-mode/rest/consumer/getLastChunkInfo/" + m_serverid + "/" + m_gameid + "/0/token"));
        if(json_lastChunkInfo.isEmpty())
        {
            emit toLog("[WARN] Spectator server response empty, stop recording : " + m_serverid + "/" + m_gameid);
            break;
        }

        if(json_lastChunkInfo.object().value("keyFrameId").toInt() > lastsavedkeyframeid && json_lastChunkInfo.object().value("keyFrameId").toInt() > 0)
        {
            //Get a keyframe
            if(lastsavedkeyframeid == keyframeid || keyframeid < json_lastChunkInfo.object().value("keyFrameId").toInt() - 2)
            {
                keyframeid += 1;
            }
            bytearray_keyframe = getFileFromUrl(QString("http://" + m_serveraddress + "/observer-mode/rest/consumer/getKeyFrame/" + m_serverid + "/" + m_gameid + "/" + QString::number(keyframeid) + "/0"));
            if(!bytearray_keyframe.isEmpty())
            {
                int nextchunkid = json_lastChunkInfo.object().value("nextChunkId").toInt();
                if(json_lastChunkInfo.object().value("keyFrameId").toInt() != keyframeid)
                {
                    //We're late
                    nextchunkid = 2*keyframeid + json_lastChunkInfo.object().value("endStartupChunkId").toInt();
                }
                list_keyframes.append(Keyframe(keyframeid, bytearray_keyframe, nextchunkid));
                lastsavedkeyframeid = keyframeid;
                emit toLog("Recorder: " + m_serverid + "/" + m_gameid + " : Keyframe " + QString::number(keyframeid) + " " + QString::number(nextchunkid) + " Size: " + QString::number(bytearray_keyframe.size()/1024) + " ko");

                timer3.start(2000);
                loop3.exec();
            }
            else
            {
                emit toLog("Recorder: KeyFrame lost " + m_serverid + "/" + m_gameid + " : " + QString::number(keyframeid));
            }
        }

        if(json_lastChunkInfo.object().value("chunkId").toInt() > lastsavedchunkid && json_lastChunkInfo.object().value("chunkId").toInt() >= json_lastChunkInfo.object().value("startGameChunkId").toInt())
        {
            //Get a chunk
            if(lastsavedchunkid == chunkid || chunkid < json_lastChunkInfo.object().value("chunkId").toInt() - 3)
            {
                chunkid += 1;
            }
            bytearray_chunk = getFileFromUrl(QString("http://" + m_serveraddress + "/observer-mode/rest/consumer/getGameDataChunk/" + m_serverid + "/" + m_gameid + "/" + QString::number(chunkid) + "/0"));
            if(!bytearray_chunk.isEmpty())
            {
                int local_keyframeid = json_lastChunkInfo.object().value("keyFrameId").toInt();
                int duration = json_lastChunkInfo.object().value("duration").toInt();
                if(chunkid != json_lastChunkInfo.object().value("chunkId").toInt())
                {
                    local_keyframeid = floor((chunkid - json_lastChunkInfo.object().value("endStartupChunkId").toInt())/2.0);
                    duration = 30000;
                }
                list_chunks.append(Chunk(chunkid, bytearray_chunk, local_keyframeid, duration));
                lastsavedchunkid = chunkid;
                emit toLog("Recorder: " + m_serverid + "/" + m_gameid + " : Chunk " + QString::number(chunkid) + " " + QString::number(local_keyframeid) + " " + QString::number(duration) + " Size: " + QString::number(bytearray_chunk.size()/1024) + " ko");

                timer3.start(2000);
                loop3.exec();
            }
            else
            {
                if(chunkid < json_lastChunkInfo.object().value("chunkId").toInt())
                {
                    lastsavedchunkid++;
                    emit toLog("Recorder: " + m_serverid + "/" + m_gameid + " : Skip Chunk " + QString::number(chunkid));
                }
                else
                {
                    emit toLog("Recorder: " + m_serverid + "/" + m_gameid + " : Empty Chunk " + QString::number(chunkid));
                }

                if(chunkid > json_lastChunkInfo.object().value("endGameChunkId").toInt() && json_lastChunkInfo.object().value("endGameChunkId").toInt() > 0){
                    emit toLog("[WARN] Chunk out of range : stop recording");
                    break;
                }
            }
        }

        if(startupchunkid <= json_lastChunkInfo.object().value("endStartupChunkId").toInt())
        {
            //Get a primary chunk
            bytearray_chunk = getFileFromUrl(QString("http://" + m_serveraddress + "/observer-mode/rest/consumer/getGameDataChunk/" + m_serverid + "/" + m_gameid + "/" + QString::number(startupchunkid) + "/0"));
            if(!bytearray_chunk.isEmpty())
            {
                list_primarychunks.append(Chunk(startupchunkid, bytearray_chunk, 0));
                emit toLog("Recorder: " + m_serverid + "/" + m_gameid + " : PrimaryChunk " + QString::number(startupchunkid) + " Size: " + QString::number(bytearray_chunk.size()/1024) + " ko");
                startupchunkid += 1;
            }
            else
            {
                emit toLog("Recorder: PrimaryChunk lost " + m_serverid + "/" + m_gameid + " : " + QString::number(startupchunkid));
            }
        }

        //Retry every 16 seconds
        timer.start(16000);
        loop.exec();
    }

    m_startgamechunkid = QString::number(json_gameMetaData.object().value("startGameChunkId").toInt());
    m_endstartupchunkid = QString::number(json_gameMetaData.object().value("endStartupChunkId").toInt());

    QByteArray gamestats = getFileFromUrl(QString("http://" + m_serveraddress + "/observer-mode/rest/consumer/endOfGameStats/" + m_serverid + "/" + m_gameid + "/null"));

    emit toLog("End of recording " + m_serverid + "/" + m_gameid);

    emit toShowmessage("End of recording " + m_serverid + "/" + m_gameid);

    //Save all chunks, infos and keyframes in a file

    QFile file(m_replaydirectory + "/" + m_serverid + "-" + m_gameid + ".lor");

    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream stream(&file);

        stream << "::ORHeader:" << m_serverid << ":" << m_gameid << ":" << m_encryptionkey << ":" << version << ":" << m_endstartupchunkid << ":" << m_startgamechunkid << "::" << endl;

        if(!m_gameinfo.isEmpty())
        {
            stream << "::ORGameInfos:" << m_gameinfo.toJson(QJsonDocument::Compact).toBase64() << "::" << endl;
            emit toLog("Recorder: " + m_serverid + "/" + m_gameid + " Game Infos retrieved");
        }
        else
        {
            emit toLog("[WARN] Recorder: " + m_serverid + "/" + m_gameid + " Game Infos lost");
        }

        if(!gamestats.isEmpty())
        {
            stream << "::ORGameStats:" << gamestats << "::" << endl;
            emit toLog("Recorder: " + m_serverid + "/" + m_gameid + " Game Stats retrieved");
        }
        else
        {
            emit toLog("[WARN] Recorder: " + m_serverid + "/" + m_gameid + " Game Stats lost");
        }

        for(int i = 0; i < list_keyframes.size(); i++)
        {
            stream << "::ORKeyFrame:" << QString::number(list_keyframes.at(i).getId()) << ":";
            stream << QString::number(list_keyframes.at(i).getNextchunkid()) << ":";
            stream << list_keyframes.at(i).getData().toBase64() << "::" << endl;
        }
        for(int i = 0; i < list_primarychunks.size(); i++)
        {
            stream << "::ORChunk:" << QString::number(list_primarychunks.at(i).getId()) << ":" << QString::number(list_primarychunks.at(i).getKeyframeId()) << ":" << QString::number(list_primarychunks.at(i).getDuration()) << ":";
            stream << list_primarychunks.at(i).getData().toBase64() << "::" << endl;
        }
        for(int i = 0; i < list_chunks.size(); i++)
        {
            stream << "::ORChunk:" << QString::number(list_chunks.at(i).getId()) << ":" << QString::number(list_chunks.at(i).getKeyframeId()) << ":" << QString::number(list_chunks.at(i).getDuration()) << ":";
            stream << list_chunks.at(i).getData().toBase64() << "::" << endl;
        }
        stream << "::OREnd::" << endl;
        file.close();

        emit toLog("Replay file created : " + m_replaydirectory + "/" + m_serverid + "-" + m_gameid + ".lor");

        if(list_primarychunks.isEmpty() || list_chunks.isEmpty() || list_keyframes.isEmpty())
        {
            emit toLog("[WARN] Replay : " + m_serverid + "-" + m_gameid + ".lor is incomplete and may not work correctly");
        }
    }
    else
    {
        emit toLog("[ERROR] Error saving replay : cannot open output file : " + m_serverid + "-" + m_gameid + ".lor");
    }

    emit end(m_serverid, m_gameid);
}
