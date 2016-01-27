#include "compass.h"

Compass::Compass(QWidget *parent) : QWidget(parent)
{
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start();
    angle = 0;
}

void Compass::paintEvent(QPaintEvent *)
{
    /*Defining points for the triangle-shaped hand*/
    static const QPoint hourHand[3] = {
        QPoint(7, 10),
        QPoint(-7, 10),
        QPoint(0, -60)
    };

    /*Setting colors*/
    QColor minuteColor(Qt::black);
    QColor hourColor(Qt::green);

    /*Sets the side length of the square to the smaller side*/
    int side = qMin(width(), height());

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.translate(width() / 2, height() / 2);
    painter.scale(side / 200.0, side / 200.0);

    painter.setBrush(minuteColor);

    painter.save();

    /*Paint hand at right angle*/
    painter.rotate(angle);
    painter.drawConvexPolygon(hourHand, 3);
    painter.restore();

    /*Paint hour markings*/
    painter.setPen(hourColor);

    for (int i = 0; i < 12; ++i) {
        painter.drawLine(60, 0, 75, 0);
        painter.rotate(30.0);
    }

    painter.setBrush(hourColor);

    /*Paint minute markings*/
    painter.setPen(minuteColor);

    for (int j = 0; j < 60; ++j) {
        if ((j % 5) != 0)
            painter.drawLine(67, 0, 75, 0);
        painter.rotate(6.0);
    }

    /*Paint NESW markings*/
    static const QPoint north = QPoint(-7,-84);
    static const QPoint east = QPoint(83,8);
    static const QPoint south = QPoint(-6.5,97);
    static const QPoint west = QPoint(-103,8);

    QFont font=painter.font();
    font.setPointSize(15);
    font.setWeight(QFont::DemiBold);

    painter.setPen(hourColor);
    painter.setFont(font);

    painter.drawText(north, "N");
    painter.drawText(east, "E");
    painter.drawText(south, "S");
    painter.drawText(west, "W");

    painter.end();
}
