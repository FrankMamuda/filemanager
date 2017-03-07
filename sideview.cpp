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

/**
 * @brief SideView::SideView
 * @param parent
 */
SideView::SideView( QWidget* parent ) : QListView( parent ), m_model( new BookmarkModel( this )) {
    this->setModel( this->m_model );
    this->setStyleSheet( "QListView { background-color: transparent; }" );
    this->connect( this, SIGNAL( clicked( QModelIndex )), this, SLOT( processItemOpen( QModelIndex )));

    // set view delegate
    // TODO: delete me
    this->setItemDelegate( new ListViewDelegate( this ));

    // NOTE: for some reason drops aren't accepted without the ugly drop indicator
    // while it is enabled, we do however abstain from painting it
    this->m_style = new ContainerStyle( this->style());
    this->setStyle( this->m_style );
}

/**
 * @brief SideView::~SideView
 */
SideView::~SideView() {
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

    // TODO: get icon from mimetype? or .dekstop entry?

    if ( urls.count() > 1 ) {
        m.notifications()->push( NotificationPanel::Error, "Bookmarks", "Can add only one bookmark at a time" );
        return;
    } else if ( !urls.count()) {
        return;
    }

    path = urls.first().toLocalFile();
    info.setFile( path );
    if ( !info.isDir()) {
        m.notifications()->push( NotificationPanel::Error, "Bookmarks", "Cannot add files as bookmarks" );
        return;
    }

    Bookmark::add( info.fileName(), info.absoluteFilePath(), "inode-directory" );
    this->model()->reset();
    e->accept();
}

/**
 * @brief SideView::processItemOpen
 * @param index
 */
void SideView::processItemOpen( const QModelIndex &index ) {
    if ( QApplication::keyboardModifiers() & Qt::ControlModifier )
        return;

    m.gui()->setCurrentPath( Bookmark::value( index.row(), Bookmark::Path ));
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
    menu.exec( pos );

    this->currentIndex() = index;
}

/**
 * @brief SideView::renameBookmark
 */
void SideView::renameBookmark() {
    bool ok;
    QString alias;

    alias = QInputDialog::getText( m.gui(), this->tr( "Rename bookmark" ), this->tr( "New alias:" ), QLineEdit::Normal, Bookmark::value( this->currentIndex().row(), Bookmark::Alias ), &ok );
    if ( ok && !alias.isEmpty()) {
        Bookmark::setValue( this->currentIndex().row(), Bookmark::Alias, alias );
        this->model()->reset();
        //this->update( this->currentIndex());
    }
}


/**
 * @brief SideView::changeBookmarkTarget
 */
void SideView::changeBookmarkTarget() {
    bool ok;
    QString target;

    target = QInputDialog::getText( m.gui(), this->tr( "Change bookmark target" ), this->tr( "New path:" ), QLineEdit::Normal, Bookmark::value( this->currentIndex().row(), Bookmark::Path ), &ok );
    if ( ok && !target.isEmpty()) {
        Bookmark::setValue( this->currentIndex().row(), Bookmark::Path, target );
        this->model()->reset();
    }
}

/**
 * @brief SideView::removeBookmark
 */
void SideView::removeBookmark() {
    m.notifications()->push( NotificationPanel::Warning, "Bookmarks", "Bookmark removal not supported yet" );
}

/**
 * @brief SideView::changeBookmarkIcon
 */
void SideView::changeBookmarkIcon() {
    m.notifications()->push( NotificationPanel::Warning, "Bookmarks", "Icon change not supported yet" );
}
