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
#include "sideview.h"
#include "bookmarkmodel.h"
#include "bookmark.h"
#include "main.h"
#include "listviewdelegate.h"
#include "mainwindow.h"
#include <QApplication>
#include <QMenu>
#include <QDebug>
#include <QInputDialog>
#include <QMimeData>
#include "containerstyle.h"
#include "notificationpanel.h"
#include "iconbrowser.h"
#include "pixmapcache.h"

/**
 * @brief SideView::SideView
 * @param parent
 */
SideView::SideView( QWidget* parent ) : QListView( parent ), m_model( new BookmarkModel( this )) {
    this->setModel( this->m_model );
    this->setStyleSheet( "QListView { background-color: transparent; }" );
    this->connect( this, SIGNAL( clicked( QModelIndex )), this, SLOT( processItemOpen( QModelIndex )));

    // set view delegate
    this->m_delegate = new ListViewDelegate( this );
    this->setItemDelegate( this->delegate());

    // NOTE: for some reason drops aren't accepted without the ugly drop indicator
    // while it is enabled, we do however abstain from painting it
    this->m_style = new ContainerStyle( this->style());
    this->setStyle( this->m_style );

    // TODO: set in UI
    this->setDragEnabled( true );
    this->setDropIndicatorShown( true );
    this->setAcceptDrops( true );
    this->setSelectionMode( QAbstractItemView::SingleSelection );
    this->setDragDropMode( QAbstractItemView::InternalMove );

    // filter events for dragging
    this->setMouseTracking( true );
}

/**
 * @brief SideView::~SideView
 */
SideView::~SideView() {    
    this->disconnect( this, SIGNAL( clicked( QModelIndex )));
    this->m_delegate->deleteLater();
    this->m_model->deleteLater();
    this->m_style->deleteLater();
}

/**
 * @brief SideView::setModel
 * @param model
 */
void SideView::setModel( BookmarkModel *model ) {
    this->m_model = model;
    QListView::setModel( model );
}

/**
 * @brief SideView::mouseReleaseEvent
 */
void SideView::mouseReleaseEvent( QMouseEvent *e ) {
    QModelIndex index;

    index = this->indexAt( e->pos());
    if ( e->button() == Qt::RightButton && index.isValid())
        this->processContextMenu( index, this->mapToGlobal( e->pos()));

    QListView::mouseReleaseEvent( e );
}

/**
 * @brief SideView::dropEvent
 */
void SideView::dropEvent( QDropEvent *e ) {
    QList<QUrl> urls = e->mimeData()->urls();
    QString path;
    QFileInfo info;
    int y;

    // construct drop-in-between regions
    QRegion region( this->viewport()->rect());
    for ( y = 0; y < this->model()->rowCount(); y++ )
        region -= this->rectForIndex( this->model()->index( y ));

    // detect bookmark drop
    // handle differently since this is internal move
    if ( e->mimeData()->text().startsWith( "bookmark" )) {
        if ( this->currentDragRow == -1 )
            return;

        // determine if we are dropping item in between two others
        for ( y = 0; y < region.rects().size(); y++ ) {
            if ( region.rects().at( y ).contains( this->mapFromGlobal( QCursor::pos()))) {
                this->model()->bookmarks()->move( this->currentDragRow, y );
                break;
            }
        }

        // reset model
        this->model()->reset();
        return;
    }

    // make sure we only add one bookmark
    if ( urls.count() > 1 ) {
        m.notifications()->push( NotificationPanel::Error, "Bookmarks", "Can add only one bookmark at a time" );
        return;
    } else if ( !urls.count()) {
        return;
    }

    // accept only folders, ignore files
    path = urls.first().toLocalFile();
    info.setFile( path );
    if ( !info.isDir()) {
        m.notifications()->push( NotificationPanel::Error, "Bookmarks", "Cannot add files as bookmarks" );
        return;
    }

    // determine position
    for ( y = 0; y < region.rects().size(); y++ ) {
        if ( region.rects().at( y ).contains( this->mapFromGlobal( QCursor::pos()))) {
            this->model()->bookmarks()->add( info.fileName(), info.absoluteFilePath(), QPixmap(), "inode-directory", true, y + 1 );
            break;
        }
    }

    // reset model
    this->model()->reset();
    e->accept();
}

/**
 * @brief SideView::paintEvent
 */
void SideView::paintEvent( QPaintEvent *event ) {
    if ( QApplication::mouseButtons() & Qt::LeftButton ) {
        QPainter painter( this->viewport());
        QRegion region( this->viewport()->rect());
        int y;

        painter.save();
        painter.setBrush( QColor::fromRgb( 255, 255, 255, 64 ));

        // subtract item rects
        for ( y = 0; y < this->model()->rowCount(); y++ )
            region -= this->rectForIndex( this->model()->index( y ));;

        for ( y = 0; y < region.rects().size(); y++ ) {
            QRect rect;

            rect = region.rects().at( y );
            if ( rect.contains( this->mapFromGlobal( QCursor::pos()))) {
                 rect.setHeight( rect.height() - 2 );
                 painter.drawRoundedRect( rect, 2, 2 );
            }
        }

        painter.restore();
    }

    QListView::paintEvent( event );
}

/**
 * @brief SideView::dragEnterEvent
 */
void SideView::dragEnterEvent( QDragEnterEvent *event ) {
    this->currentDragRow = this->currentIndex().row();
    QListView::dragEnterEvent( event );
}

/**
 * @brief SideView::dragLeaveEvent
 * @param event
 */
void SideView::dragLeaveEvent( QDragLeaveEvent *event ) {
    this->currentDragRow = -1;
    QListView::dragLeaveEvent( event );
}

/**
 * @brief SideView::processItemOpen
 * @param index
 */
void SideView::processItemOpen( const QModelIndex &index ) {
    if ( QApplication::keyboardModifiers() & Qt::ControlModifier )
        return;

    m.gui()->setCurrentPath( this->model()->bookmarks()->value( index.row(), Bookmark::Path ).toString());
}

/**
 * @brief SideView::processContextMenu
 * @param index
 * @param pos
 */
void SideView::processContextMenu( const QModelIndex &index, const QPoint &pos ) {
    QMenu menu;

    menu.addAction( this->tr( "Rename" ), this, SLOT( renameBookmark()));
    menu.addAction( this->tr( "Change icon" ), this, SLOT( changeBookmarkIcon()));
    menu.addAction( this->tr( "Change target" ), this, SLOT( changeBookmarkTarget()));
    menu.addSeparator();
    menu.addAction( this->tr( "Remove bookmark" ), this, SLOT( removeBookmark()));
    //menu.addSeparator();
    //menu.addAction( this->tr( "Default bookmarks" ), this, SLOT( rebuildIcons()));
    menu.exec( pos );

    this->currentIndex() = index;
}

/**
 * @brief SideView::renameBookmark
 */
void SideView::renameBookmark() {
    bool ok;
    QString alias;

    alias = QInputDialog::getText( m.gui(), this->tr( "Rename bookmark" ), this->tr( "New alias:" ), QLineEdit::Normal, this->model()->bookmarks()->value( this->currentIndex().row(), Bookmark::Alias ).toString(), &ok );
    if ( ok && !alias.isEmpty()) {
        this->model()->bookmarks()->setValue( this->currentIndex().row(), Bookmark::Alias, alias );
        this->model()->reset();
    }
}

/**
 * @brief SideView::changeBookmarkTarget
 */
void SideView::changeBookmarkTarget() {
    bool ok;
    QString target;

    target = QInputDialog::getText( m.gui(), this->tr( "Change bookmark target" ), this->tr( "New path:" ), QLineEdit::Normal, this->model()->bookmarks()->value( this->currentIndex().row(), Bookmark::Path ).toString(), &ok );
    if ( ok && !target.isEmpty()) {
        this->model()->bookmarks()->setValue( this->currentIndex().row(), Bookmark::Path, target );
        this->model()->reset();
    }
}

/**
 * @brief SideView::removeBookmark
 */
void SideView::removeBookmark() {
    this->model()->bookmarks()->remove( this->currentIndex().row());
    this->model()->reset();
}

/**
 * @brief SideView::changeBookmarkIcon
 */
void SideView::changeBookmarkIcon() {
    IconBrowser ibDialog;

    if ( ibDialog.exec() == QDialog::Accepted ) {
        QPixmap pixmap;

        pixmap = ibDialog.selectedPixmap();
        if ( pixmap.isNull())
            return;

        this->model()->bookmarks()->setValue( this->currentIndex().row(), Bookmark::Stock, "" );
        this->model()->bookmarks()->setValue( this->currentIndex().row(), Bookmark::Pixmap, pixmap.scaledToHeight( this->iconSize().width(), Qt::SmoothTransformation ));
    }
}
