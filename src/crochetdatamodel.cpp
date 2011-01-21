/*************************************************\
| (c) 2010-2011 Stitch Works Software             |
| Brian C. Milco <brian@stitchworkssoftware.com>  |
\*************************************************/
#include "crochetdatamodel.h"

#include "cell.h"
#include "stitchcollection.h"
#include "stitchset.h"

CrochetDataModel::CrochetDataModel(QObject *parent) :
    QAbstractItemModel(parent)
{

}

void CrochetDataModel::setCell(int row, int column, Cell *c)
{

    if(row >= mGrid.count())
        return;
    if(column >= mGrid[row].count())
        return;

    mGrid[row][column] = c;
}

Cell* CrochetDataModel::cell(int row, int column)
{

    if(row >= mGrid.count())
        return 0;
    if(column >= mGrid[row].count())
        return 0;

    return mGrid[row].at(column);
}

QModelIndex CrochetDataModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return this->createIndex(row, column);
}

QModelIndex CrochetDataModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int CrochetDataModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return mGrid.count();
}

int CrochetDataModel::columnCount (const QModelIndex &parent) const
{
    int row = parent.row();
    if (mGrid.count() <= row)
        return -1;

    return mGrid[row].count();
}

int CrochetDataModel::columnCount(int row)
{
    return columnCount(this->index(row, 0));
}

QVariant CrochetDataModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(index);
    Q_UNUSED(role);
    return QVariant();
}

void CrochetDataModel::setRowCount(int rows)
{
    if(rows <= 0)
        return;

    //TODO: allow code to truncate the row count here? or rename the function?
    if(rows < mGrid.count())
        return;

    for(int i = 0; i < rows; ++i) {
        QList<Cell *> row;
        mGrid.append(row);
    }
}

//FIXME: pass in the information about what cells to set as defaults.
void CrochetDataModel::setColumnCount(int row, int columns)
{
    if(columns <= 0)
        return;
    if(mGrid.count() <= row)
        return;

    for(int i = 0; i < columns; ++i) {
        Cell *c = new Cell(0);
        mGrid[row].append(c);
    }
}

void CrochetDataModel::appendRow(QList<Cell *> row)
{
    mGrid.append(row);
}

void CrochetDataModel::appendColumn(int row)
{
    if(row >= mGrid.count())
        return;

    //FIXME: finish function...
    //mGrid[row].insert(column, new Cell());

}

void CrochetDataModel::addRow(int columns)
{
    //FIXME: don't hard code the stitch, use the default stitch.
    QList<Cell*> row;
    Stitch* s = StitchCollection::inst()->masterStitchSet()->findStitch("ch");
    for(int i = 0; i < columns; ++i) {
        Cell* c = new Cell();
        c->setStitch(s);
        row.append(c);
    }

    mGrid.append(row);
}

void CrochetDataModel::removeColumn(int row, int column)
{

    if(row >= mGrid.count())
        return;
    if(column >= mGrid[row].count())
        return;

//FIXME: finish function...
}

bool CrochetDataModel::insertRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(row);
    Q_UNUSED(count);
    Q_UNUSED(parent);
    return true;
}

bool CrochetDataModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    Q_UNUSED(column);
    Q_UNUSED(count);
    Q_UNUSED(parent);
    return true;
}
