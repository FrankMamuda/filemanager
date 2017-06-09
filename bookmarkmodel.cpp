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
#include "bookmarkmodel.h"
#include "bookmark.h"
#include "sideview.h"
#include <QMimeData>
#include "pixmapcache.h"
#include "mainwindow.h"
#include "main.h"

/**
 * @brief BookmarkModel::BookmarkModel
 * @param parent
 */
BookmarkModel::BookmarkModel( QAbstractItemView *parent ) : m_parent( parent ) {
    this->m_bookmarks = new Bookmark( QDir::currentPath() + "/.config", parent );
}

/**
 * @brief BookmarkModel::~BookmarkModel
 */
BookmarkModel::~BookmarkModel() {
    this->m_bookmarks->shutdown();
    this->m_bookmarks->deleteLater();
}

/**
 * @brief BookmarkModel::rowCount
 * @return
 */
int BookmarkModel::rowCount( const QModelIndex & ) const {
    return this->bookmarks()->count();
}

/**
 * @brief BookmarkModel::data
 * @param index
 * @param role
 * @return
 */
QVariant BookmarkModel::data( const QModelIndex &modelIndex, int role ) const {
    QAbstractItemView *parentView;
    QPixmap pixmap;
    QString stockIcon;

    parentView = qobject_cast<QAbstractItemView *>( this->parent());
    if ( parentView == nullptr )
        return QVariant();

    switch ( role )  {
    case Qt::DecorationRole:
        stockIcon = this->bookmarks()->list.at( modelIndex.row()).stockIcon;

        if ( stockIcon.isEmpty() )
            pixmap = this->bookmarks()->list.at( modelIndex.row()).pixmap;
        else
            pixmap = m.pixmapCache->pixmap( stockIcon, parentView->iconSize().width(), Ui::darkIconTheme );

        if ( !pixmap.isNull())
            return pixmap.scaled( parentView->iconSize(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
        else
            return QVariant();

    case Qt::DisplayRole:
        return this->bookmarks()->list.at( modelIndex.row()).alias;
    }

    return QVariant();
}

/**
 * @brief BookmarkModel::flags
 * @param index
 * @return
 */
Qt::ItemFlags BookmarkModel::flags( const QModelIndex &index ) const {
    if ( index.isValid())
        return ( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled );

    // allow invalid indexes to put bookmarks "in-between"
    // TODO: currently not supported
    return Qt::ItemIsDropEnabled;
}

/**
 * @brief BookmarkModel::mimeData
 * @param indexes
 * @return
 */
QMimeData *BookmarkModel::mimeData( const QModelIndexList &indexes ) const {
    QMimeData *mimeData;
    mimeData = QAbstractItemModel::mimeData( indexes );
    mimeData->setText( "bookmark" );
    mimeData->setObjectName( "bookmark" );
    return mimeData;
}
