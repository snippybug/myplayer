#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QProcess>
#include <QFileDialog>
#include <QTimer>

#define LCDWIDTH    480
#define LCDHEIGHT   272

namespace Ui {
class Widget;
}

enum playMode{
    SEQUENCE, RANDOM, LOOP,CYCLE
};

enum Direction{
    UP, DOWN
};

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void catchOutput();
    void mplayerEnded(int exitCode, QProcess::ExitStatus exitStatus);
    void handleDrag(int oldp, int newp);
    void getTimepos();
    void on_m_playButton_clicked();

    void on_m_openButton_clicked();

    void on_m_stopButton_clicked();

    void on_m_volumeSlider_valueChanged(int value);

    void on_m_deleteButton_clicked();

    void on_m_loopButton_clicked();

    void on_m_randomButton_clicked();

    void on_m_nextButton_clicked();

    void on_m_prevButton_clicked();

    void on_m_cycleButton_clicked();

private:
    bool startMPlayer(int pos=0);
    bool pauseMPlayer();
    bool stopMPlayer();
    bool isDuplicating(QString &);
    int selectNext();
    void addStr(QString &);

    Ui::Widget *ui;
    QProcess *m_mplayerProcess;

    QStringList m_filelist;
    QTimer *m_timeTimer;

    bool m_isplaying;
    bool m_ispause;
    bool nextfile;
    int playpos;
    enum playMode playmode;
    enum Direction direction;
};



#endif // WIDGET_H
