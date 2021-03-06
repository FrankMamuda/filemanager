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
#include "cache.h"
#include <QInputDialog>
#include <QMimeData>
#include <QClipboard>

/**
 * @brief ContainerModel::ContainerModel
 * @param parent
 * @param mode
 * @param iconSize
 */
ContainerModel::ContainerModel( QAbstractItemView *view, Containers container ) : m_parent( view ), m_iconSize( Common::DefaultListIconSize ), m_selectionLocked( false ), m_container( container ) {
    // create rubber band
    if ( this->parent() != nullptr )
        this->m_rubberBand = new QRubberBand( QRubberBand::Rectangle, this->parent()->viewport());

    // listen to cache updates
    this->connect( m.cache, SIGNAL( finished( QString, DataEntry )), this, SLOT( mimeTypeDetected( QString, DataEntry )));
}


/**
 * @brief ContainerModel::~ContainerModel
 */
ContainerModel::~ContainerModel() {
    this->disconnect( m.cache, SIGNAL( finished( QString, DataEntry )));
    this->m_rubberBand->deleteLater();
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
    if ( this->parent() == nullptr )
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
            return nullptr;

        return this->list.at( row );
    }

    return nullptr;
}

/**
 * @brief ContainerModel::setIconSize
 * @param iconSize
 */
void ContainerModel::setIconSize( int iconSize ) {
    this->m_iconSize = iconSize;

    foreach ( Entry *entry, this->list ) {
        entry->reset();

        if ( entry->type() == Entry::Thumbnail || entry->type() == Entry::Executable )
            entry->setType( Entry::FileFolder );
    }

    this->determineMimeTypes();
}

/**
 * @brief ContainerModel::populate
 */
void ContainerModel::populate() {
    if ( m.gui() != nullptr ) {
        this->buildList( pathUtils.currentPath );
    } else
        this->buildList( QDir::currentPath());
}

/**
 * @brief ContainerModel::flags
 * @param index
 * @return
 */
Qt::ItemFlags ContainerModel::flags( const QModelIndex &index ) const {
    if ( !index.isValid())
        return Qt::NoItemFlags;

    switch ( SpecialDirectory::pathToType( pathUtils.currentPath )) {
    case SpecialDirectory::General:
        return ( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled );

    case SpecialDirectory::NoType:
#ifdef Q_OS_WIN32
    case SpecialDirectory::Root:
#endif
    case SpecialDirectory::Trash:
    case SpecialDirectory::Bookmarks:
    default:
        return Qt::ItemIsEnabled;
    }
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
    if ( entry == nullptr )
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
 * @brief ContainerModel::supportedDropActions
 * @return
 */
Qt::DropActions ContainerModel::supportedDropActions() const {
    switch ( SpecialDirectory::pathToType( pathUtils.currentPath )) {
    case SpecialDirectory::General:
        return Qt::CopyAction;

#ifdef Q_OS_WIN32
    case SpecialDirectory::Root:
#endif
    case SpecialDirectory::NoType:
    case SpecialDirectory::Trash:
    case SpecialDirectory::Bookmarks:
    default:
        return Qt::IgnoreAction;
    }
}

/**
 * @brief ContainerModel::supportedDragActions
 * @return
 */
Qt::DropActions ContainerModel::supportedDragActions() const {
    return this->supportedDropActions();
}

/**
 * @brief ContainerModel::buildList
 * @param path
 */
void ContainerModel::buildList( const QString &path ) {
    QFileInfoList infoList;
    QDir directory;

    // clear previous list
    qDeleteAll( this->list );
    this->list.clear();

    // get filelist
    switch ( SpecialDirectory::pathToType( path )) {
    case SpecialDirectory::General:
        directory.setPath( PathUtils::toWindowsPath( path ));
        infoList = directory.entryInfoList( QDir::NoDotAndDotDot | QDir::AllEntries, QDir::IgnoreCase | QDir::DirsFirst );
        foreach ( QFileInfo info, infoList )
            this->list << new Entry( Entry::FileFolder, info, this );
        break;

#ifdef Q_OS_WIN32
    case SpecialDirectory::Root:
        // add root pseudo-folder
        this->list << new Entry( Entry::Root, QFileInfo(), this );
        this->list << new Entry( Entry::Home, QFileInfo( QDir::home().absolutePath()), this );

        // build new storage list
        infoList = QDir::drives();
        foreach ( QFileInfo driveInfo, infoList )
            this->list << new Entry( Entry::HardDisk, driveInfo, this );

        this->list << new Entry( Entry::Trash, QFileInfo(), this );
        break;
#endif

    case SpecialDirectory::NoType:
    case SpecialDirectory::Trash:
    case SpecialDirectory::Bookmarks:
    default:
        break;
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
            if ( entry != nullptr )
                this->selectionList << entry;
        }
    }
}

/**
 * @brief ContainerModel::processTextDisplay
 */
void ContainerModel::processEntries() {
    int y;

    if ( this->parent() == nullptr )
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
        if ( view == nullptr )
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

    if ( this->parent() == nullptr )
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

    if ( this->parent() == nullptr )
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

    if ( this->parent() == nullptr )
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

    if ( this->parent() == nullptr )
        return;

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
    if ( this->parent() == nullptr )
        return;

    if ( !this->currentIndex.isValid() || !this->parent()->underMouse())
        return;

    if ( !( QApplication::keyboardModifiers() & Qt::ControlModifier ))
        this->selectionModel()->clearSelection();

    this->parent()->setCurrentIndex( this->currentIndex );
    this->selectionModel()->select( this->currentIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows );
}

/**
 * @brief ContainerModel::deselectCurrent
 */
void ContainerModel::deselectCurrent() {
    if ( this->parent() == nullptr )
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
    QModelIndex index;
    QRect rect;
    Entry *entry;
    int y, k;//, z = 0;

    if ( SpecialDirectory::pathToType( pathUtils.currentPath ) != SpecialDirectory::General )
        return;

    // TODO: must read files in batches via QDirIterator from a separate thread?

    // clean up
    m.cache->stop();
    this->fileHash.clear();

    // build file hash
    for ( y = 0; y < this->rowCount(); y++ ) {
        for ( k = 0; k < this->columnCount(); k++ ) {
            index = this->index( y, k );
            entry = this->indexToEntry( index );

            if ( entry->isUpdated() || entry->info().isDir() || entry->type() != Entry::FileFolder || entry->info().fileName().endsWith( ".cache" ))
                continue;

            rect = this->parent()->visualRect( index );
            if ( entry != nullptr && this->parent()->viewport()->rect().intersects( rect )) {
                this->fileHash.insert( entry->path(), index );
                m.cache->process( entry->path());
                //z++;
            }
        }
    }

    // report
    //if ( z > 0 )
    //    qDebug() << "about to detect mimetypes of" << z << "files";
}

/**
 * @brief ContainerModel::mimeTypeDetected
 * @param fileName
 * @param entry
 */
void ContainerModel::mimeTypeDetected( const QString &fileName, const DataEntry &data ) {
    QMimeDatabase mdb;
    int y, index;

    // FIXME: this could be called from another thread
    //        while the list is empty? could this be a potential crash
    //        reason?

    // no updates for invalid mimetypes and non-active widgets
    if ( data.mimeType.isEmpty() || !this->parent()->isVisible())
        return;

    QList<QModelIndex> values = this->fileHash.values( fileName );
    for ( y = 0; y < values.size(); y++ ) {
        Entry *entry;

        entry = this->indexToEntry( values.at( y ));
        if ( entry != nullptr ) {
            if ( !QString::compare( entry->path(), fileName )) {
                if ( data.pixmapList.count() == 4 ) {
                    index = this->iconSize() / 16;
                    index = 4 - index;

                    if ( index < 0 )
                        index = 0;
                    else if ( index > 3 )
                        index = 3;

                    entry->setIconPixmap( data.pixmapList.at( index ));

                    if ( entry->info().fileName().endsWith( ".exe" ))
                        entry->setType( Entry::Executable );
                    else
                        entry->setType( Entry::Thumbnail );
                }

                entry->setMimeType( mdb.mimeTypeForName( data.mimeType ));
                entry->setUpdated( true );
                this->parent()->update( values.at( y ));
            }
        }
    }
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

    if ( entry == nullptr )
        return;

    foreach ( QModelIndex modelIndex, this->selection ) {
        Entry *modelEntry;

        if ( modelIndex.column() > 0 )
            continue;

        modelEntry = this->indexToEntry( modelIndex );
        if ( modelEntry != nullptr )
            items << modelEntry->alias();
    }

    // a dummy menu for now
    QMenu menu;
    menu.addAction( "Copy here" );
    menu.addAction( "Move here" );
    menu.addAction( "Link here" );

    // TODO: support executables
    if ( entry != nullptr ) {
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
    if ( entry == nullptr )
        return;

    // a dummy menu for now
    QMenu menu;

    // open only single files and dirs
    if ( this->selectionList.count() == 1 ) {
        menu.addAction( "Open", this, SLOT( open()));

        if ( !entry->isDirectory())
            menu.addAction( "Open With", this, SLOT( openWith()));

        menu.addSeparator();
    }

    // cut and copy multiple files and dirs
    menu.addAction( "Cut", this, SLOT( cut()));
    menu.addAction( "Copy", this, SLOT( copy()));

    // paste only to single directories
    if ( entry->isDirectory() && this->selectionList.count() == 1 )
        menu.addAction( "Paste", this, SLOT( paste()));

    menu.addSeparator();
    menu.addAction( "Recycle", this, SLOT( recycle()));
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
    if ( entry == nullptr )
        return;

    if ( entry->isDirectory()) {
        m.gui()->setCurrentPath( entry->path());
    } else {
        switch ( SpecialDirectory::pathToType( pathUtils.currentPath )) {
        case SpecialDirectory::General:
            QDesktopServices::openUrl( QUrl::fromLocalFile( entry->path()));
            return;

#ifdef Q_OS_WIN32
        case SpecialDirectory::Root:
#endif
        case SpecialDirectory::NoType:
        case SpecialDirectory::Trash:
        case SpecialDirectory::Bookmarks:
        default:
            return;
        }
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
    QMimeData *mimeData;
    QList<QUrl> pathList;

    // NOTE: use clipboard at all?
    // just offload this to YET*TO*BE*CREATED fileUtils
    // and add fileSystemWatcher to prevent changes

    // build file list
    foreach ( Entry *entry, this->selectionList )
        pathList << QUrl( entry->info().absoluteFilePath());

    // create mime data
    mimeData = new QMimeData();
    mimeData->setUrls( pathList );

    // add data to clipboard
    QApplication::clipboard()->setMimeData( mimeData );
}

/**
 * @brief ContainerModel::paste
 */
void ContainerModel::paste() {
    if ( !QApplication::clipboard()->mimeData()->hasUrls())
        return;

    qDebug() << "paste:";
    foreach ( QUrl url, QApplication::clipboard()->mimeData()->urls()) {
        qDebug() << " " << url.toString();
    }
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
 * @brief ContainerModel::recycle
 */
void ContainerModel::recycle() {

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

    //this->copy;
    //this.
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
            if ( entry == nullptr )
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

/**
 * @brief SpecialDirectory::pathToType
 * @param path
 * @return
 */
SpecialDirectory::Types SpecialDirectory::pathToType( const QString &path ) {
    SpecialDirectory::Types type = SpecialDirectory::General;

    // determine directory type
    foreach ( SpecialDirectory sd, ContainerNamespace::SpecialDirectories ) {
        if ( /*path.startsWith( sd.path )*/ !QString::compare( path, sd.path )) {
            type = sd.type;
            break;
        }
    }

    return type;
}
