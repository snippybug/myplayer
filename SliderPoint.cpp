#include "SliderPoint.h"
#include <QApplication>

SliderPoint::SliderPoint(QWidget *parent) :
	QSlider(parent){
    ;
}

void SliderPoint::mousePressEvent(QMouseEvent *ev){
	QSlider::mousePressEvent(ev);
	double pos = ev->pos().x();
	emit send_pos(pos);
}
