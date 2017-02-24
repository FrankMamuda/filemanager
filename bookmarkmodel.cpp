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
#include "main.h"
#include "mainwindow.h"
#include <QApplication>

/**
 * @brief BookmarkModel::rowCount
 * @return
 */
int BookmarkModel::rowCount( const QModelIndex & ) const {
    return Bookmark::count();
}

/**
 * @brief BookmarkModel::data
 * @param index
 * @param role
 * @return
 */
QVariant BookmarkModel::data( const QModelIndex &modelIndex, int role ) const {
    switch ( role )  {
    case Qt::DecorationRole:
        return QIcon::fromTheme( Bookmark::value( modelIndex.row(), Bookmark::IconName )).pixmap( 32, 32 );

    case Qt::DisplayRole:
        return Bookmark::value( modelIndex.row(), Bookmark::Alias );
    }

    return QVariant();
}

/**
 * @brief Bookmark::processItemOpen
 * @param index
 */
void BookmarkModel::processItemOpen( const QModelIndex &index ) {
    if ( QApplication::keyboardModifiers() & Qt::ControlModifier )
        return;

    m.gui()->setCurrentPath( Bookmark::value( index.row(), Bookmark::Path ));
}
