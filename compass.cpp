#include <QtWidgets>

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
    //Defining points for the triangle-shaped hand
    static const QPoint hourHand[3] = {
        QPoint(7, 8),
        QPoint(-7, 8),
        QPoint(0, -40)
    };

    //Setting colors for the hand / lines
    QColor minuteColor(Qt::green);
    QColor hourColor(Qt::gray);

    //Sets the side length of the square to the smallest side
    int side = qMin(width(), height());

    //Initializing painter
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    //Moving painter to the middle of the square
    painter.translate(width() / 2, height() / 2);
    //Scaling the canvas so that there are 100 "points" in each direction?
    painter.scale(side / 200.0, side / 200.0);

    //Initializing painter
    painter.setPen(Qt::NoPen);
    painter.setBrush(minuteColor);

    painter.save();

    //Paint hand
    painter.rotate(angle);
    painter.drawConvexPolygon(hourHand, 3);
    painter.restore();

    //Paint hour markings
    painter.setPen(hourColor);

    for (int i = 0; i < 12; ++i) {
        painter.drawLine(60, 0, 75, 0);
        painter.rotate(30.0);
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(hourColor);

    //Paint minute markings
    painter.setPen(minuteColor);

    for (int j = 0; j < 60; ++j) {
        if ((j % 5) != 0)
            painter.drawLine(67, 0, 75, 0);
        painter.rotate(6.0);
    }

    //Paint NESW markings
    static const QPoint north = QPoint(-4.5,-85);
    static const QPoint east = QPoint(85,5);
    static const QPoint south = QPoint(-4.5,95);
    static const QPoint west = QPoint(-96,5);
    painter.setPen(hourColor);
    QFont font=painter.font();
    font.setPointSize(10);
    font.setWeight(QFont::DemiBold);
    painter.setFont(font);
    painter.drawText(north, "N");
    painter.drawText(east, "E");
    painter.drawText(south, "S");
    painter.drawText(west, "W");

    painter.end();
}
