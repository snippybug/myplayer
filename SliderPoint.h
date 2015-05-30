#ifndef SLIDERPOINT_H
#define SLIDERPOINT_H

#include <QSlider>
#include <QMouseEvent>

class SliderPoint: public QSlider{
	Q_OBJECT
public:
	explicit SliderPoint(QWidget *parent = 0);

signals:
	void send_pos(double pos);
protected:
	void mousePressEvent(QMouseEvent *ev);
};

#endif // SLIDERPOINT_H
