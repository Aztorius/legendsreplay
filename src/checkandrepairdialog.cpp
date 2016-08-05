#include "checkandrepairdialog.h"
#include "ui_checkandrepairdialog.h"

CheckAndRepairDialog::CheckAndRepairDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CheckAndRepairDialog)
{
    ui->setupUi(this);

    connect(ui->pushButton_check, SIGNAL(clicked()), this, SLOT(check()));
}

CheckAndRepairDialog::~CheckAndRepairDialog()
{
    delete ui;
}

void CheckAndRepairDialog::load(Replay replay){
    m_replay = replay;
    check();
}

void CheckAndRepairDialog::check(){
    if(m_replay.getFilepath().isEmpty()){
        return;
    }

    ui->progressBar->setValue(0);

    ui->lineEdit_filename->setText(m_replay.getFilepath());
    ui->lineEdit_serverversion->setText(m_replay.getServerversion());
    ui->lineEdit_platformid->setText(m_replay.getPlatformId());
    ui->lineEdit_gameid->setText(m_replay.getGameid());
    ui->lineEdit_encryptionkey->setText(m_replay.getEncryptionkey());

    ui->spinBox_endstartupchunkid->setValue(m_replay.getEndstartupchunkid().toInt());
    ui->spinBox_startgamechunkid->setValue(m_replay.getStartgamechunkid().toInt());

    ui->checkBox_gameinfos->setChecked(!m_replay.getGameinfos().isEmpty());
    ui->checkBox_endofgamestats->setChecked(!m_replay.getEndOfGameStats().isEmpty());

    bool containsAllPrimaryChunks = true;

    for(int i = 1; i <= m_replay.getEndstartupchunkid().toInt(); i++){
        if(m_replay.getPrimaryChunk(i).isEmpty()){
            containsAllPrimaryChunks = false;
            break;
        }
    }

    ui->checkBox_primarychunks->setChecked(containsAllPrimaryChunks);

    //Chunks missing
    foreach(Keyframe currentKeyframe, m_replay.getKeyFrames()){
        if(m_replay.getChunk(currentKeyframe.getNextchunkid()).isEmpty()){
            ui->listWidget_chunks->addItem("Chunk " + QString::number(currentKeyframe.getNextchunkid()));
        }

        if(m_replay.getChunk(currentKeyframe.getNextchunkid() + 1).isEmpty()){
            ui->listWidget_chunks->addItem("Chunk " + QString::number(currentKeyframe.getNextchunkid() + 1));
        }
    }

    //Keyframes missing
    foreach(Chunk currentChunk, m_replay.getChunks()){
        if(m_replay.getKeyFrame(currentChunk.getKeyframeId()).isEmpty() && currentChunk.getKeyframeId() > 0){
            if(ui->listWidget_keyframes->findItems("Keyframe " + QString::number(currentChunk.getKeyframeId()), Qt::MatchExactly).isEmpty()){
                ui->listWidget_keyframes->addItem("Keyframe " + QString::number(currentChunk.getKeyframeId()));
            }
        }
    }

    ui->progressBar->setValue(100);
}

void CheckAndRepairDialog::repair(){

}
