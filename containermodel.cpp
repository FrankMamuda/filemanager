/*
 * Copyright (C) 2017 Zvaigznu Planetarijs
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 *
 */

//
// includes
//
#include <QApplication>
#include <QtConcurrent>
#include <QMenu>
#include <QDesktopServices>
#include <QVariant>
#include "containermodel.h"
#include "entry.h"
#include "textutils.h"
#include "properties.h"
#include "pixmapcache.h"
#include "mainwindow.h"
#include "main.h"
#include "pathutils.h"
#include "bookmark.h"
#include "notificationpanel.h"
#include <QInputDialog>

#include <QtWin>
#include <QSysInfo>
#include <commctrl.h>
#include <commoncontrols.h>
#include <shellapi.h>

#include "cache.h"

/**
 * @brief ContainerModel::ContainerModel
 * @param parent
 * @param mode
 * @param iconSize
 */
ContainerModel::ContainerModel( QAbstractItemView *view, ContainerModel::Modes mode, Containers container ) : m_parent( view ), m_mode( mode ), m_iconSize( Common::DefaultListIconSize ), m_selectionLocked( false ), m_container( container ) {
    // create rubber band
    if ( this->parent() != NULL )
        this->m_rubberBand = new QRubberBand( QRubberBand::Rectangle, this->parent()->viewport());

    // create selection model
    this->m_selectionModel = new QItemSelectionModel( this );

    // TODO: disconnect
    this->connect( m.cache, SIGNAL( finished( QString, DataEntry )), this, SLOT( mimeTypeDetected( QString, DataEntry )));
}


/**
 * @brief ContainerModel::~ContainerModel
 */
ContainerModel::~ContainerModel() {
    this->m_rubberBand->deleteLater();
    this->m_selectionModel->deleteLater();
}

/**
 * @brief ContainerModel::columnCount
 * @return
 */
int ContainerModel::columnCount( const QModelIndex & ) const {
    if ( this->container() == ListContainer )
        return 1;
    else if ( this->container() == TableContainer )
        return 4;

    return 0;
}

/**
 * @brief ContainerModel::reset
 * @param force
 */
void ContainerModel::reset( bool force ) {
    if ( this->parent() == NULL )
        return;

    if ( !this->parent()->isVisible() && !force )
        return;

    this->processEntries();
    this->beginResetModel();
    this->endResetModel();
    this->determineMimeTypes();
    this->restoreSelection();
}

/**
 * @brief ContainerModel::softReset
 */
void ContainerModel::softReset() {
    // do as little as possible
    this->beginResetModel();
    this->endResetModel();
    this->determineMimeTypes();
    this->restoreSelection();
}

/**
 * @brief ContainerModel::restoreSelection
 */
void ContainerModel::restoreSelection() {
    int y;

    this->m_selectionLocked = true;

    // restore selection
    foreach ( Entry *entry, this->list ) {
        foreach ( Entry *selectedEntry, this->selectionList ) {
            if ( entry == selectedEntry )
                for ( y = 0; y < this->columnCount(); y++ )
                    this->selectionModel()->select( this->index( this->list.indexOf( entry ), y ), QItemSelectionModel::Select );
        }
    }

    this->m_selectionLocked = false;
}

/**
 * @brief ContainerModel::indexToEntry
 * @param modelIndex
 * @return
 */
Entry *ContainerModel::indexToEntry( const QModelIndex &index ) const {
    int row;

    if ( index.isValid()) {
        row = index.row();

        if ( row < 0 || row >= this->numItems())
            return NULL;

        return this->list.at( row );
    }

    return NULL;
}

/**
 * @brief ContainerModel::setIconSize
 * @param iconSize
 */
void ContainerModel::setIconSize( int iconSize ) {
    /*
    FIXME
    *if ( this->m_iconSize == iconSize )
        return;*/

    this->m_iconSize = iconSize;

    foreach ( Entry *entry, this->list ) {
        entry->reset();

        if ( entry->type() == Entry::Thumbnail || entry->type() == Entry::Executable )
            entry->setType( Entry::FileFolder );
    }
}

/**
 * @brief ContainerModel::setMode
 * @param mode
 */
void ContainerModel::setMode( ContainerModel::Modes mode ) {
    this->m_mode = mode;

    if ( m.gui() != NULL )
        this->buildList( PathUtils::toWindowsPath( m.gui()->currentPath()));
    else
        this->buildList( QDir::currentPath());
}

/**
 * @brief ContainerModel::setActiveContainer
 * @param container
 */
/*void ContainerModel::setActiveContainer( ContainerModel::Containers container ) {
    this->m_container = container;
    this->m_rubberBand->setParent( this->parent());
}*/

/**
 * @brief ContainerModel::flags
 * @param index
 * @return
 */
Qt::ItemFlags ContainerModel::flags( const QModelIndex &index ) const {
    if ( index.isValid() && this->mode() == FileMode )
        return ( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled );

    return Qt::ItemIsEnabled;
}

/**
 * @brief ContainerModel::data
 * @param index
 * @param role
 * @return
 */
QVariant ContainerModel::data( const QModelIndex &modelIndex, int role ) const {
    Entry *entry;
    ContainerItem item;

    entry = this->indexToEntry( modelIndex );
    if ( entry == NULL )
        return QVariant();

    if ( this->container() == ListContainer ) {
        if ( this->displayList.count() <= modelIndex.row())
            return QVariant();
        else
            item = this->displayList.at( modelIndex.row());
    }

    // transparency
    if ( role == Qt::UserRole + 1 )
        return entry->isCut();

    if ( modelIndex.column() == 0 ) {
        switch ( role )  {
        case Qt::DecorationRole:
        {
            return entry->pixmap( this->iconSize());
        }

        case Qt::DisplayRole:
            return entry->alias();

        case Qt::UserRole + DisplayItem:
            if ( this->container() == ListContainer )
                return QVariant::fromValue( item );
        }
    } else if ( modelIndex.column() == 1 ) {
        switch ( role )  {
        case Qt::DisplayRole:
            return entry->info().lastModified().toString( Qt::SystemLocaleShortDate );
        }
    } else if ( modelIndex.column() == 2 ) {
        switch ( role )  {
        case Qt::DisplayRole:
            return entry->mimeType().comment();
        }
    } else if ( modelIndex.column() == 3 ) {
        switch ( role )  {
        case Qt::DisplayRole:
            if ( entry->info().size() > 0 )
                return TextUtils::sizeToText( entry->info().size());
        }
    }

    return QVariant();
}

/**
 * @brief ContainerModel::headerData
 * @param section
 * @param orientation
 * @param role
 * @return
 */
QVariant ContainerModel::headerData( int section, Qt::Orientation orientation, int role ) const {
    if ( this->container() != TableContainer || role != Qt::DisplayRole || orientation != Qt::Horizontal )
        return QVariant();

    if ( section == SectionName )
        return this->tr( "Name" );
    else if ( section == SectionDate )
        return this->tr( "Modified" );
    else if ( section == SectionMimetype )
        return this->tr( "Mimetype" );
    else if ( section == SectionSize )
        return this->tr( "Size" );

    return QVariant();
}

/**
 * @brief ContainerModel::buildList
 * @param path
 */
void ContainerModel::buildList( const QString &path ) {
    QFileInfoList infoList;
    QDir directory( path );

    // clear previous list
    qDeleteAll( this->list );
    this->list.clear();

    // get filelist
    if ( this->mode() == FileMode ) {
        infoList = directory.entryInfoList( QDir::NoDotAndDotDot | QDir::AllEntries, QDir::IgnoreCase | QDir::DirsFirst );
        foreach ( QFileInfo info, infoList )
            this->list << new Entry( Entry::FileFolder, info, this );
    } else if ( this->mode() == SideMode ) {
        // add root pseudo-folder
        this->list << new Entry( Entry::Root, QFileInfo(), this );
        this->list << new Entry( Entry::Home, QFileInfo( QDir::home().absolutePath()), this );

        // build new storage list
        infoList = QDir::drives();
        foreach ( QFileInfo driveInfo, infoList )
            this->list << new Entry( Entry::HardDisk, driveInfo, this );

        this->list << new Entry( Entry::Trash, QFileInfo(), this );
    } else {
        return;
    }

    // reset view
    this->reset();
}

/**
 * @brief ContainerModel::setSelection
 * @param selection
 * @param onReset
 */
void ContainerModel::setSelection( const QModelIndexList &selection ) {
    this->selectionTimer.stop();
    this->selection = selection;

    // update selection list
    if ( !this->selectionLocked()) {
        this->selectionList.clear();
        foreach ( QModelIndex index, selection ) {
            Entry *entry;

            entry = this->indexToEntry( index );
            if ( entry != NULL )
                this->selectionList << entry;
        }
    }
}

/**
 * @brief ContainerModel::processTextDisplay
 */
void ContainerModel::processEntries() {
    int y;

    if ( this->parent() == NULL )
        return;

    // this is only needed for list containers
    if ( this->container() != ListContainer )
        return;

    // clear previous list
    this->displayList.clear();

    // calculate multi-line text sizes
    for ( y = 0; y < this->list.count(); y++ ) {
        const int maxTextLines = 3;
        QString text, line[maxTextLines];
        QListView *view;
        int k, textHeight, numLines = 0, numChars = 0;
        ContainerItem item;

        // get parent listview
        view = qobject_cast<QListView*>( this->parent());
        if ( view == NULL )
            return;

        // get font metrics from parent
        QFontMetrics fm( view->fontMetrics());

        // get display text and height
        text = this->list.at( y )->alias();
        textHeight = fm.height();

        // split text into max 3 lines
        while ( maxTextLines - numLines ) {
            for ( k = 0; k < text.length(); k++ ) {
                if ( fm.width( text.left( k + 1 )) > view->gridSize().width() - 8 )
                    break;
            }

            if ( k > 0 ) {
                numChars = k;
                line[numLines] = text.left( numChars );
                text = text.mid( numChars, text.length() - numChars );
            } else {
                break;
            }

            numLines++;
        }

        // add dots if text does not fit into three lines
        if ( text.length())
            line[2].replace( line[2].length() - 3, 3, "..."  );

        // store data as a new display item
        for ( k = 0; k < numLines; k++ ) {
            item.lines << line[k];
            item.lineWidths << fm.width( line[k] );
        }

        item.textHeight = textHeight;
        this->displayList << item;
    }
}

/**
 * @brief ContainerModel::processMouseRelease
 */
void ContainerModel::processMouseRelease( QMouseEvent *e ) {
    QModelIndex index;

    if ( this->parent() == NULL )
        return;

    this->rubberBand()->hide();

    index = this->parent()->indexAt( e->pos());
    if ( e->button() == Qt::RightButton ) {
        if ( index.isValid())
            this->processContextMenu( index, this->parent()->mapToGlobal( e->pos()) );
        else
            this->selectionModel()->clear();
    }
}

/**
 * @brief ContainerModel::processMouseMove
 * @param selection
 */
void ContainerModel::processMouseMove( QMouseEvent *e ) {
    QModelIndex index;

    if ( this->parent() == NULL )
        return;

    this->currentMousePos = e->pos();
    this->updateRubberBand();

    if ( !this->rubberBand()->isVisible()) {
        index = this->parent()->indexAt( e->pos());

        if ( index.isValid() && this->currentIndex.row() != index.row() && !( QApplication::mouseButtons() & Qt::LeftButton )) {
            this->selectionTimer.stop();

            if ( !this->selectionModel()->isSelected( index ))
                this->selectionTimer.singleShot( 500, this, SLOT( selectCurrent()));
            else
                this->selectionTimer.singleShot( 500, this, SLOT( deselectCurrent()));
        }

        this->currentIndex = index;
    }
}

/**
 * @brief ContainerModel::processMousePress
 * @param pos
 */
void ContainerModel::processMousePress( QMouseEvent *e ) {
    QModelIndex index;

    if ( this->parent() == NULL )
        return;

    index = this->parent()->indexAt( e->pos());
    if ( e->button() == Qt::LeftButton ) {
        if ( !index.isValid()) {
            this->selectionOrigin = e->pos();
            this->selectionOrigin.setY( this->selectionOrigin.y() + this->verticalOffset());
            this->rubberBand()->setGeometry( QRect( this->selectionOrigin, QSize()));
            this->rubberBand()->show();
        }
    }
}

/**
 * @brief ContainerModel::updateRubberBand
 */
void ContainerModel::updateRubberBand() {
    QPoint origin;
    int y, k;

    if ( this->parent() == NULL )
        return;

   // this->determineMimeTypes();

    if ( !this->rubberBand()->isHidden()) {
        origin = this->selectionOrigin;
        origin.setY( this->selectionOrigin.y() - this->verticalOffset());
        this->rubberBand()->setGeometry( QRect( origin, this->currentMousePos ).normalized());

        // manually update selection
        for ( y = 0; y < this->rowCount(); y++ ) {
            for ( k = 0; k < this->columnCount(); k++ ) {
                QModelIndex index;

                index = this->index( y, k );
                if ( this->rubberBand()->geometry().intersects( this->parent()->visualRect( index )))
                    this->selectionModel()->select( index, QItemSelectionModel::Select | QItemSelectionModel::Rows );
                else
                    this->selectionModel()->select( index, QItemSelectionModel::Deselect | QItemSelectionModel::Rows );
            }
        }
    }
}

/**
 * @brief ContainerModel::selectCurrent
 */
void ContainerModel::selectCurrent() {
    if ( this->parent() == NULL )
        return;

    if ( !this->currentIndex.isValid() || !this->parent()->underMouse())
        return;

    if ( !( QApplication::keyboardModifiers() & Qt::ControlModifier ))
        this->selectionModel()->clearSelection();

    this->selectionModel()->select( this->currentIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows );
}

/**
 * @brief ContainerModel::deselectCurrent
 */
void ContainerModel::deselectCurrent() {
    if ( this->parent() == NULL )
        return;

    if ( !this->parent()->underMouse())
        return;

    if ( this->selectionModel()->selectedIndexes().contains( this->currentIndex ) && !( QApplication::keyboardModifiers() & Qt::ControlModifier ))
        return;

    this->selectionModel()->select( this->currentIndex, QItemSelectionModel::Deselect | QItemSelectionModel::Rows );
}

/**
 * @brief ContainerModel::determineMimeTypes
 */
void ContainerModel::determineMimeTypes() {    
    if ( this->mode() != FileMode )
        return;


    // TODO: must read files in batches?
    qDebug() << "mimetypes";

    // use batched  QDirIterator? from a separate thread?
    //
    // abort pending thumbnailing on directory change?

    int y, k;
    this->fileHash.clear();
    for ( y = 0; y < this->rowCount(); y++ ) {
        for ( k = 0; k < this->columnCount(); k++ ) {
            QModelIndex index;
            index = this->index( y, k );
            Entry *entry;
            entry = this->indexToEntry( index );


            // FIXME: is this even necesarry, bottleneck is opening dir and getting entrylist
           // QRect rect = this->parent()->visualRect( index );
            if ( entry != NULL /*&& this->parent()->viewport()->rect().intersects( rect )*/)
                this->fileHash.insert( entry->info().absoluteFilePath(), index );
        }
    }

    foreach ( Entry *entry, this->list ) {
        if ( entry->type() == Entry::FileFolder && !entry->info().isDir()) {
            if ( this->fileHash.contains( entry->info().absoluteFilePath()))
                m.cache->process( entry->info().absoluteFilePath());
        }
    }
}

/**
 * @brief ContainerModel::mimeTypeDetected
 * @param fileName
 * @param entry
 */
void ContainerModel::mimeTypeDetected( const QString &fileName, const DataEntry &data ) {
    QMimeDatabase mdb;
    int y;//, k;

    if ( data.mimeType.isEmpty())
        return;

    // TODO: optimize (QMap or smth, filenames to indexes)
    QList<QModelIndex> values = this->fileHash.values( fileName );
    for ( y = 0; y < values.size(); y++ ) {
        Entry *entry;

        entry = this->indexToEntry( values.at( y ));
        if ( entry != NULL ) {
            if ( !QString::compare( entry->info().absoluteFilePath(), fileName )) {
                if ( data.pixmapList.count()) {
                    entry->setIconPixmap( data.pixmapList.first());
                    entry->setType( Entry::Thumbnail );
                }

                entry->setMimeType( mdb.mimeTypeForName( data.mimeType ));
                this->parent()->update( values.at( y ));
            }
        }
    }

    /*for ( y = 0; y < this->rowCount(); y++ ) {
        for ( k = 0; k < this->columnCount(); k++ ) {
            QModelIndex index;
            Entry *entry;

            index = this->index( y, k );
            entry = this->indexToEntry( index );
            if ( entry != NULL ) {
                if ( !QString::compare( entry->info().absoluteFilePath(), fileName )) {
                    if ( data.pixmapList.count()) {
                        entry->setIconPixmap( data.pixmapList.first());
                        entry->setType( Entry::Thumbnail );
                    }

                    entry->setMimeType( mdb.mimeTypeForName( data.mimeType ));
                    this->parent()->update( index );
                }
            }
        }
    }*/
}

/**
 * @brief ContainerModel::processDropEvent
 * @param index
 * @param selectedIndexes
 * @param pos
 */
void ContainerModel::processDropEvent( const QModelIndex &index, const QPoint &pos ) {
    Entry *entry;
    QStringList items;

    entry = this->indexToEntry( index );

    if ( entry == NULL )
        return;

    foreach ( QModelIndex modelIndex, this->selection ) {
        Entry *modelEntry;

        if ( modelIndex.column() > 0 )
            continue;

        modelEntry = this->indexToEntry( modelIndex );
        if ( modelEntry != NULL )
            items << modelEntry->alias();
    }

    // a dummy menu for now
    QMenu menu;
    menu.addAction( "Copy here" );
    menu.addAction( "Move here" );
    menu.addAction( "Link here" );

    // TODO: support executables
    if ( entry != NULL ) {
        if ( !entry->isDirectory())
            m.notifications()->push( NotificationPanel::Warning, "Invalid drop", this->tr( "Cannot drop files on '%1'" ).arg( entry->alias()));
        else {
            m.notifications()->push( NotificationPanel::Information, "Drop", this->tr( "Dropping %1 files on '%2'" ).arg( items.count()).arg( entry->alias()));
            menu.exec( pos );
        }
    }
}

/**
 * @brief ContainerModel::processContextMenu
 * @param index
 * @param selectedIndexes
 * @param pos
 */
void ContainerModel::processContextMenu( const QModelIndex &index, const QPoint &pos ) {
    Entry *entry;

    entry = this->indexToEntry( index );
    if ( entry == NULL )
        return;

    // a dummy menu for now
    QMenu menu;
    menu.addAction( "Open", this, SLOT( open()));
    menu.addAction( "Open With", this, SLOT( openWith()));
    menu.addSeparator();
    menu.addAction( "Cut", this, SLOT( cut()));
    menu.addAction( "Copy", this, SLOT( copy()));
    menu.addAction( "Paste", this, SLOT( paste()));
    menu.addSeparator();
    menu.addAction( "Rename", this, SLOT( rename()));
    menu.addSeparator();
    menu.addAction( "Properties", this, SLOT( displayProperties()));
    menu.exec( pos );
}

/**
 * @brief ContainerModel::processItemOpen
 * @param index
 */
void ContainerModel::processItemOpen( const QModelIndex &index ) {
    Entry *entry;

    if ( QApplication::keyboardModifiers() & Qt::ControlModifier )
        return;

    entry = this->indexToEntry( index );
    if ( entry == NULL )
        return;

    if ( entry->isDirectory()) {
        m.gui()->setCurrentPath( entry->path());
    } else {
        if ( this->mode() == ContainerModel::FileMode )
            QDesktopServices::openUrl( QUrl::fromLocalFile( entry->path()));
    }
}

/**
 * @brief ContainerModel::displayProperties
 */
void ContainerModel::displayProperties() {
    Properties props;
    int count = 0;

    // ignore additional columns
    foreach ( QModelIndex index, this->selection ) {
        if ( index.column() > 0 )
            continue;

        count++;
    }

    if ( count == 1 ) {
        props.setEntry( this->indexToEntry( this->selection.first()));
    } else if ( count > 1 ) {
        QList <Entry*> entryList;

        foreach ( QModelIndex index, this->selection ) {
            if ( index.column() > 0 )
                continue;

            entryList << this->indexToEntry( index );
        }

        props.setEntries( entryList );
    } else {
        return;
    }

    props.exec();
    //props.resize( props.width(), props.minimumSizeHint().height());
}

/**
 * @brief ContainerModel::copy
 */
void ContainerModel::copy() {
    qDebug() << "copy";
}

/**
 * @brief ContainerModel::paste
 */
void ContainerModel::paste() {
    qDebug() << "paste";
}

/**
 * @brief ContainerModel::open
 */
void ContainerModel::open() {
    qDebug() << "open";
    this->processItemOpen( this->currentIndex );

    //foreach ( Entry *entry, this->selectionList )
    //    ();
}

/**
 * @brief ContainerModel::rename
 */
void ContainerModel::rename() {
    bool ok;
    QString fileName;
    QString current;

    // currently one selection
    if ( this->selectionList.count() > 1 )
        return;

    current = this->selectionList.first()->alias();
    fileName = QInputDialog::getText( m.gui(), this->tr( "Rename file" ), this->tr( "New filename:" ), QLineEdit::Normal, current, &ok );
    if ( ok && !fileName.isEmpty() && QString::compare( current, fileName )) {
        qDebug() << "rename simulation from" << current << "to" << fileName;
    }
}

/**
 * @brief ContainerModel::openWith
 */
void ContainerModel::openWith() {
    qDebug() << "openWith";
}

/**
 * @brief ContainerModel::cut
 */
void ContainerModel::cut() {
    foreach ( Entry *entry, this->list )
        entry->setCut( false );

    // FIXME/NOTE: must store differently because entry list is rebuild on every dir change
    foreach ( Entry *entry, this->selectionList )
        entry->setCut();
}

/**
 * @brief ContainerModel::mimeData
 * @param indexes
 * @return
 */
QMimeData *ContainerModel::mimeData( const QModelIndexList &indexes ) const {
    QMimeData *mimeData;
    Entry *entry;
    QList<QUrl>urlList;

    mimeData = QAbstractItemModel::mimeData( indexes );

    foreach ( QModelIndex index, indexes ) {
        if ( index.column() > 0 )
            continue;

        if ( index.isValid()) {
            entry = this->indexToEntry( index );
            if ( entry == NULL )
                continue;

            urlList << QUrl::fromLocalFile( entry->path());
        }
    }

    mimeData->setUrls( urlList );
    return mimeData;
}

/**
 * @brief ContainerModel::iconSize
 * @return
 */
int ContainerModel::iconSize() const {
    if ( this->container() == TableContainer )
        return Common::DefaultTableIconSize;

    return this->m_iconSize;
}
