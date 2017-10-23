/* ****************************************************************************
 *
 * Copyright 2016-2017 William Bonnaventure
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

#include "repairtooldialog.h"
#include "ui_repairtooldialog.h"

RepairToolDialog::RepairToolDialog(QWidget *parent) :
    QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    ui(new Ui::RepairToolDialog)
{
    ui->setupUi(this);

    m_checked = false;

    ui->checkBox_playable->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->checkBox_playable->setFocusPolicy(Qt::NoFocus);

    ui->checkBox_primarychunks->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->checkBox_primarychunks->setFocusPolicy(Qt::NoFocus);

    ui->checkBox_endofgamestats->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->checkBox_endofgamestats->setFocusPolicy(Qt::NoFocus);

    ui->checkBox_gameinfos->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->checkBox_gameinfos->setFocusPolicy(Qt::NoFocus);

    connect(ui->pushButton_check, SIGNAL(clicked()), this, SLOT(check()));
    connect(ui->pushButton_openfile, SIGNAL(clicked()), this, SLOT(openFile()));
    connect(ui->pushButton_repairandsave, SIGNAL(clicked()), this, SLOT(repair()));
}

RepairToolDialog::~RepairToolDialog()
{
    delete ui;
}

void RepairToolDialog::load(Replay replay){
    m_replay = replay;
    check();
}

void RepairToolDialog::check(){
    ui->listWidget_chunks->clear();
    ui->listWidget_keyframes->clear();

    if(m_replay.getFilepath().isEmpty()){
        ui->lineEdit_filename->clear();
        ui->lineEdit_serverversion->clear();
        ui->lineEdit_platformid->clear();
        ui->lineEdit_gameid->clear();
        ui->lineEdit_encryptionkey->clear();

        ui->spinBox_endstartupchunkid->setValue(0);
        ui->spinBox_startgamechunkid->setValue(0);

        ui->checkBox_primarychunks->setChecked(false);
        ui->checkBox_endofgamestats->setChecked(false);
        ui->checkBox_gameinfos->setChecked(false);
        ui->checkBox_playable->setChecked(false);

        m_checked = false;

        return;
    }

    ui->progressBar->setValue(0);

    bool isPlayable = true;

    ui->lineEdit_filename->setText(m_replay.getFilepath());
    ui->lineEdit_serverversion->setText(m_replay.getServerVersion());
    ui->lineEdit_platformid->setText(m_replay.getPlatformId());
    ui->lineEdit_gameid->setText(m_replay.getGameId());
    ui->lineEdit_encryptionkey->setText(m_replay.getEncryptionkey());

    if(m_replay.getFilepath().isEmpty() || m_replay.getServerVersion().isEmpty() || m_replay.getPlatformId().isEmpty() || m_replay.getGameId().isEmpty() || m_replay.getEncryptionkey().isEmpty()){
        isPlayable = false;
    }

    ui->spinBox_endstartupchunkid->setValue(m_replay.getEndStartupChunkId().toInt());
    ui->spinBox_startgamechunkid->setValue(m_replay.getStartGameChunkId().toInt());

    if(m_replay.getEndStartupChunkId().isEmpty() || m_replay.getStartGameChunkId().isEmpty()){
        isPlayable = false;
    }

    ui->checkBox_gameinfos->setChecked(!m_replay.getGameinfos().isEmpty());
    ui->checkBox_endofgamestats->setChecked(!m_replay.getEndOfGameStats().isEmpty());

    bool containsAllPrimaryChunks = true;

    for(int i = 1; i <= m_replay.getEndStartupChunkId().toInt(); i++){
        if(m_replay.getPrimaryChunk(i).isEmpty()){
            containsAllPrimaryChunks = false;
            break;
        }
    }

    ui->checkBox_primarychunks->setChecked(containsAllPrimaryChunks);

    if(!containsAllPrimaryChunks){
        isPlayable = false;
    }

    //Chunks missing
    foreach(Keyframe currentKeyframe, m_replay.getKeyFrames()){
        if(m_replay.getChunk(currentKeyframe.getNextchunkid()).isEmpty()){
            ui->listWidget_chunks->addItem(tr("Chunk ") + QString::number(currentKeyframe.getNextchunkid()));
        }

        if(m_replay.getChunk(currentKeyframe.getNextchunkid() + 1).isEmpty()){
            ui->listWidget_chunks->addItem(tr("Chunk ") + QString::number(currentKeyframe.getNextchunkid() + 1));
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

    ui->checkBox_playable->setChecked(isPlayable);

    ui->progressBar->setValue(100);

    m_checked = true;

    if(!isPlayable){
        QMessageBox::warning(this, tr("LegendsReplay"), tr("The game is not currently playable.\nA repair is needed."));
    }
}

void RepairToolDialog::repair(){
    if(!m_checked){
        check();
    }

    if(ui->checkBox_playable->checkState() == Qt::Checked && QMessageBox::question(this, tr("LegendsReplay"), tr("This replay is detected as readable.\nDo you really want to repair it ?")) == QMessageBox::No){
        return;
    }

    ui->progressBar->setValue(0);

    if(ui->checkBox_primarychunks->checkState() == Qt::Unchecked){
        QMessageBox::warning(this, tr("LegendsReplay"), tr("This replay cannot be repaired.\nRepair is impossible without all PrimaryChunks."));
        return;
    }

    if(m_replay.getChunks().size() <= 1 || m_replay.getKeyFrames().isEmpty()){
        QMessageBox::warning(this, tr("LegendsReplay"), tr("This replay cannot be repaired.\nRepair is impossible without at least some Chunks and Keyframes."));
        return;
    }

    if(ui->lineEdit_gameid->text().isEmpty() || ui->lineEdit_gameid->text().toLongLong() <= 0){
        QMessageBox::warning(this, tr("LegendsReplay"), tr("A valid game id number is needed."));
        return;
    }

    if(ui->lineEdit_encryptionkey->text().isEmpty()){
        QMessageBox::warning(this, tr("LegendsReplay"), tr("A valid encryption key is needed."));
        return;
    }

    if(ui->lineEdit_platformid->text().isEmpty()){
        QMessageBox::warning(this, tr("LegendsReplay"), tr("A valid platform id is needed."));
        return;
    }

    if(ui->spinBox_endstartupchunkid->value() <= 0){
        QMessageBox::warning(this, tr("LegendsReplay"), tr("A valid endStartupChunkId number is needed."));
        return;
    }

    if(ui->spinBox_startgamechunkid->value() <= 0){
        QMessageBox::warning(this, tr("LegendsReplay"), tr("A valid startGameChunkId number is needed."));
        return;
    }

    ui->progressBar->setValue(20);

    if(!ui->lineEdit_serverversion->text().isEmpty()){
        m_replay.setServerVersion(ui->lineEdit_serverversion->text());
    }

    m_replay.setGameId(ui->lineEdit_gameid->text());
    m_replay.setPlatformId(ui->lineEdit_platformid->text());
    m_replay.setEncryptionKey(ui->lineEdit_encryptionkey->text());

    m_replay.setEndStartupChunkId(ui->spinBox_endstartupchunkid->value());
    m_replay.setStartGameChunkId(ui->spinBox_startgamechunkid->value());

    ui->progressBar->setValue(40);

    //Remove unused KeyFrames
    while(m_replay.getKeyFrames().size() > 0 && (m_replay.getChunk(m_replay.getKeyFrames().first().getNextchunkid()).getId() == 0 || m_replay.getChunk(m_replay.getKeyFrames().first().getNextchunkid() + 1).getId() == 0)){
        m_replay.removeKeyFrame(m_replay.getKeyFrames().first().getId());
    }

    while(m_replay.getKeyFrames().size() > 0 && (m_replay.getChunk(m_replay.getKeyFrames().last().getNextchunkid()).getId() == 0 || m_replay.getChunk(m_replay.getKeyFrames().last().getNextchunkid() + 1).getId() == 0)){
        m_replay.removeKeyFrame(m_replay.getKeyFrames().last().getId());
    }

    QList<Keyframe> finalkeyframes;
    while(!m_replay.getKeyFrames().isEmpty() && m_replay.getChunk(m_replay.getKeyFrames().first().getNextchunkid()).getId() > 0 && m_replay.getChunk(m_replay.getKeyFrames().first().getNextchunkid() + 1).getId() > 0){
        finalkeyframes.append(m_replay.getKeyFrames().first());
        m_replay.removeKeyFrame(m_replay.getKeyFrames().first().getId());
    }

    m_replay.setKeyFrames(finalkeyframes);

    if(finalkeyframes.isEmpty()){
        QMessageBox::warning(this, tr("LegendsReplay"), tr("Repair failed.\nNo valid keyframes left after repairing."));
        ui->progressBar->setValue(0);
        return;
    }

    ui->progressBar->setValue(60);

    //Remove unused Chunks
    while(m_replay.getChunks().size() > 0 && m_replay.getChunks().first().getId() >= m_replay.getStartGameChunkId().toInt() && m_replay.getKeyFrame(m_replay.getChunks().first().getKeyframeId()).isEmpty()){
        m_replay.removeChunk(m_replay.getChunks().first().getId());
    }

    while(m_replay.getChunks().size() > 0 && m_replay.getKeyFrame(m_replay.getChunks().last().getKeyframeId()).isEmpty()){
        m_replay.removeChunk(m_replay.getChunks().last().getId());
    }

    if(m_replay.getChunks().isEmpty()){
        QMessageBox::warning(this, tr("LegendsReplay"), tr("Repair failed.\nNo valid chunks left after repairing."));
        ui->progressBar->setValue(0);
        return;
    }

    ui->progressBar->setValue(80);

    QString filepath = QFileDialog::getSaveFileName(this, tr("Save as"), m_replay.getFilepath());
    if(filepath.isEmpty()){
        ui->progressBar->setValue(0);
        return;
    }

    if(!m_replay.saveAs(filepath)){
        QMessageBox::warning(this, tr("LegendsReplay"), tr("Cannot open the output file."));
        ui->progressBar->setValue(0);
        return;
    }

    ui->progressBar->setValue(100);
}

void RepairToolDialog::openFile()
{
    QString directory = m_replay.getFilepath();

    if (directory.isEmpty()) {
        directory = m_directory;
    }

    QString filepath = QFileDialog::getOpenFileName(this, tr("Open a replay file"), directory);

    if(!filepath.isEmpty()){
        load(Replay(filepath));
    }
}

void RepairToolDialog::setDirectory(QString replays_directory)
{
    m_directory = replays_directory;
}
