#include "recorder.h"

Recorder::Recorder(QString serverid, QString serveraddress, QString gameid, QString encryptionkey, QJsonDocument gameinfo, QString replaydirectory)
{
    m_serverid = serverid;
    m_serveraddress = serveraddress;
    m_gameid = gameid;
    m_gameinfo = gameinfo;
    m_replaydirectory = replaydirectory;
    m_startgamechunkid = "0";
    m_endstartupchunkid = "0";

    if(!encryptionkey.isEmpty()){
        m_encryptionkey = encryptionkey;
    }
    else if(!m_gameinfo.isEmpty()){
        m_encryptionkey = m_gameinfo.object().value("observers").toObject().value("encryptionKey").toString();
    }
}

Recorder::~Recorder()
{

}

QByteArray Recorder::getFileFromUrl(QString url){
    QNetworkAccessManager local_networkResult;
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setPriority(QNetworkRequest::LowPriority);

    QNetworkReply *reply = local_networkResult.get(request);
    //reply->setReadBufferSize(qint64(32 * 1024));

    //connect(reply, &QNetworkReply::metaDataChanged, [reply]() { reply->setReadBufferSize(qint64(32 * 1024)); qDebug() << "set speed to" << 32 * 1024; });

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

    /*QByteArray data;
    QTimer timer;
    timer.start(1000);
    QEventLoop loop;
    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));

    while(!reply->isFinished() && reply->error() == QNetworkReply::NoError){
        loop.exec();
        qDebug() << reply->bytesAvailable() << "/" << reply->readBufferSize();
        data.append(reply->read(reply->bytesAvailable()));
        timer.start(500);
    }

    return data;*/
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

void Recorder::launch(){

    emit toLog("Start recording : " + m_serverid + "/" + m_gameid);

    QString version;
    version = getFileFromUrl(QString("http://" + m_serveraddress + "/observer-mode/rest/consumer/version"));

    QJsonDocument json_gameMetaData = getJsonFromUrl(QString("http://" + m_serveraddress + "/observer-mode/rest/consumer/getGameMetaData/" + m_serverid + "/" + m_gameid + "/token"));
    QJsonDocument json_lastChunkInfo = getJsonFromUrl(QString("http://" + m_serveraddress + "/observer-mode/rest/consumer/getLastChunkInfo/" + m_serverid + "/" + m_gameid + "/0/token"));

    QTimer timer;
    QEventLoop loop;
    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    unsigned int counter = 0;

    while(json_gameMetaData.isEmpty() || json_lastChunkInfo.isEmpty() || json_lastChunkInfo.object().value("chunkId").toInt() == 0 || json_lastChunkInfo.object().value("endStartupChunkId").toInt() < 1)
    {
        counter++;
        if(counter > 30)
        {
            emit toLog("[ERROR] No valid response from spectator server, aborting recorder : " + m_serverid + "/" + m_gameid);

            emit end(m_serverid, m_gameid);
            emit finished();

            return;
        }
        json_gameMetaData = getJsonFromUrl(QString("http://" + m_serveraddress + "/observer-mode/rest/consumer/getGameMetaData/" + m_serverid + "/" + m_gameid + "/token"));
        json_lastChunkInfo = getJsonFromUrl(QString("http://" + m_serveraddress + "/observer-mode/rest/consumer/getLastChunkInfo/" + m_serverid + "/" + m_gameid + "/0/token"));

        //Retry every 20 seconds
        timer.start(20000);
        loop.exec();
    }

    int endstartupchunkid = json_lastChunkInfo.object().value("endStartupChunkId").toInt();

    QList<Keyframe> list_keyframes;
    QList<Chunk> list_chunks;
    QList<Chunk> list_primarychunks;

    QByteArray bytearray_keyframe, bytearray_chunk;

    QJsonArray array_currentKeyframes = json_gameMetaData.object().value("pendingAvailableKeyFrameInfo").toArray();
    QJsonArray array_currentChunks = json_gameMetaData.object().value("pendingAvailableChunkInfo").toArray();

    QList<int> list_remainingChunks;
    QList<int> list_remainingKeyframes;

    QList<int> list_retrievedChunks;
    QList<int> list_retrievedKeyframes;

    int firstchunk = array_currentChunks.first().toObject().value("id").toInt();
    for(int i = firstchunk + 1; i <= json_lastChunkInfo.object().value("chunkId").toInt(); i++){
        if(i > endstartupchunkid && !list_remainingChunks.contains(i)){
            list_remainingChunks.append(i);
        }
    }

    int firstkeyframe = array_currentKeyframes.first().toObject().value("id").toInt();
    for(int i = firstkeyframe + 1; i <= json_lastChunkInfo.object().value("keyFrameId").toInt(); i++){
        if(!list_remainingKeyframes.contains(i)){
            list_remainingKeyframes.append(i);
        }
    }

    for(int i = 1; i <= endstartupchunkid; i++){
        bytearray_chunk = getFileFromUrl(QString("http://" + m_serveraddress + "/observer-mode/rest/consumer/getGameDataChunk/" + m_serverid + "/" + m_gameid + "/" + QString::number(i) + "/0"));
        if(!bytearray_chunk.isEmpty())
        {
            list_primarychunks.append(Chunk(i, bytearray_chunk, 0));

            emit toLog("Recorder: " + m_serverid + "/" + m_gameid + " : PrimaryChunk " + QString::number(i) + " Size: " + QString::number(bytearray_chunk.size()/1024) + " ko");
        }
        else
        {
            emit toLog("Recorder: PrimaryChunk not found " + m_serverid + "/" + m_gameid + " : " + QString::number(i));
        }
    }

    int lastsavedchunkid = firstchunk;
    int lastsavedkeyframeid = firstkeyframe;

    while(lastsavedchunkid != json_lastChunkInfo.object().value("endGameChunkId").toInt() || lastsavedkeyframeid != json_lastChunkInfo.object().value("keyFrameId").toInt() || json_lastChunkInfo.object().value("endGameChunkId").toInt() == -1)
    {
        json_lastChunkInfo = getJsonFromUrl(QString("http://" + m_serveraddress + "/observer-mode/rest/consumer/getLastChunkInfo/" + m_serverid + "/" + m_gameid + "/0/token"));
        if(json_lastChunkInfo.isEmpty())
        {
            emit toLog("[WARN] Spectator server response empty, stop recording : " + m_serverid + "/" + m_gameid);
            break;
        }

        int currentkeyframeid = json_lastChunkInfo.object().value("keyFrameId").toInt();
        int currentchunkid = json_lastChunkInfo.object().value("chunkId").toInt();

        if(!list_retrievedKeyframes.contains(currentkeyframeid) && !list_remainingKeyframes.contains(currentkeyframeid))
        {
            for(int i = lastsavedkeyframeid + 1; i <= currentkeyframeid; i++){
                if(list_remainingKeyframes.indexOf(i) == -1){
                    list_remainingKeyframes.append(i);
                }
            }
        }

        QList<int> list_remainingKeyframes_temp;

        for(int i = 0; i < list_remainingKeyframes.size(); i++){
            bytearray_keyframe = getFileFromUrl(QString("http://" + m_serveraddress + "/observer-mode/rest/consumer/getKeyFrame/" + m_serverid + "/" + m_gameid + "/" + QString::number(list_remainingKeyframes.at(i)) + "/0"));

            if(!bytearray_keyframe.isEmpty()){
                int nextchunkid = json_lastChunkInfo.object().value("nextChunkId").toInt();

                if(json_lastChunkInfo.object().value("keyFrameId").toInt() != list_remainingKeyframes.at(i))
                {
                    //We're late
                    nextchunkid = 2*list_remainingKeyframes.at(i) + endstartupchunkid;
                }

                list_keyframes.append(Keyframe(list_remainingKeyframes.at(i), bytearray_keyframe, nextchunkid));
                list_retrievedKeyframes.append(list_remainingKeyframes.at(i));
                lastsavedkeyframeid = list_retrievedKeyframes.last();

                emit toLog("Recorder: " + m_serverid + "/" + m_gameid + " : Keyframe " + QString::number(list_remainingKeyframes.at(i)) + " " + QString::number(nextchunkid) + " Size: " + QString::number(bytearray_keyframe.size()/1024) + " ko");

                timer.start(500);
                loop.exec();
            }
            else if(list_remainingKeyframes.at(i) + 5 >= currentkeyframeid){
                list_remainingKeyframes_temp.append(list_remainingKeyframes.at(i));

                emit toLog("Recorder: " + m_serverid + "/" + m_gameid + " : Keyframe " + QString::number(list_remainingKeyframes.at(i)) + " not found");
            }
            else{
                emit toLog("Recorder: " + m_serverid + "/" + m_gameid + " : Keyframe " + QString::number(list_remainingKeyframes.at(i)) + " skiped");
            }
        }

        list_remainingKeyframes = list_remainingKeyframes_temp;

        timer.start(2000);
        loop.exec();

        if(!list_retrievedChunks.contains(currentchunkid) && !list_remainingChunks.contains(currentchunkid))
        {
            for(int i = lastsavedchunkid + 1; i <= currentchunkid; i++){
                if(list_remainingChunks.indexOf(i) == -1){
                    list_remainingChunks.append(i);
                }
            }
        }

        QList<int> list_remainingChunks_temp;

        for(int i = 0; i < list_remainingChunks.size(); i++){
            bytearray_chunk = getFileFromUrl(QString("http://" + m_serveraddress + "/observer-mode/rest/consumer/getGameDataChunk/" + m_serverid + "/" + m_gameid + "/" + QString::number(list_remainingChunks.at(i)) + "/0"));

            if(!bytearray_chunk.isEmpty())
            {
                int duration = json_lastChunkInfo.object().value("duration").toInt();
                int local_keyframeid = currentkeyframeid;

                if(list_remainingChunks.at(i) != json_lastChunkInfo.object().value("chunkId").toInt())
                {
                    local_keyframeid = floor((list_remainingChunks.at(i) - json_lastChunkInfo.object().value("endStartupChunkId").toInt())/2.0);
                    duration = 30000;
                }

                list_chunks.append(Chunk(list_remainingChunks.at(i), bytearray_chunk, local_keyframeid, duration));
                list_retrievedChunks.append(list_remainingChunks.at(i));
                lastsavedchunkid = list_retrievedChunks.last();

                emit toLog("Recorder: " + m_serverid + "/" + m_gameid + " : Chunk " + QString::number(list_remainingChunks.at(i)) + " " + QString::number(local_keyframeid) + " " + QString::number(duration) + " Size: " + QString::number(bytearray_chunk.size()/1024) + " ko");

                timer.start(500);
                loop.exec();
            }
            else if(list_remainingChunks.at(i) + 10 >= currentchunkid)
            {
                list_remainingChunks_temp.append(list_remainingChunks.at(i));

                emit toLog("Recorder: " + m_serverid + "/" + m_gameid + " : Chunk not found " + QString::number(list_remainingChunks.at(i)));
            }
            else{
                emit toLog("Recorder: " + m_serverid + "/" + m_gameid + " : Chunk " + QString::number(list_remainingChunks.at(i)) + " skiped");
            }
        }

        list_remainingChunks = list_remainingChunks_temp;

        timer.start(json_lastChunkInfo.object().value("nextAvailableChunk").toInt() + 2000);
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

    emit finished();
}
