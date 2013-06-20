#include "colorlabel.h"

#include <QPainter>
#include <QDebug>

#include <QColorDialog>
#include <QPaintEvent>

ColorLabel::ColorLabel(QWidget *parent) :
    QLabel(parent),
    mColor(QColor(Qt::black))
{
    setAutoFillBackground(true);
    updateColor();
    setAcceptDrops(true);

}

void ColorLabel::setColor(QColor c)
{
    if(c == mColor)
        return;

    mColor = c;
    updateColor();
    emit colorChanged(c);
}

void ColorLabel::setText(const QString &text)
{
    QPainter p(this);
    int w = p.fontMetrics().width(text);
    setMinimumWidth(w + 4);
    QLabel::setText(text);
}

void ColorLabel::mouseReleaseEvent(QMouseEvent *ev)
{
    QLabel::mouseReleaseEvent(ev);
    selectColor();
}

void ColorLabel::dragEnterEvent(QDragEnterEvent *e)
{
    if(e->mimeData()->hasFormat("application/x-color")) {
        e->acceptProposedAction();
    }
}

void ColorLabel::dropEvent(QDropEvent *e)
{

    QColor c = e->mimeData()->colorData().value<QColor>();
    if(c.isValid()) {
        setColor(c);
    }
}

void ColorLabel::updateColor()
{

    QPalette pal = palette();
    pal.setBrush(QPalette::Window, QBrush(mColor));
    setPalette(pal);
}

void ColorLabel::selectColor()
{

    QColorDialog cDlg;
    QColor newColor = cDlg.getColor(mColor, this, text());

    if(newColor.isValid())
        setColor(newColor);
}

void ColorLabel::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QFont font = painter.font();
    painter.setPen(QPen(QColor(Qt::black)));
    painter.setFont(font);

    int txtWidth = painter.fontMetrics().width(text());
    int txtHeight = painter.fontMetrics().height();

    QRectF rect = QRectF((width() - txtWidth)/2 -2, (height() - txtHeight)/2 -2, txtWidth+4, txtHeight+4);
    //painter.fillRect(rect,);
    painter.setBrush(QBrush(QColor(255,255,255,128)));
    painter.setPen(QPen(QColor(255,255,255,128)));
    painter.drawRoundedRect(rect, 3, 3);

    QLabel::paintEvent(event);
}