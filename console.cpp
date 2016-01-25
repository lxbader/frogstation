#include "console.h"

Console::Console(QWidget *parent)
    : QPlainTextEdit(parent)
{
    document()->setMaximumBlockCount(10000);
    QPalette p = palette();
    p.setColor(QPalette::Base, Qt::black);
    p.setColor(QPalette::Text, Qt::green);
    setPalette(p);
}


void Console::writeString(QString input){
    insertPlainText(QString("%1\n").arg(input));

    QScrollBar *bar = verticalScrollBar();
    bar->setValue(bar->maximum());
}

void Console::keyPressEvent(QKeyEvent *e){
    switch (e->key()) {
    case Qt::Key_Backspace:
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Up:
    case Qt::Key_Down:
        break;
    }
}

void Console::mousePressEvent(QMouseEvent *e){
    Q_UNUSED(e)
    setFocus();
}

void Console::mouseDoubleClickEvent(QMouseEvent *e){
    Q_UNUSED(e)
}

void Console::contextMenuEvent(QContextMenuEvent *e){
    Q_UNUSED(e)
}
