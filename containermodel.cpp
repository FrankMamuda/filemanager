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

/**
 * @brief ContainerModel::ContainerModel
 * @param parent
 * @param mode
 * @param iconSize
 */
ContainerModel::ContainerModel( QAbstractItemView *parent, Containers container, ContainerModel::Modes mode, int iconSize ) : m_listParent( parent ), m_container( container ), m_mode( mode ), m_iconSize( iconSize ) {
    this->connect( &this->futureWatcher, SIGNAL( resultReadyAt( int )), this, SLOT( mimeTypeDetected( int )));
    this->connect( this, SIGNAL( stop()), &this->futureWatcher, SLOT( cancel()));
    this->connect( qApp, SIGNAL( aboutToQuit()), this, SLOT( quit()));

    // create rubber band
    this->m_rubberBand = new QRubberBand( QRubberBand::Rectangle, this->listParent()->viewport());
}

/**
 * @brief ContainerModel::quit
 */
void ContainerModel::quit() {
    this->disconnect( &this->futureWatcher, SIGNAL( resultReadyAt( int )));
    this->future.cancel();
    this->futureWatcher.cancel();
    this->future.waitForFinished();
    this->futureWatcher.waitForFinished();
}

/**
 * @brief ContainerModel::~ContainerModel
 */
ContainerModel::~ContainerModel() {
    this->m_rubberBand->deleteLater();
    qDeleteAll( this->workList );
}

/**
 * @brief ContainerModel::reset
 */
int ContainerModel::columnCount( const QModelIndex & ) const {
    if ( this->container() == ListContainer )
        return 1;
    else if ( this->container() == TableContainer )
        return 4;

    return 0;
}

void ContainerModel::reset( bool force ) {
    if ( !this->listParent()->isVisible() && !force )
        return;

    this->processEntries();
    this->determineMimeTypes();
    this->beginResetModel();
    this->endResetModel();
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
    if ( this->m_iconSize == iconSize )
        return;

    this->m_iconSize = iconSize;

    foreach ( Entry *entry, this->list ) {
        entry->reset();

        if ( entry->type() == Entry::Thumbnail )
            entry->setType( Entry::FileFolder );
    }

    // recache images
    qDebug() << "# ICON SIZE CHANGE";
    this->determineMimeTypes();
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

    if ( modelIndex.column() == 0 ) {
        switch ( role )  {
        case Qt::DecorationRole:
            return entry->pixmap( this->iconSize());

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
 * @brief ContainerModel::processTextDisplay
 */
void ContainerModel::processEntries() {
    int y;

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
        view = qobject_cast<QListView*>( this->listParent());
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

    this->rubberBand()->hide();

    index = this->listParent()->indexAt( e->pos());
    if ( e->button() == Qt::RightButton ) {
        if ( index.isValid())
            this->processContextMenu( index, this->listParent()->mapToGlobal( e->pos()) );
        else
            this->listParent()->selectionModel()->clear();
    }
}

/**
 * @brief ContainerModel::processMouseMove
 * @param selection
 */
void ContainerModel::processMouseMove( QMouseEvent *e ) {
    QModelIndex index;

    this->currentMousePos = e->pos();
    this->updateRubberBand();

    if ( !this->rubberBand()->isVisible()) {
        index = this->listParent()->indexAt( e->pos());

        if ( index.isValid() && this->currentIndex.row() != index.row() && !( QApplication::mouseButtons() & Qt::LeftButton )) {
            this->selectionTimer.stop();

            if ( !this->listParent()->selectionModel()->isSelected( index ))
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

    index = this->listParent()->indexAt( e->pos());
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

    if ( !this->rubberBand()->isHidden()) {
        origin = this->selectionOrigin;
        origin.setY( this->selectionOrigin.y() - this->verticalOffset());
        this->rubberBand()->setGeometry( QRect( origin, this->currentMousePos ).normalized());

        // manually update selection
        for ( y = 0; y < this->rowCount(); y++ ) {
            for ( k = 0; k < this->columnCount(); k++ ) {
                QModelIndex index;

                index = this->index( y, k );
                if ( this->rubberBand()->geometry().intersects( this->listParent()->visualRect( index )))
                    this->listParent()->selectionModel()->select( index, QItemSelectionModel::Select | QItemSelectionModel::Rows );
                else
                    this->listParent()->selectionModel()->select( index, QItemSelectionModel::Deselect | QItemSelectionModel::Rows );
            }
        }
    }
}

/**
 * @brief ContainerModel::selectCurrent
 */
void ContainerModel::selectCurrent() {
    if ( !this->currentIndex.isValid() || !this->listParent()->underMouse())
        return;

    if ( !( QApplication::keyboardModifiers() & Qt::ControlModifier ))
        this->listParent()->selectionModel()->clearSelection();

    this->listParent()->selectionModel()->select( this->currentIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows );
}

/**
 * @brief ContainerModel::deselectCurrent
 */
void ContainerModel::deselectCurrent() {
    if ( !this->listParent()->underMouse())
        return;

    if ( this->listParent()->selectionModel()->selectedIndexes().contains( this->currentIndex ) && !( QApplication::keyboardModifiers() & Qt::ControlModifier ))
        return;

    this->listParent()->selectionModel()->select( this->currentIndex, QItemSelectionModel::Deselect | QItemSelectionModel::Rows );
}

/**
 * @brief ContainerModel::determineMimeTypes
 */
void ContainerModel::determineMimeTypes() {
    int y = 0;

    emit this->stop();

    qDebug() << "#### DETERMINE MIME TYPES";

    foreach ( ASyncWorker *worker, this->workList ) {
        if ( worker != NULL )
            delete worker;
    }
    this->workList.clear();

    //qDeleteAll( this->workList );
    foreach ( Entry *entry, this->list ) {
        if ( entry->type() == Entry::FileFolder )
            this->workList << new ASyncWorker( entry->info(), y, this->iconSize());

        y++;
    }

    this->future = QtConcurrent::mapped( this->workList, determineMimeTypeAsync );
    this->futureWatcher.setFuture( this->future );
}

/**
 * @brief ContainerModel::mimeTypeDetected
 * @param index
 */
void ContainerModel::mimeTypeDetected( int index ) {
    Entry *entry;
    ASyncWorker *worker;
    int y;
    bool update = false;

    // update entry in view with corresponding icon or thumbnail
    worker = this->workList.at( index );
    entry = this->list.at( worker->index());
    if ( worker != NULL && entry != NULL ) {
        if ( worker->info() == entry->info()) {
            if ( worker->mimeType() != entry->mimeType()) {
                 entry->setMimeType( worker->mimeType());
                 update = true;
            }

            if ( worker->update()) {
                entry->setIconName( worker->iconName());
                entry->setType( Entry::Thumbnail );
                update = true;
            }

            if ( this->listParent() != NULL && update ) {
                for ( y = 0; y < this->columnCount(); y++ )
                    this->listParent()->update( this->index( worker->index(), y ));
            }
        }
    }
}

/**
 * @brief ContainerModel::determineMimeTypeAsync
 * @param worker
 * @return
 */
ASyncWorker *ContainerModel::determineMimeTypeAsync( ASyncWorker *worker ) {
    QMimeDatabase m;
    QPixmap pm;

    // get mimetype from contents
    worker->setMimeType( m.mimeTypeForFile( worker->info(), QMimeDatabase::MatchContent ));

    // precache thumbnail
    if ( worker->mimeType().iconName().startsWith( "image" )) {
        pm = pixmapCache.pixmap( worker->info().absolutePath(), worker->iconSize(), true );
        if ( pm.width() && pm.height()) {
            worker->scheduleUpdate();
            worker->setIconName( worker->info().absoluteFilePath());
        }
    }

    return worker;
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
            qDebug() << "invalid drop";
        else {
            qDebug() << "drop " << items.join( " " ) << "on" << entry->alias();
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
    menu.addAction( "Open" );
    menu.addAction( "Open With" );
    menu.addSeparator();
    menu.addAction( "Cut" );
    menu.addAction( "Copy" );
    menu.addSeparator();
    menu.addAction( "Properties" );
    this->connect( &menu, SIGNAL( triggered( QAction*)), SLOT( displayProperties()));
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
