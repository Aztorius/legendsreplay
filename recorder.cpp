#include "recorder.h"

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

    QList<QByteArray> list_bytearray_keyframes;
    QList<QByteArray> list_bytearray_gamedatachunks;

    int keyframeid = json_gameMetaData.object().value("pendingAvailableKeyFrameInfo").toArray().first().toObject().value("id").toInt();
    int lastsavedkeyframeid = -1;
    int chunkid = json_gameMetaData.object().value("pendingAvailableChunkInfo").toArray().first().toObject().value("id").toInt();
    int lastsavedchunkid = -1;

    QByteArray bytearray_keyframe, bytearray_chunk;

    QTimer timer;
    QEventLoop loop;
    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));

    while(lastsavedchunkid != json_gameMetaData.object().value("endGameChunkId").toInt() || lastsavedkeyframeid != json_gameMetaData.object().value("endGameKeyFrameId").toInt() || json_gameMetaData.object().value("endGameChunkId").toInt() == -1){
        if(json_gameMetaData.object().value("pendingAvailableKeyFrameInfo").toArray().last().toObject().value("id").toInt() > lastsavedkeyframeid){
            //Get a keyframe
            if(lastsavedkeyframeid == keyframeid || keyframeid < json_gameMetaData.object().value("pendingAvailableKeyFrameInfo").toArray().first().toObject().value("id").toInt()){
                keyframeid += 1;
            }
            bytearray_keyframe = getFileFromUrl(QString("http://" + m_serveraddress + "/observer-mode/rest/consumer/getKeyFrame/" + m_serverid + "/" + m_gameid + "/" + QString::number(keyframeid) + "/0"));
            if(!bytearray_keyframe.isEmpty()){
                list_bytearray_keyframes.append(bytearray_keyframe);
                lastsavedkeyframeid = keyframeid;
                emit toLog("Keyframe-" + m_serverid + "/" + m_gameid + " : " + QString::number(keyframeid));
            }
        }

        if(json_gameMetaData.object().value("pendingAvailableChunkInfo").toArray().last().toObject().value("id").toInt() > lastsavedchunkid){
            //Get a chunk
            if(lastsavedchunkid == chunkid || chunkid < json_gameMetaData.object().value("pendingAvailableChunkInfo").toArray().first().toObject().value("id").toInt()){
                chunkid += 1;
            }
            bytearray_chunk = getFileFromUrl(QString("http://" + m_serveraddress + "/observer-mode/rest/consumer/getGameDataChunk/" + m_serverid + "/" + m_gameid + "/" + QString::number(chunkid) + "/0"));
            if(!bytearray_chunk.isEmpty()){
                list_bytearray_gamedatachunks.append(bytearray_chunk);
                lastsavedchunkid = chunkid;
                emit toLog("Chunk-" + m_serverid + "/" + m_gameid + " : " + QString::number(chunkid));
            }
        }

        json_gameMetaData = getJsonFromUrl(QString("http://" + m_serveraddress + "/observer-mode/rest/consumer/getGameMetaData/" + m_serverid + "/" + m_gameid + "/token"));
        if(json_gameMetaData.isEmpty()){
            break;
        }

        //Retry every 5 seconds
        timer.start(5000);
        loop.exec();
    }

    emit toLog("End of recording : " + m_serverid + "/" + m_gameid);

    //TODO : Emit signal end of recording

    //Save all chunks, infos and keyframes in a file

    QFile file(m_replaydirectory + "/" + m_serverid + "-" + m_gameid + ".lor");

    if(file.open(QIODevice::WriteOnly)){
        QTextStream stream(&file);

        stream << "::OpenReplayInfos:" << m_serverid << ":" << m_gameid << ":" << m_encryptionkey << ":" << version << "::" << endl;

        if(!m_gameinfo.isEmpty()){
            stream << "::OpenReplayGameInfos::" << m_gameinfo.toJson(QJsonDocument::Compact).toBase64() << endl;
        }

        int first_keyframeid = json_gameMetaData.object().value("endGameKeyFrameId").toInt() - list_bytearray_keyframes.size() + 1;
        int first_chunkid = json_gameMetaData.object().value("endGameChunkId").toInt() - list_bytearray_gamedatachunks.size() + 1;

        for(int i = 0; i < list_bytearray_keyframes.size(); i++){
            stream << "::OpenReplayKeyFrame:" << QString::number(first_keyframeid + i) << "::";
            stream << list_bytearray_keyframes.at(i).toBase64() << endl;
        }
        for(int i = 0; i < list_bytearray_gamedatachunks.size(); i++){
            stream << "::OpenReplayChunk:" << QString::number(first_chunkid + i) << "::";
            stream << list_bytearray_gamedatachunks.at(i).toBase64() << endl;
        }
        stream << "::OpenReplayEnd::";
        file.close();

        emit toLog("Replay file created : " + m_replaydirectory + "/" + m_serverid + "-" + m_gameid + ".lor");
    }
    else{
        emit toLog("Error saving replay.");
    }

    emit end(m_serverid, m_gameid);
}
