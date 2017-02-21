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
#include "storagelistmodel.h"

/**
 * @brief StorageListModel::rowCount
 * @param index
 * @return
 */
int StorageListModel::rowCount( const QModelIndex & ) const {
    if ( this->gui == NULL )
        return 0;

    return this->gui->storageList.count();
}

/**
 * @brief StorageListModel::data
 * @param index
 * @param role
 * @return
 */
QVariant StorageListModel::data( const QModelIndex &index, int role ) const {
    int storageIndex;
    FileSystemEntry *storage;

    if ( index.isValid() && this->gui != NULL ) {
        storageIndex = index.row();

        if ( storageIndex < 0 || storageIndex >= this->gui->storageList.count())
            return QVariant();

        storage = this->gui->storageList.at( storageIndex );
        if ( storage == NULL )
            return QVariant();

        switch ( role )  {
        case Qt::DecorationRole:
            return storage->pixmap( 32 );

        case Qt::DisplayRole:
            return storage->alias();
        }
    }

    return QVariant();
}

/**
 * @brief StorageListModel::reset
 */
void StorageListModel::reset() {
    this->beginResetModel();
    this->endResetModel();
}
