#include "console.h"

Console::Console(QWidget *parent)
    : QPlainTextEdit(parent)
{
    /*Set maximum number of displayed lines*/
    document()->setMaximumBlockCount(100000);

    /*Set matrix-like colors :) */
    QPalette p = palette();
    p.setColor(QPalette::Base, Qt::black);
    p.setColor(QPalette::Text, Qt::green);
    setPalette(p);
}


/*Give out string on console*/
void Console::writeString(QString input){
    insertPlainText(QString("%1\n").arg(input));

    /*Ensure that scrollbar scrolls with published data*/
    QScrollBar *bar = verticalScrollBar();
    bar->setValue(bar->maximum());
}


/*Disable typing*/
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


/*Disable mouse actions (pressing/clicking/rightclick)*/
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
