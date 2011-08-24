/*************************************************\
| Copyright (c) 2010 Stitch Works Software        |
| Brian C. Milco <brian@stitchworkssoftware.com>  |
\*************************************************/
#include "scenerounds.h"

#include "crochetcell.h"

#include <QFontMetrics>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsSceneEvent>
#include <QApplication> //start drag min distance.

#include <math.h>

#include <QDebug>

#include "chartview.h"

#include "settings.h"
#include "stitchset.h"
#include "appinfo.h"
#include "crochetchartcommands.h"
#include "indicatorundo.h"

#include <QKeyEvent>
#include "stitchlibrary.h"

SceneRounds::SceneRounds(QObject *parent)
    : Scene(parent),
    mCenterSymbol(0)
{
}

SceneRounds::~SceneRounds()
{
}

void SceneRounds::setShowChartCenter(bool state)
{
    mShowChartCenter = state;

    if(mShowChartCenter) {
        if(!mCenterSymbol) {
            QPen pen;
            pen.setWidth(5);

            double radius = (defaultSize().height() *0.45);

            mCenterSymbol = addEllipse(-radius, -radius, radius * 2, radius * 2, pen);
            mCenterSymbol->setToolTip(tr("Chart Center"));
        } else {
            addItem(mCenterSymbol);
        }
    } else {
        removeItem(mCenterSymbol);
    }

}

void SceneRounds::removeCell(CrochetCell *c)
{
    int y = findGridPosition(c).y();
    removeItem(c);
    for(int i = 0; i < grid().count(); ++i) {
        if (grid()[i].contains(c)) {
            grid()[i].removeOne(c);
        }
    }

    redistributeCells(y);
}

int SceneRounds::rowCount()
{
    return grid().count();
}

int SceneRounds::columnCount(int row)
{
    if(grid().count() <= row)
        return 0;
    return grid()[row].count();
}

void SceneRounds::appendCell(int row, CrochetCell *c, bool fromSave)
{
    //append any missing rows.
    if(grid().count() <= row) {
        for(int i = grid().count(); i < row + 1; ++i) {
            QList<CrochetCell*> row;
            grid().append(row);
        }
    }
    /*QPoint(grid()[row].count(), row)*/
    addCell(c, QPointF());

    int col = grid()[row].count() -1;
    setCellPosition(row, col, c, grid()[row].count());
    c->setColor(QColor(Qt::white));
   
    if(!fromSave) {
        redistributeCells(row);
    }
}

void SceneRounds::addCell(CrochetCell* c, QPointF p)
{

    //TODO: simplify the connect() statements...
    addItem(c);
    int x = p.x();

    if(grid().count() <= p.y()) {
        QList<CrochetCell*> row;
        grid().append(row);
    }

    if(grid()[p.y()].count() <= p.x())
            x = grid()[p.y()].count();

    grid()[p.y()].insert(x, c);

    setCellPosition(p.y(), x, c, grid()[p.y()].count());

    connect(c, SIGNAL(stitchChanged(QString,QString)), this, SIGNAL(stitchChanged(QString,QString)));
    connect(c, SIGNAL(colorChanged(QString,QString)), this, SIGNAL(colorChanged(QString,QString)));

    redistributeCells(p.y());

}

void SceneRounds::setCellPosition(int row, int column, CrochetCell *c, int columns, bool updateAnchor)
{
    double widthInDegrees = 360.0 / columns;

    double radius = defaultSize().height() * (row + 1) + (defaultSize().height() *0.5);

    double degrees = widthInDegrees*column;
    QPointF finish = calcPoint(radius, degrees, QPointF(0,0));

    qreal delta = defaultSize().width() * 0.5;
    if(updateAnchor || c->anchor().isNull())
        c->setAnchor(finish.x() - delta, finish.y());
    c->setPos(finish.x() - delta, finish.y());
    c->setTransform(QTransform().translate(delta,0).rotate(degrees + 90).translate(-delta, 0));
    c->setAngle(degrees + 90);
    c->setToolTip(tr("Row: %1, St: %2").arg(row+1).arg(column+1));
}

void SceneRounds::redistributeCells(int row)
{
    if(row >= grid().count())
        return;
    int columns = grid()[row].count();

    for(int i = 0; i < columns; ++i) {
        CrochetCell *c = grid()[row].at(i);
        setCellPosition(row, i, c, columns, true);
    }
}

void SceneRounds::createChart(int rows, int cols, QString stitch, QSizeF rowSize)
{
 
    defaultSize() = rowSize;

    for(int i = 0; i < rows; ++i) {
        //FIXME: this padding should be dependant on the height of the sts.
        int pad = i * 12;

        createRow(i, cols + pad, stitch);
    }

    setShowChartCenter(Settings::inst()->value("showChartCenter").toBool());
    
    initDemoBackground();
}

void SceneRounds::createRow(int row, int columns, QString stitch)
{
    CrochetCell *c = 0;
    
    QList<CrochetCell*> modelRow;
    for(int i = 0; i < columns; ++i) {
        c = new CrochetCell();
        connect(c, SIGNAL(stitchChanged(QString,QString)), this, SIGNAL(stitchChanged(QString,QString)));
        connect(c, SIGNAL(colorChanged(QString,QString)), this, SIGNAL(colorChanged(QString,QString)));
        
        c->setStitch(stitch, (row % 2));
        addItem(c);
        modelRow.append(c);
        c->setColor(QColor(Qt::white));
        setCellPosition(row, i, c, columns);
    }
    grid().append(modelRow);

}

int SceneRounds::getClosestRow(QPointF mousePosition)
{
    //double radius = defaultSize().height() * (row + 1) + (defaultSize().height() *0.5);
    qreal radius = sqrt(mousePosition.x()*mousePosition.x() + mousePosition.y()*mousePosition.y());

    qreal temp = radius - (defaultSize().height() *0.5);
    qreal temp2 = temp / defaultSize().height();
    
    int row = round(temp2 - 1);
    if(row < 0)
        row = 0;
    if(row >= grid().count()) {
        row = grid().count();

        QList<CrochetCell*> r;
        grid().append(r);
    }

    return row;
}

int SceneRounds::getClosestColumn(QPointF mousePosition, int row)
{
    /*
              |
          -,- | +,-
        ------+------
          -,+ | +,+
              |
    */
    qreal tanx = mousePosition.y() / mousePosition.x();
    qreal rads = atan(tanx);
    qreal angleX = rads * 180 / M_PI;
    qreal angle = 0.0;
    if (mousePosition.x() >= 0 && mousePosition.y() >= 0)
        angle = angleX;
    else if(mousePosition.x() <= 0 && mousePosition.y() >= 0)
        angle = 180 + angleX;
    else if(mousePosition.x() <= 0 && mousePosition.y() <= 0)
        angle = 180 + angleX;
    else if(mousePosition.x() >= 0 && mousePosition.y() <= 0)
        angle = 360 + angleX;

    qreal degreesPerPos = 360.0 / grid()[row].count();

    return ceil(angle / degreesPerPos);
}

QPointF SceneRounds::calcPoint(double radius, double angleInDegrees, QPointF origin)
{
    // Convert from degrees to radians via multiplication by PI/180
    qreal x = (radius * cos(angleInDegrees * M_PI / 180)) + origin.x();
    qreal y = (radius * sin(angleInDegrees * M_PI / 180)) + origin.y();
    return QPointF(x, y);
}

QPoint SceneRounds::findGridPosition(CrochetCell* c)
{
    for(int y = 0; y < grid().count(); ++y) {
        if(grid()[y].contains(c)) {
            return QPoint(grid()[y].indexOf(c), y);
        }
    }
    
    return QPoint();
}

void SceneRounds::keyReleaseEvent(QKeyEvent* keyEvent)
{
    Scene::keyReleaseEvent(keyEvent);
}

void SceneRounds::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
    Scene::mousePressEvent(e);
}

void SceneRounds::mouseMoveEvent(QGraphicsSceneMouseEvent *e)
{
    switch(mMode) {
        case Scene::StitchMode:
            stitchModeMouseMove(e);
            break;
        default:
            break;
    }

    Scene::mouseMoveEvent(e);
    
}

void SceneRounds::mouseReleaseEvent(QGraphicsSceneMouseEvent *e)
{
    
    switch(mMode) {
        case Scene::StitchMode:
            stitchModeMouseRelease(e);
            break;
        default:
            break;
    }

    Scene::mouseReleaseEvent(e);
}

void SceneRounds::stitchModeMouseMove(QGraphicsSceneMouseEvent* e)
{
    if(e->buttons() != Qt::LeftButton)
        return;
    
    if(!mCurCell)
        return;

    //FIXME: if you're not draging a stitch you should be able to drag and paint.
}

void SceneRounds::stitchModeMouseRelease(QGraphicsSceneMouseEvent* e)
{
    
    //FIXME: foreach(stitch in selection()) create an undo group event.
    if(mCurCell) {
        
        if(mCurCell->name() != mEditStitch && !mMoving)
            undoStack()->push(new SetCellStitch(this, mCurCell, mEditStitch));
    
        mCurCell = 0;
    } else if(!mRubberBand && !mMoving){
        //FIXME: combine getClosestRow & getClosestColumn into 1 function returning a QPoint.
        int y = getClosestRow(e->scenePos());
        //FIXME: the row has to be passed in because getClosestRow modifies the row
        int x = getClosestColumn(e->scenePos(), y);

        if(e->button() == Qt::LeftButton && !(e->modifiers() & Qt::ControlModifier)) {

            AddCell *addCell = new AddCell(this, QPoint(x, y));
            CrochetCell *c = addCell->cell();
            c->setStitch(mEditStitch, (y % 2));
            undoStack()->push(addCell);

        } else {
            if(!mCurCell)
                return;

            undoStack()->push(new RemoveCell(this, mCurCell));
            mCurCell = 0;
        }
    }
    
}