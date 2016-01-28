#include "debrismap.h"

DebrisMap::DebrisMap(QWidget *parent) : QWidget(parent)
{
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start();

    angle = 0;
}

void DebrisMap::paintEvent(QPaintEvent *)
{
    /*Defining points for the triangle-shaped hand*/
    static const QPoint hourHand[3] = {
        QPoint(7, 10),
        QPoint(-7, 10),
        QPoint(0, -60)
    };

    /*Setting colors*/
    QColor hourColor(Qt::yellow);
    QColor minuteColor(Qt::black);
    QColor foundColor(Qt::red);
    QColor cleanedColor(Qt::green);

    /*Sets the side length of the square to the smaller side*/
    int side = qMin(width(), height());

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    /*Moving painter to the middle of the canvas*/
    painter.translate(width() / 2, height() / 2);
    painter.scale(side / 200.0, side / 200.0);

    painter.setPen(Qt::NoPen);
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

    painter.setPen(Qt::NoPen);
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

   /*Paint debris markings*/

    foreach (const Debris* piece, debrisList){
        if(piece->isCleaned){
            painter.setPen(cleanedColor);
            painter.setBrush(cleanedColor);
            painter.drawEllipse(piece->location, 4, 4);
        }else{
            painter.setPen(foundColor);
            painter.setBrush(foundColor);
            painter.drawEllipse(piece->location, 4, 4);
        }
    }

    painter.end();
}

void DebrisMap::processDebris(Debris* tc_debris){
    foreach (Debris* piece, debrisList){
        if(piece->partNumber == tc_debris->partNumber){
            piece->location = tc_debris->location;
            piece->isCleaned = tc_debris->isCleaned;
            return;
        }
    }
    debrisList.append(tc_debris);
}

int DebrisMap::getFoundNumber(){
    return debrisList.length();
}

int DebrisMap::getCleanedNumber(){
    int cleaned = 0;
    foreach (Debris* piece, debrisList){
        if(piece->isCleaned){
            cleaned++;
        }
    }
    return cleaned;
}

Debris::Debris(int tc_partNumber, float tc_angle, bool tc_isCleaned): partNumber(0), location(QPoint(0,0)), isCleaned(false){
    partNumber = tc_partNumber;
    float radius = 50;
    float x = radius*qSin(qDegreesToRadians(tc_angle));
    float y = radius*qCos(qDegreesToRadians(tc_angle));
    location = QPoint(x,y);
    isCleaned = tc_isCleaned;
}
