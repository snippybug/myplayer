#include "widget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    //w.setWindowFlags(Qt::FramelessWindowHint);
    w.setWindowTitle("PickyPlayer");           // 设置窗口名称
    w.setFixedHeight(LCDHEIGHT);
    w.setFixedWidth(LCDWIDTH);
    w.show();

    return a.exec();
}
