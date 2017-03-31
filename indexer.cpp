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
#include "indexer.h"
#include "cache.h"

/**
 * @brief Indexer::work
 * @param fileName
 * @return
 */
Hash Indexer::work( const QString &fileName ) {
    quint32 hash = 0;
    QFile file( fileName );

    if ( file.size() <= 10485760 ) {
        if ( file.open( QFile::ReadOnly )) {
            hash = Cache::checksum( file.readAll(), file.size());
            file.close();
        }
    }

    return Hash( hash, file.size());
}

/**
   * @brief Indexer::run
   */
void Indexer::run() {
    // enter event loop
    while ( !this->isInterruptionRequested()) {
        // LIFO - prioritizing most recent entries
        if ( !this->workList.isEmpty()) {
            QString fileName;
            fileName = this->workList.takeLast();
            emit this->workDone( fileName, this->work( fileName ));
        }
    }
}
