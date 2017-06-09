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
#include <QIcon>
#include "iconfetcher.h"
#include "pixmapcache.h"
#include "main.h"

/**
 * @brief IconFetcher::addWork
 * @param iconName
 * @param iconScale
 */
void IconFetcher::addWork( const QString &iconName, quint8 iconScale ) {
    QMutexLocker( &this->m_mutex );
    IconIndex index;

    index = IconIndex( iconName, iconScale );
    if ( !this->workList.contains( index ))
        this->workList << index;
}

/**
 * @brief IconFetcher::run
 */
void IconFetcher::run() {
    // enter event loop
    while ( !this->isInterruptionRequested()) {
        QMutexLocker( &this->m_mutex );

        // LIFO - prioritizing most recent entries
        if ( !this->workList.isEmpty()) {
            IconIndex index;
            QPixmap pixmap;

            index = this->workList.takeLast();
            pixmap = m.pixmapCache->pixmap( index.first, index.second );

            if ( !pixmap.isNull() && pixmap.width())
                emit this->workDone( index.first, index.second, pixmap );
        } else {
            msleep( 100 );
        }
    }
}
