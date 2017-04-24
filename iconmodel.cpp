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
#include "iconmodel.h"
#include "main.h"
#include "iconcache.h"
#include "pixmapcache.h"
#include <QMimeDatabase>
#include <QIcon>
#include <QListView>
#include <QDebug>

/**
 * @brief IconModel::IconModel
 * @param parent
 */
IconModel::IconModel( QObject *listView ) {
    // set parent
    this->parent = qobject_cast<QListView *>( listView );
    if ( this->parent == NULL )
        return;

    // listen to cache updates
    this->connect( m.iconCache, SIGNAL( finished( QString, quint8, QPixmap )), this, SLOT( iconFetched( QString, quint8, QPixmap )));
    this->connect( m.iconCache, SIGNAL( update()), this, SLOT( fetch()));

   // // reset model
    this->pixmapList.clear();
    QMetaObject::invokeMethod( m.iconCache, "requestUpdate", Qt::QueuedConnection );

}

/**
 * @brief IconModel::~IconModel
 */
IconModel::~IconModel() {
    this->clear();
    this->parent = NULL;
}

/**
 * @brief IconModel::clear
 */
void IconModel::clear() {
    QMetaObject::invokeMethod( m.iconCache, "clear", Qt::QueuedConnection );
    this->iconNames.clear();
    this->pixmapList.clear();
}

/**
 * @brief IconModel::data
 * @param index
 * @param role
 * @return
 */
QVariant IconModel::data( const QModelIndex &index, int role ) const {
    if ( !index.isValid())
        return QVariant();

    if ( role == Qt::DecorationRole && index.row() >= 0 && index.row() < this->pixmapList.count())
        return this->pixmapList.at( index.row());

    if ( role == Qt::DisplayRole && index.row() >= 0 && index.row() < this->iconNames.count())
        return this->iconNames.at( index.row());

    return QVariant();
}

/**
 * @brief IconModel::addIcon
 * @param iconName
 * @param pixmap
 */
void IconModel::addIcon( const QString &iconName, const QPixmap &pixmap ) {
    QModelIndex index;

    index = this->createIndex( this->pixmapList.count(), 0 );
    this->beginInsertRows( index, this->pixmapList.count(), this->pixmapList.count()+1 );
    this->insertRow( this->pixmapList.count(), index );
    this->pixmapList << pixmap;
    this->iconNames << iconName;
    this->endInsertRows();
}

/**
 * @brief IconModel::iconFetched
 * @param iconName
 * @param iconScale
 * @param pixmap
 */
void IconModel::iconFetched( const QString &iconName, quint8, const QPixmap &pixmap ) {
    this->addIcon( iconName, pixmap );
}

/**
 * @brief IconModel::reset
 */
void IconModel::reset() {    
    this->beginResetModel();
    this->endResetModel();
}

/**
 * @brief IconModel::fetch
 */
void IconModel::fetch() {
    QMimeDatabase db;

    foreach ( QMimeType mimeType, db.allMimeTypes())
        QMetaObject::invokeMethod( m.iconCache, "process", Qt::QueuedConnection, Q_ARG( QString, mimeType.iconName()), Q_ARG( quint8, 48 ));
}
