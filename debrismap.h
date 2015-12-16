#ifndef DEBRISMAP_H
#define DEBRISMAP_H

#include <QWidget>

class DebrisMap : public QWidget
{
    Q_OBJECT

public:
    explicit DebrisMap(QWidget *parent = 0);
    QVector<QPoint>* debrisFound;
    QVector<QPoint>* debrisCleaned;    
    float angle;

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

signals:

public slots:
    void addDebris(double angle);
    void cleanedDebris(QPoint debris);

};

#endif // DEBRISMAP_H
