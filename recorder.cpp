#include "recorder.h"
#include "replay.h"

Recorder::Recorder(MainWindow *window, QString serverid, QString serveraddress, QString gameid, QString encryptionkey, QJsonDocument gameinfo, QString replaydirectory)
{
    m_serverid = serverid;
    m_serveraddress = serveraddress;
    m_gameid = gameid;
    m_encryptionkey = encryptionkey;
    m_gameinfo = gameinfo;
    m_replaydirectory = replaydirectory;

    connect(this, SIGNAL(toLog(QString)), window, SLOT(log(QString)));
}

QByteArray Recorder::getFileFromUrl(QString url){
    QNetworkAccessManager local_networkResult;
    QNetworkReply *reply = local_networkResult.get(QNetworkRequest(QUrl(url)));

    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if(reply->error() != QNetworkReply::NoError){
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

    if(reply->error() != QNetworkReply::NoError){
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
    QJsonDocument json_lastChunkInfo = getJsonFromUrl(QString("http://" + m_serveraddress + "/observer-mode/rest/consumer/getLastChunkInfo/" + m_serverid + "/" + m_gameid + "/token"));

    QList<Keyframe> list_keyframes;
    QList<Chunk> list_chunks;
    QList<Chunk> list_primarychunks;

    int keyframeid = json_gameMetaData.object().value("pendingAvailableKeyFrameInfo").toArray().first().toObject().value("id").toInt();
    int lastsavedkeyframeid = -1;
    int chunkid = json_gameMetaData.object().value("pendingAvailableChunkInfo").toArray().first().toObject().value("id").toInt();
    int firstchunkid = chunkid;
    int lastsavedchunkid = -1;
    int startupchunkid = 1;

    QByteArray bytearray_keyframe, bytearray_chunk;

    QTimer timer;
    QEventLoop loop;
    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));

    while(lastsavedchunkid != json_lastChunkInfo.object().value("endGameChunkId").toInt() || lastsavedkeyframeid != json_lastChunkInfo.object().value("keyFrameId").toInt() || json_lastChunkInfo.object().value("endGameChunkId").toInt() == -1){

        json_lastChunkInfo = getJsonFromUrl(QString("http://" + m_serveraddress + "/observer-mode/rest/consumer/getLastChunkInfo/" + m_serverid + "/" + m_gameid + "/0/token"));
        if(json_lastChunkInfo.isEmpty())
        {
            break;
        }

        if(json_lastChunkInfo.object().value("keyFrameId").toInt() > lastsavedkeyframeid){
            //Get a keyframe
            if(lastsavedkeyframeid == keyframeid || keyframeid < json_lastChunkInfo.object().value("keyFrameId").toInt() + 3)
            {
                keyframeid += 1;
            }
            bytearray_keyframe = getFileFromUrl(QString("http://" + m_serveraddress + "/observer-mode/rest/consumer/getKeyFrame/" + m_serverid + "/" + m_gameid + "/" + QString::number(keyframeid) + "/0"));
            if(!bytearray_keyframe.isEmpty())
            {
                list_keyframes.append(Keyframe(keyframeid, bytearray_keyframe, json_lastChunkInfo.object().value("nextChunkId").toInt()));
                lastsavedkeyframeid = keyframeid;
                emit toLog("Recorder: " + m_serverid + "/" + m_gameid + " : Keyframe " + QString::number(keyframeid) + " " + QString::number(json_lastChunkInfo.object().value("nextChunkId").toInt()));
            }
        }

        if(json_lastChunkInfo.object().value("chunkId").toInt() > lastsavedchunkid && json_lastChunkInfo.object().value("chunkId").toInt() >= json_lastChunkInfo.object().value("startGameChunkId").toInt()){
            //Get a chunk
            if(lastsavedchunkid == chunkid || chunkid < json_lastChunkInfo.object().value("chunkId").toInt() + 3)
            {
                chunkid += 1;
            }
            bytearray_chunk = getFileFromUrl(QString("http://" + m_serveraddress + "/observer-mode/rest/consumer/getGameDataChunk/" + m_serverid + "/" + m_gameid + "/" + QString::number(chunkid) + "/0"));
            if(!bytearray_chunk.isEmpty())
            {
                list_chunks.append(Chunk(chunkid, bytearray_chunk, json_lastChunkInfo.object().value("keyFrameId").toInt(), json_lastChunkInfo.object().value("duration").toInt()));
                lastsavedchunkid = chunkid;
                emit toLog("Recorder: " + m_serverid + "/" + m_gameid + " : Chunk " + QString::number(chunkid) + " " + QString::number(json_lastChunkInfo.object().value("keyFrameId").toInt()) + " " + QString::number(json_lastChunkInfo.object().value("duration").toInt()));
            }
        }

        if(startupchunkid <= json_lastChunkInfo.object().value("endStartupChunkId").toInt() && startupchunkid < firstchunkid){
            //Get a primary chunk
            bytearray_chunk = getFileFromUrl(QString("http://" + m_serveraddress + "/observer-mode/rest/consumer/getGameDataChunk/" + m_serverid + "/" + m_gameid + "/" + QString::number(startupchunkid) + "/0"));
            if(!bytearray_chunk.isEmpty()){
                list_primarychunks.append(Chunk(startupchunkid, bytearray_chunk, 0));
                emit toLog("Recorder: " + m_serverid + "/" + m_gameid + " : PrimaryChunk " + QString::number(startupchunkid));
                startupchunkid += 1;
            }
            else{
                emit toLog("Recorder: PrimaryChunk lost " + m_serverid + "/" + m_gameid + " : " + QString::number(startupchunkid));
            }
        }

        //Retry every 10 seconds
        timer.start(10000);
        loop.exec();
    }

    json_gameMetaData = getJsonFromUrl(QString("http://" + m_serveraddress + "/observer-mode/rest/consumer/getGameMetaData/" + m_serverid + "/" + m_gameid + "/token"));

    //TODO: loop until we get good values

    m_startgamechunkid = QString::number(json_gameMetaData.object().value("startGameChunkId").toInt());
    m_endstartupchunkid = QString::number(json_gameMetaData.object().value("endStartupChunkId").toInt());

    QByteArray ba_gamestats = getFileFromUrl(QString("http://" + m_serveraddress + "/observer-mode/rest/consumer/endOfGameStats/" + m_serverid + "/" + m_gameid + "/null"));
    QJsonDocument json_gamestats = QJsonDocument::fromJson(QByteArray::fromBase64(ba_gamestats));

    emit toLog("End of recording : " + m_serverid + "/" + m_gameid);

    //Save all chunks, infos and keyframes in a file

    QFile file(m_replaydirectory + "/" + m_serverid + "-" + m_gameid + ".lor");

    if(file.open(QIODevice::WriteOnly)){
        QTextStream stream(&file);

        stream << "::ORHeader:" << m_serverid << ":" << m_gameid << ":" << m_encryptionkey << ":" << version << ":" << m_endstartupchunkid << ":" << m_startgamechunkid << "::" << endl;

        if(!m_gameinfo.isEmpty()){
            stream << "::ORGameInfos:" << m_gameinfo.toJson(QJsonDocument::Compact).toBase64() << "::" << endl;
        }

        if(!json_gamestats.isEmpty()){
            stream << "::ORGameStats:" << json_gamestats.toJson(QJsonDocument::Compact).toBase64() << "::" << endl;
        }

        for(int i = 0; i < list_keyframes.size(); i++){
            stream << "::ORKeyFrame:" << QString::number(list_keyframes.at(i).getId()) << ":";
            stream << QString::number(list_keyframes.at(i).getNextchunkid()) << ":";
            stream << list_keyframes.at(i).getData().toBase64() << "::" << endl;
        }
        for(int i = 0; i < list_primarychunks.size(); i++){
            stream << "::ORChunk:" << QString::number(list_primarychunks.at(i).getId()) << ":" << QString::number(list_primarychunks.at(i).getKeyframeId()) << ":" << QString::number(list_primarychunks.at(i).getDuration()) << ":";
            stream << list_primarychunks.at(i).getData().toBase64() << "::" << endl;
        }
        for(int i = 0; i < list_chunks.size(); i++){
            stream << "::ORChunk:" << QString::number(list_chunks.at(i).getId()) << ":" << QString::number(list_chunks.at(i).getKeyframeId()) << ":" << QString::number(list_chunks.at(i).getDuration()) << ":";
            stream << list_chunks.at(i).getData().toBase64() << "::" << endl;
        }
        stream << "::OREnd::";
        file.close();

        emit toLog("Replay file created : " + m_replaydirectory + "/" + m_serverid + "-" + m_gameid + ".lor");
    }
    else{
        emit toLog("Error saving replay.");
    }

    emit end(m_serverid, m_gameid);
}
