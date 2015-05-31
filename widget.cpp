#include "widget.h"
#include "ui_widget.h"
#include <QDebug>

#define DEBUG

#ifdef DEBUG
    #define MPLAYER_PATH "/usr/bin/mplayer"
#else
    #define MPLAYER_PATH "/bin/mplayer"
#endif

#define LISTPATH "playlist.list"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    m_isplaying=false;
    m_mplayerProcess=new QProcess(this);
    connect(m_mplayerProcess, SIGNAL(readyReadStandardOutput()),        // 捕捉播放器进程输出
            this, SLOT(catchOutput()));
    connect(m_mplayerProcess, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(mplayerEnded(int, QProcess::ExitStatus)));
    connect(ui->m_playlist, SIGNAL(dragEvent(int,int)), this, SLOT(handleDrag(int,int)));

    connect(ui->m_playSlider,SIGNAL(send_pos(double)),this,SLOT(receive_pos(double)));

    m_timeTimer = new QTimer(this);
    connect(m_timeTimer, SIGNAL(timeout()), this, SLOT(getTimepos()));
    ui->m_playSlider->setEnabled(false);

    // 设置音量槽数值
    ui->m_volumeSlider->setMinimum(0);
    ui->m_volumeSlider->setMaximum(100);
    ui->m_volumeSlider->setValue(80);

   playpos=-1;      // 当前播放的位置
    playmode=SEQUENCE;  // 默认为顺序播放
    nextfile = true;       // 结束时选择下一曲
    direction = DOWN;       // 默认向下播放

    QFile list(LISTPATH);
    if(list.open(QIODevice::ReadOnly)){
        QDataStream in(&list);
        int i=0;
        QStringList tmp;
        in>>tmp;
        if(in.status()==QDataStream::Ok){
            m_filelist<<tmp;
        }
        if(m_filelist.length()!=0){
            playpos=0;
        }
        while(i<m_filelist.length()){
            addStr(m_filelist[i]);
            i++;
        }
        list.close();
    }

}

Widget::~Widget()
{
    if(m_filelist.length()!=0){     // 将播放列表写回文件
        QFile list(LISTPATH);
        if(list.open(QIODevice::WriteOnly)){
            QDataStream  out(&list);
            out<<m_filelist;
        }
    }
    delete m_mplayerProcess;
    delete ui;
}

void Widget::on_m_playButton_clicked()
{
    if(m_isplaying){
        pauseMPlayer();
    }else if(m_filelist.length()!=0){           // 如果播放列表不为0
        playpos = ui->m_playlist->currentRow();
        if(playpos == -1)                              // currentRow会在未选中任何项返回-1，播放器第一次播放会出现这种情况，这时从第一首开始播放
            playpos=0;
        startMPlayer();
    }else{
        on_m_openButton_clicked();
    }
}

bool Widget::startMPlayer(int pos){
    if(m_isplaying)
        return true;
    QStringList args;
    args.clear();
    args << "-slave"<< "-quiet"<< m_filelist[playpos];

    m_mplayerProcess->setProcessChannelMode(QProcess::MergedChannels);          // 设置混合模式，使得播放器的进程的标准输出重定向到父进程的标准输出
    m_mplayerProcess->start(MPLAYER_PATH, args);
    if(!m_mplayerProcess->waitForStarted((3000))){
        return false;
    }

    m_isplaying=true;
    m_ispause=false;
    ui->m_playButton->setIcon(QIcon(":/images/pause.png"));

    on_m_volumeSlider_valueChanged(ui->m_volumeSlider->sliderPosition());   // 获取当前音量值
    nextfile = true;

    m_timeTimer->start(1000);
    ui->m_playSlider->setEnabled(true);
    ui->m_playlist->setCurrentRow(playpos);         //  默认选中当前播放曲目

    return true;
}

bool Widget::pauseMPlayer(){
    if(!m_isplaying)
        return true;

   m_mplayerProcess->write("pause\n");          // 第一次写入pause使播放器暂停，第二次写入pause恢复播放
    m_ispause = !m_ispause;
    if (m_ispause) {
        ui->m_playButton->setIcon(QIcon(":/images/play.png"));
        m_timeTimer->stop();
    } else {
        ui->m_playButton->setIcon(QIcon(":/images/pause.png"));
        m_timeTimer->start(1000);
        ui->m_playSlider->setEnabled(true);
    }

    return true;
}

bool Widget::stopMPlayer(){
    if(!m_isplaying){
        return true;
    }
    m_mplayerProcess->write("quit\n");
    nextfile = false;       // 正常退出，不用继续播放
    char buf[20];
    int p=0;
    sprintf(buf, "%02d:%02d", (int)p/60, (int)p%60);
    ui->m_curLabel->setText(buf);
    ui->m_totalLabel->setText(buf);
    ui->m_playSlider->setValue(0);
    return true;
}

void Widget::catchOutput(){

    char buf[10];
    while(m_mplayerProcess->canReadLine()){
        QByteArray buffer(m_mplayerProcess->readLine());
        if(buffer.startsWith("ANS_LENGTH")){
            buffer.remove(0, 11);           // 去掉ANS_LENGTH
            buffer.replace(QByteArray("'"), QByteArray(""));
            buffer.replace(QByteArray(" "), QByteArray(""));
            buffer.replace(QByteArray("\n"), QByteArray(""));
            buffer.replace(QByteArray("\r"), QByteArray(""));
            maxTime = buffer.toFloat();
            ui->m_playSlider->setMaximum(maxTime);
            sprintf(buf, "%02d:%02d",(int)maxTime/60, (int)maxTime%60);
            ui->m_totalLabel->setText(buf);
        }else if(buffer.startsWith("ANS_TIME_POSITION")){
            buffer.remove(0, 18);
            buffer.replace(QByteArray("'"), QByteArray(""));
            buffer.replace(QByteArray(" "), QByteArray(""));
            buffer.replace(QByteArray("\n"), QByteArray(""));
            buffer.replace(QByteArray("\r"), QByteArray(""));
            float curtime;
            curtime =  buffer.toFloat();
            ui->m_playSlider->setValue(curtime);
            sprintf(buf, "%02d:%02d", (int)curtime/60, (int)curtime%60);
            ui->m_curLabel->setText(buf);
        }else{
        }
    }
}

void Widget::mplayerEnded(int exitCode, QProcess::ExitStatus exitStatus){
    m_isplaying = false;
    m_ispause = false;
    ui->m_playButton->setIcon(QIcon(":/images/play.png"));
    if(nextfile){       // 如果有意继续播放
        int nextpos = selectNext();
        if(nextpos != -1){
            playpos = nextpos;
            startMPlayer();
         }
    }
}

// 添加歌曲到播放列表尾部，并设置为当前播放曲目
void Widget::on_m_openButton_clicked()
{
    QFileDialog *fileDialog = new QFileDialog(this);
    fileDialog->setWindowTitle("Open Media File");
#ifndef DEBUG
    fileDialog->setDirectory("/sdcard/music");
    fileDialog->setFixedHeight(LCDHEIGHT);
    fileDialog->setFixedWidth(LCDWIDTH);
#endif
    if(fileDialog->exec() == QDialog::Accepted){
        QString path = fileDialog->selectedFiles()[0];
        // 防止重复添加
        if(isDuplicating(path)) return;
        m_filelist << path;
       playpos = m_filelist.length()-1;
       addStr(path);        // 将歌曲加入播放列表
    }
}

void Widget::on_m_stopButton_clicked()
{
    stopMPlayer();
}

void Widget::on_m_volumeSlider_valueChanged(int value)
{
    char buf[20];
    sprintf(buf,"volume %d 1 \n", value);
    m_mplayerProcess->write(buf);
}

void Widget::on_m_deleteButton_clicked()
{
    QListWidgetItem* pitem = ui->m_playlist->currentItem();
    if(pitem == NULL) return;       // 未选中或者已为空
    QStringList list = m_filelist.filter(pitem->text());        // 返回包含text的字符串列表
    m_filelist.removeOne(list[0]);
    delete pitem;
}

bool Widget::isDuplicating(QString &path){
    m_filelist.append(path);
    if(m_filelist.removeDuplicates()==0){
        m_filelist.removeAt(m_filelist.length()-1);
        return false;
    }
    else{
        return true;
    }
}

void Widget::on_m_loopButton_clicked()
{
    switch(playmode){
    case RANDOM:
        ui->m_randomButton->setIcon(QIcon(":/images/random.png"));
        /* fall through */
    case CYCLE:
        ui->m_cycleButton->setIcon(QIcon(":/images/cycle.png"));
        /* fall through */
    case SEQUENCE:
        ui->m_loopButton->setIcon(QIcon(":/images/loop(valid).png"));
        playmode=LOOP;
        break;
    case LOOP:
        ui->m_loopButton->setIcon(QIcon(":/images/loop.png"));
        playmode=SEQUENCE;
        break;
    }
}

void Widget::on_m_randomButton_clicked()
{
    switch(playmode){
    case LOOP:
        ui->m_loopButton->setIcon(QIcon(":/images/loop.png"));
        /* fall through */
    case CYCLE:
        ui->m_cycleButton->setIcon(QIcon(":/images/cycle.png"));
        /* fall through */
    case SEQUENCE:
        ui->m_randomButton->setIcon(QIcon(":/images/random(valid).png"));
        playmode=RANDOM;
        break;
    case RANDOM:
        ui->m_randomButton->setIcon(QIcon(":/images/random.png"));
        playmode=SEQUENCE;
        break;
    }
}

// 根据播放模式选择下一曲
int Widget::selectNext(){
    int tmppos = playpos;
    if(playmode==RANDOM){
        qsrand(time(NULL));
        tmppos = qrand() % m_filelist.length();
        goto end;
    }
    if(playmode==CYCLE){
        goto end;
    }
    if(direction == DOWN){
            tmppos++;
            if(tmppos == m_filelist.length()){
                if(playmode == SEQUENCE)
                    tmppos = -1;
                else
                    tmppos = 0;
            }
    }else{
        tmppos--;
        if(tmppos == -1){
            if(playmode == LOOP)
                tmppos = m_filelist.length()-1;
        }
        direction = DOWN;       // 向上播放只可能出现在上一曲的选项中，这个效果只使用一次
    }

end:
    return tmppos;
}

void Widget::addStr(QString &path){
    QStringList splitList=path.split("/");       // 只取出文件名
     ui->m_playlist->addItem(splitList[splitList.length()-1]);    // 将曲目添加进播放列表
}


void Widget::on_m_nextButton_clicked()
{
    m_mplayerProcess->write("quit\n");          // 在player结束后，Ended槽会自动检测nextfile的值并决定是否继续播放
}

void Widget::on_m_prevButton_clicked()
{
    direction = UP;                                             // 在selNext自动切换为DOWN
    m_mplayerProcess->write("quit\n");
}

void Widget::handleDrag(int oldp, int newp){
    if(newp == -1) return;                  // 移出边界

    // 处理当前的播放位置
    if(oldp == playpos){         // 选中当前播放歌曲
        if(newp < playpos)
            playpos = newp+1;
        else
            playpos = newp;
    }else if(oldp < playpos){
        if(newp >= playpos)
            playpos--;
    }else{
        if(newp < playpos)
            playpos++;
    }

    if(oldp > newp)                         // 如果从下往上移动，按照已有逻辑，应该移到目的项的下方，所以newp需+1
        m_filelist.move(oldp, newp+1);
    else
        m_filelist.move(oldp, newp);
}

void Widget::on_m_cycleButton_clicked()
{
    switch(playmode){
    case RANDOM:
        ui->m_randomButton->setIcon(QIcon(":/images/random.png"));
        /* fall through */
    case LOOP:
        ui->m_loopButton->setIcon(QIcon(":/images/loop.png"));
        /* fall through */
    case SEQUENCE:
        ui->m_cycleButton->setIcon(QIcon(":/images/cycle(valid).png"));
        playmode = CYCLE;
        break;
    case CYCLE:
        ui->m_cycleButton->setIcon(QIcon(":/images/cycle.png"));
        playmode = SEQUENCE;
        break;
    }
}

void Widget::getTimepos(){
    m_mplayerProcess->write("get_time_pos\n");
    m_mplayerProcess->write("get_time_length\n");
}

void Widget::on_m_playSlider_sliderPressed()
{
    if(m_isplaying) {
        m_timeTimer->stop();
    }
}

void Widget::on_m_playSlider_sliderReleased()
{
        m_mplayerProcess->write(QString("seek " + QString::number(ui->m_playSlider->value()) + " 2\n").toUtf8());
        m_timeTimer->start(1000);
}
void Widget::receive_pos(double pos)
{
    double p=pos/241*maxTime;
    char buf[20];
    ui->m_playSlider->setValue(p);
   if(m_ispause){
        m_ispause = 0;
        ui->m_playButton->setIcon(QIcon(":/images/pause.png"));
    }
    m_mplayerProcess->write(QString("seek " + QString::number(ui->m_playSlider->value()) + " 2\n").toUtf8());
    sprintf(buf, "%02d:%02d", (int)p/60, (int)p%60);
    ui->m_curLabel->setText(buf);
}

void Widget::on_m_playlist_doubleClicked(const QModelIndex &index)
{
    int clickpos;
    clickpos = index.row();
    if(playpos == clickpos){
        on_m_playButton_clicked();
    }else{
        on_m_stopButton_clicked();
        m_mplayerProcess->waitForFinished();        // 防止进程还未结束时点击按钮
        playpos = clickpos;
        on_m_playButton_clicked();
    }
}
