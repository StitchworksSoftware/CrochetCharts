/*************************************************\
| Copyright (c) 2011 Stitch Works Software        |
| Brian C. Milco <brian@stitchworkssoftware.com>  |
\*************************************************/
#include "stitchlibrarydelegate.h"

#include "stitch.h"
#include <QPainter>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>

#include <QStyleOption>
#include <QStyleOptionViewItem>

#include <math.h>

#include <QApplication>

#include "settings.h"
#include <QFileInfo>
#include <QDir>

#include <QDebug>
#include "stitchcollection.h"
#include "stitchset.h"
#include <QSvgRenderer>
#include <QMessageBox>

StitchLibraryDelegate::StitchLibraryDelegate(QWidget *parent)
    : QStyledItemDelegate(parent)
{
    mSignalMapper = new QSignalMapper(this);   
    connect(mSignalMapper, SIGNAL(mapped(int)), this, SIGNAL(addStitchToMasterSet(int)));
}

void StitchLibraryDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(!index.isValid())
        return;

    QStyleOption opt = option;
    QString buttonText;

    if(index.column() == 5) //FIXME: don't hard code the text here.
        buttonText = tr("Add Stitch");
    else
        buttonText = index.data(Qt::DisplayRole).toString();
    
    int width = option.fontMetrics.width(buttonText);
    int height = option.fontMetrics.height();
    int borderW = ceil((option.rect.width() - width) / 2.0);
    int borderH = ceil((option.rect.height() - height) / 4.0);

    if (index.column() == 1) {
        if(option.state & QStyle::State_Selected)
            painter->fillRect(option.rect, option.palette.highlight());
        else if(option.state & QStyle::State_MouseOver)
            painter->fillRect(option.rect, option.palette.highlight().color().light(190));
        
        QRectF rect = QRectF((qreal)option.rect.x(), (qreal)option.rect.y(),
                             (qreal)option.rect.width(), (qreal)option.rect.height());

        QString fileName = index.data(Qt::EditRole).toString();
        QString sufix = QFileInfo(fileName).completeSuffix();
        if(sufix == "svg" || sufix == "svgz") {
            QSvgRenderer renderer;
            renderer.load(fileName);
            renderer.render(painter, rect);
        } else {
            QPixmap pix = QPixmap(fileName);
            painter->drawPixmap(option.rect, pix);
        }
        
    } else if (index.column() == 3 || index.column() == 4) {

        if(option.state & QStyle::State_Selected)
            painter->fillRect(option.rect, option.palette.highlight());
        
        //FIXME: QStyle::PE_IndicatorButtonDropDown causes a crash.
        qApp->style()->drawPrimitive(QStyle::PE_PanelButtonCommand, &opt, painter);
        painter->drawText(option.rect.x() + 6, option.rect.y() + (borderH + height), buttonText);
        
    } else if(index.column() == 5) {
        if(option.state & QStyle::State_Selected)
            painter->fillRect(option.rect, option.palette.highlight());
        
        qApp->style()->drawPrimitive(QStyle::PE_PanelButtonCommand, &opt, painter);

        painter->drawText(option.rect.x() + borderW, option.rect.y() + (borderH + height), buttonText);
    } else {
        //fall back to the basic painter.
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize StitchLibraryDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int padding = 0;
    
    if(!index.isValid())
        return QSize(100, 32);

    Stitch *s = static_cast<Stitch*>(index.internalPointer());
    if(!s)
        return QSize(100, 32);

    QString text;

    switch(index.column()) {
        case Stitch::Name:
            text = s->name();
            padding += 50;
            break;
        case Stitch::Icon: {
            QSize retSize;
            //TODO: should this be getting info form the *s?
            if(s->isSvg())
                retSize = s->renderSvg()->defaultSize();
            else
                retSize = s->renderPixmap()->size();
            return retSize;
        }
        case Stitch::Description:
            padding +=150;
            text = s->description();
            break;
        case Stitch::Category:
            padding += 50;
            text = s->category();
            break;
        case Stitch::WrongSide:
            padding +=50;
            text = s->wrongSide();
            break;
        case 5:
            padding += 50;
            text = tr("Add Stitch"); //TODO: there's a button to estimate too.
            break;
        default:
            text = "";
            break;
    }
    
    QSize hint = option.fontMetrics.size(Qt::TextWordWrap, text);
    hint.setWidth(hint.width() + padding);

    //HACK: make the height of the icon the height of the whole row.
    StitchSet *set = static_cast<StitchSet*>((QAbstractItemModel*)index.model());
    QSize sizeH = sizeHint(option, set->index(index.row(), Stitch::Icon));
    hint.setHeight(sizeH.height());

    return hint;
}

QWidget* StitchLibraryDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    if(!index.isValid())
        return new QWidget(parent);
    
    switch(index.column()) {
        case Stitch::Name:{ //TODO: add a validator that checks if the name already exists.
            QLineEdit *editor = new QLineEdit(parent);
            QRegExpValidator *validator = new QRegExpValidator(QRegExp("[a-zA-Z][a-zA-Z0-9]+"), editor);
            editor->setValidator(validator);
            return editor;
        }
        case Stitch::Icon: {
            IconComboBox *cb = new IconComboBox(parent);
            loadIcons(cb);
            cb->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
            cb->setIconSize(QSize(32,32));
            cb->model()->sort(0); //TODO: check case sensitivity to the sorting.
            return cb;
        }
        case Stitch::Description: {
            QLineEdit *editor = new QLineEdit(parent);
            return editor;
        }
        case Stitch::Category: {
            QComboBox *cb = new QComboBox(parent);
            cb->addItems(StitchCollection::inst()->categoryList());
            return cb;
        }
        case Stitch::WrongSide: {
            QComboBox *cb = new QComboBox(parent);
            cb->addItems(StitchCollection::inst()->stitchList());
            return cb;
        }
        case 5: {
            QPushButton *pb = new QPushButton(parent);
            pb->setText(tr("Add Stitch"));
            mSignalMapper->setMapping(pb, index.row());
            connect(pb, SIGNAL(clicked(bool)), mSignalMapper, SLOT(map()));
            return pb;
        }
        default:
            return new QWidget(parent);
    }

    return new QWidget(parent);
}

void StitchLibraryDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if(!index.isValid())
        return;

    switch(index.column()) {
        case Stitch::Name: {
            QLineEdit *le = static_cast<QLineEdit*>(editor);
            le->setText(index.data(Qt::EditRole).toString());
            break;
        }
        case Stitch::Icon: {
            IconComboBox *cb = static_cast<IconComboBox*>(editor);
            cb->setCurrentIndex(cb->findData(index.data(Qt::EditRole), Qt::UserRole));
            break;
        }
        case Stitch::Description: {
            QLineEdit *le = static_cast<QLineEdit*>(editor);
            le->setText(index.data(Qt::EditRole).toString());
            break;
        }
        case Stitch::Category: {
            QComboBox *cb = static_cast<QComboBox*>(editor);
            cb->setCurrentIndex(cb->findText(index.data(Qt::EditRole).toString()));
            break;
        }
        case Stitch::WrongSide: {
            QComboBox *cb = static_cast<QComboBox*>(editor);
            cb->setCurrentIndex(cb->findText(index.data(Qt::EditRole).toString()));
            break;
        }
        default:
            break;
    }
}

void StitchLibraryDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    StitchSet *set = static_cast<StitchSet*>(model);
    if(set == StitchCollection::inst()->masterStitchSet()) {
        //TODO:use an overlay or proxy set for changes to the master set...
        qDebug() << "TODO: use an overlay or proxy set for changes to the master set...";
    }
    
    switch(index.column()) {
        case Stitch::Icon: {
            IconComboBox *cb = static_cast<IconComboBox*>(editor);
            model->setData(index, cb->itemData(cb->currentIndex(), Qt::UserRole), Qt::EditRole);
            break;
        }
        case Stitch::Name: {
            QLineEdit *le = static_cast<QLineEdit*>(editor);

            Stitch *s = static_cast<Stitch*>(index.internalPointer());
            Stitch *found = set->findStitch(le->text());

            //is there a stitch with the new name in this set already?
            if(found && found != s) {
                QMessageBox msgbox;
                //TODO: return to the editor with the bad data.
                msgbox.setText(tr("A stitch with this name already exists in the set."));
                msgbox.exec();

                break;
            }
            
            //is this stitch in the master list? if so is there a stitch with the new name already?
            found = 0;
            found = StitchCollection::inst()->masterStitchSet()->findStitch(s->name());
            if(found && found == s) {
                Stitch *m = 0;
                m = StitchCollection::inst()->masterStitchSet()->findStitch(le->text());
                if(m && m != s) {
                    QMessageBox msgbox;
                    msgbox.setText("There is already a stitch with this name in the master list");
                    msgbox.exec();
                    //TODO: offer to remove the stitch already there with this name.

                    break;
                }
            }
                
            model->setData(index, le->text(), Qt::EditRole);
            
            break;
        }
        case Stitch::Description: {
            QLineEdit *le = static_cast<QLineEdit*>(editor);
            model->setData(index, le->text(), Qt::EditRole);
            break;
        }
        case Stitch::WrongSide:
        case Stitch::Category: {
            QComboBox *cb = static_cast<QComboBox*>(editor);
            model->setData(index, cb->currentText(), Qt::EditRole);
            break;
        }
        default:
            break;
    }
}

void StitchLibraryDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);

    editor->setGeometry(option.rect);
}

void StitchLibraryDelegate::loadIcons(QComboBox *cb) const
{
    QStringList dirs, setDir;
    QString userFolder = Settings::inst()->userSettingsFolder();
    
    dirs << ":/stitches";
    dirs << userFolder + "icons";
    
    QDir dir;
    dir.setPath(userFolder + "sets/");
    
    //get all set folders.
    foreach(QString folder, dir.entryList(QDir::Dirs)) {
        if(folder != "." && folder != "..")
            dirs << userFolder + "sets/" + folder;
    }
    
    //get all files from all set folders.
    foreach(QString folder, dirs) {
        dir.setPath(folder);
        foreach(QString file, dir.entryList(QDir::Files)) {
            QIcon icon = QIcon(folder + "/" + file);
            QString name = QFileInfo(file).baseName();
            cb->addItem(icon, name, QVariant(folder + "/" + file));
        }
    }
}

/*
void StitchLibraryDelegate::addStitchToMasterSet(int row)
{
    qDebug() << "row: " << row;
    
}
*/
