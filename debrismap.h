#ifndef DEBRISMAP_H
#define DEBRISMAP_H

#include <QWidget>
#include <QtWidgets>

struct Debris{
    int partNumber;
    QPoint location;
    bool isCleaned;
    Debris(int tc_partNumber, float tc_angle, bool tc_isCleaned);
};

class DebrisMap : public QWidget
{
    Q_OBJECT

public:
    explicit DebrisMap(QWidget *parent = 0);
    float angle;

    void processDebris(Debris* tc_debris);
    int getFoundNumber();
    int getCleanedNumber();

private:
    QVector<Debris*> debrisList;

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

};

#endif // DEBRISMAP_H
