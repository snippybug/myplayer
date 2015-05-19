#include "playlistwidget.h"
#include <QApplication>


PlayListWidget::PlayListWidget(QWidget *parent)
    : QListWidget(parent)
{
    setAcceptDrops(true);           // 列表框的放下生效
}

void PlayListWidget::mousePressEvent(QMouseEvent *event){
    if(event->button() == Qt::LeftButton)
        startPos = event->pos();                            // 保存拖动的起始坐标
    QListWidget::mousePressEvent(event);
}

void PlayListWidget::mouseMoveEvent(QMouseEvent *event){
    if(event->buttons() & Qt::LeftButton){
        int distance = (event->pos() - startPos).manhattanLength();
        if(distance >= QApplication::startDragDistance())       // 如果矢量长度大于QApplication推荐的长度，则启动拖动操作，避免用户因为手握鼠标抖动而产生拖动
            performDrag();
    }
    QListWidget::mouseMoveEvent(event);
}

void PlayListWidget::performDrag(){
    QListWidgetItem *item = currentItem();
    if(item){
        QMimeData *mimeData = new QMimeData;
        mimeData->setText(item->text());            // 将文本传递过去

        QDrag *drag = new QDrag(this);
        drag->setMimeData(mimeData);
        drag->setPixmap(QPixmap(":/images/person.png"));

        if(drag->exec(Qt::MoveAction) == Qt::MoveAction){       // 执行拖动操作，直到用户放下或取消此次拖动操作才会停止
            delete item;
        }
    }
}

void PlayListWidget::dragEnterEvent(QDragEnterEvent *event){
        PlayListWidget *source = qobject_cast<PlayListWidget *>(event->source());
        if(source && source == this){           // 只支持本列表框内拖放
            event->setDropAction(Qt::MoveAction);
            event->accept();
        }
}

void PlayListWidget::dragMoveEvent(QDragMoveEvent *event){
    PlayListWidget *source = qobject_cast<PlayListWidget *>(event->source());
    if(source && source == this){             // 只支持本列表框内拖放
        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
}

void PlayListWidget::dropEvent(QDropEvent *event){
    PlayListWidget *source = qobject_cast<PlayListWidget *>(event->source());
    if(source && source == this){
        QListWidgetItem *dst = itemAt(event->pos());                    // 获取终点的列表项
        QListWidgetItem *src = itemAt(startPos);                            // 起点列表项
        emit dragEvent(row(src), row(dst));                                     // 通知窗口：文本位置改变
        insertItem(row(dst)+1, event->mimeData()->text());          //  将拖动的列表项放入终点列表的下方
        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
}
