#include "groundstation.h"
#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    QPalette p;
    QApplication a(argc, argv);
    p = a.palette();
    p.setColor(QPalette::Button, QColor(150,150,150));
    a.setPalette(p);
    Groundstation w;
    w.show();

    return a.exec();
}
