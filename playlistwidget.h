#ifndef PLAYLISTWIDGET_H
#define PLAYLISTWIDGET_H

#include <QListWidget>
#include <QMouseEvent>

class PlayListWidget : public QListWidget
{
    Q_OBJECT
public:
    PlayListWidget(QWidget *parent=0);
signals:
    void dragEvent(int oldp, int newp);         // 通知窗口:文件列表需要改变
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
private:
    void performDrag();

    QPoint startPos;
};

#endif // PLAYLISTWIDGET_H

