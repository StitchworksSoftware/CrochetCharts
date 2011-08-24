/*************************************************\
| Copyright (c) 2010 Stitch Works Software        |
| Brian C. Milco <brian@stitchworkssoftware.com>  |
\*************************************************/
#include "crochetcell.h"

#include <QPainter>
#include <QDebug>

#include <QApplication>
#include <QPainter>
#include <QStyleOption>
#include "settings.h"

CrochetCell::CrochetCell()
     : mScale(1.0),
     mHighlight(false)
{
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges); //enable itemChange to pick up move changes.
}

void CrochetCell::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if(color() != Qt::white)
        painter->fillRect(option->rect, color());
    if(mHighlight)
        painter->fillRect(option->rect, option->palette.highlight());

    if(stitch()->isSvg()) {
        Cell::paint(painter, option, widget);
    } else {
        painter->drawPixmap(option->rect.x(), option->rect.y(), *(stitch()->renderPixmap()));

         if(option->state & QStyle::State_Selected) {
            painter->setPen(Qt::DashLine);
            painter->drawRect(option->rect);
            painter->setPen(Qt::SolidLine);
        }
    }
}

void CrochetCell::setScale(qreal newScale, QPointF pivotPoint)
{
    qreal newSize = mOrigHeight * newScale;
    qreal scale = newSize/mOrigHeight;
    mScale = newScale;

    setTransformOriginPoint(pivotPoint);
    Cell::setScale(scale);
}

void CrochetCell::setStitch(QString s, bool useAltRenderer)
{
    Cell::setStitch(s, useAltRenderer);
    mOrigWidth = boundingRect().width();
    mOrigHeight = boundingRect().height();
}

void CrochetCell::setStitch(Stitch *s, bool useAltRenderer)
{
   Cell::setStitch(s, useAltRenderer);
   mOrigWidth = boundingRect().width();
   mOrigHeight = boundingRect().height();
}

void CrochetCell::setRotation(qreal angle, QPointF pivotPoint)
{
    
    setTransform(QTransform().translate(pivotPoint.x(), pivotPoint.y()).rotate(angle).translate(-pivotPoint.x(), -pivotPoint.y()));
    setAngle(angle);
    
}
