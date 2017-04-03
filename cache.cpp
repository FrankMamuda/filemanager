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
#include <QDebug>
#include <QThread>
#include "cache.h"
#include "worker.h"
#include "indexer.h"

/*
  The Cache Subsystem

  OVERVIEW:
    precaches file thumbnails, icons and mimetypes and writes to disk

  DETAIL:
    contains 3 different file types:
      index.cache - holds file hashes, sizes and offsets in the data file
      data.cache - thumbnail and mimetype cache data file

  CHANGELOG:
    v3:
      implemented hash algorithm
      simplified blocked structure to a single data file
      optimized code
      introduced worker/indexer threads for hashing and mimetype detection
      implemented in app

  TODOs:
    further optimization of code (updates are SLOW)
    failsafe mode for corrupted/wrong version cache
*/

/**
 * @brief Cache::Cache
 * @param path
 */
Cache::Cache( const QString &path ) : m_path( path ), m_valid( true ) {
    this->cacheDir = QDir( this->path());

    // check if cache dir exists
    if ( !this->cacheDir.exists()) {
        qDebug() << this->tr( "Cache: creating non-existant cache dir" );
        this->cacheDir.mkpath( this->path());

        // additional failsafe
        if ( !this->cacheDir.exists()) {
            this->shutdown();
            return;
        }
    }

    // set up index file
    this->index.setFilename( cacheDir.absolutePath() + "/" + CacheSystem::IndexFilename );
    if ( !this->index.open()) {
        qDebug() << this->tr( "Cache: index file non-writable" );
        this->shutdown();
        return;
    } else {
        if ( !this->index.size())
            this->index << CacheSystem::Version;
    }

    // set up data file
    this->data.setFilename( cacheDir.absolutePath() + "/" + CacheSystem::DataFilename );
    if ( !this->data.open()) {
        qDebug() << this->tr( "Cache: data file non-writable" );
        this->shutdown();
        return;
    }

    // reead data
    if ( !this->read()) {
        qDebug() << this->tr( "Cache: failed to read cache" );
        this->shutdown();
        return;
    }

    // create a new indexer
    this->indexer = new Indexer();
    this->connect( this->indexer, SIGNAL( workDone( QString, Hash )), this, SLOT( indexingDone( QString, Hash )));
    this->connect( this->indexer, SIGNAL( finished()), this->indexer, SLOT( deleteLater()));
    this->indexer->start();

    // create a new worker
    this->worker = new Worker();
    this->connect( this->worker, SIGNAL( workDone( Work )), this, SLOT( workDone( Work )));
    this->connect( this->worker, SIGNAL( finished()), this->worker, SLOT( deleteLater()));
    this->worker->start();
}

/**
 * @brief Cache::read
 * @return
 */
bool Cache::read() {
    // failsafe
    if ( !this->isValid())
        return false;

    // read index file version
    quint8 version;
    this->index.toStart();
    this->index >> version;

    // check version
    if ( version != CacheSystem::Version ) {
        qDebug() << this->tr( "Cache::read: version mismatch for index file" );
        this->shutdown();
        return false;
    }

    // read indexes
    while ( !this->index.atEnd()) {
        IndexEntry indexEntry;
        this->index >> indexEntry;
        this->hash[Hash( indexEntry.hash, indexEntry.size )] = indexEntry;
    }

    // report
    qDebug() << this->tr( "Cache::read: found %1 entries in index file" ).arg( this->hash.count());

    // return success
    return true;
}

/**
 * @brief Cache::write
 * @param hash
 * @param size
 * @param mimeType
 * @param pixmapList
 * @return
 */
bool Cache::write( quint32 hash, qint64 size, QString mimeType, QList<QPixmap> pixmapList ) {
    // failsafe
    if ( !this->isValid())
        return false;

    // check hash
    if ( hash == 0 || size == 0 || mimeType.length() == 0 ) {
       // qDebug() << this->tr( "Cache::write: zero length hash, size or mimeType" );
        return false;
    }

    // check for duplicates
    if ( this->contains( hash, size ))
        return true;

    // create new index entry
    IndexEntry indexEntry( hash, size, this->data.size());
    this->index.seek( FileStream::End );
    this->index << indexEntry;

    // create new data entry
    DataEntry dataEntry( mimeType, pixmapList );
    this->data.seek( FileStream::End );
    this->data << dataEntry;

    // return success
    return true;
}

/**
 * @brief Cache::cachedData
 * @param index
 * @return
 */
DataEntry Cache::cachedData( quint32 hash, qint64 size ) {
    DataEntry entry;

    if ( !this->isValid() || !this->contains( hash, size ))
        return entry;

    this->data.seek( FileStream::Set, this->hash[Hash( hash, size )].offset );
    this->data >> entry;

    return entry;
}

/**
 * @brief Cache::fail
 */
void Cache::shutdown() {
    this->setValid( false );
    this->index.close();
    this->data.close();

    if ( this->indexer->isRunning()) {
        this->indexer->requestInterruption();
        this->indexer->wait();
    }

    if ( this->worker->isRunning()) {
        this->worker->requestInterruption();
        this->worker->wait();
    }
}

/**
 * @brief checksum
 * @param data
 * @param len
 * @return
 */
quint32 Cache::checksum( const char* data, size_t len ) {
    const quint32 m = 0x5bd1e995, r = 24;
    quint32 h = 0, w;
    const char *l = data + len;

    while ( data + 4 <= l ) {
        w = *( reinterpret_cast<const quint32*>( data ));
        data += 4;
        h += w;
        h *= m;
        h ^= ( h >> 16 );
    }

    switch ( l - data ) {
    case 3:
        h += data[2] << 16;

    case 2:
        h += data[1] << 8;

    case 1:
        h += data[0];
        h *= m;
        h ^= ( h >> r );
        break;
    }
    return h;
}

/**
 * @brief Cache::process
 * @param fileName
 */
void Cache::process( const QString &fileName ) {
    if ( fileName.isEmpty() )
        return;

    this->indexer->addWork( fileName );
}

/**
 * @brief Cache::process
 * @param fileList
 */
void Cache::process( const QStringList &fileList ) {
    QStringList files;

    foreach ( QString fileName, fileList ) {
        if ( !fileName.isEmpty())
            files << fileName;
    }

    std::reverse( files.begin(), files.end());
    this->indexer->addWork( files );
}

/**
 * @brief Cache::indexingDone
 * @param fileName
 */
void Cache::indexingDone( const QString &fileName, const Hash &hash ) {
    if ( !this->contains( hash )) {
        this->worker->addWork( Work( hash, fileName ));
        return;
    }

    // done
    // TODO: this could also be threaaded and async
    emit this->finished( fileName, this->cachedData( hash ));
}

/**
 * @brief Cache::workDone
 * @param fileName
 */
void Cache::workDone( const Work &work ) {
    //qDebug() << "finished" << work.fileName << work.hash << work.data.mimeType << work.data.pixmapList.count();

    // cache to disk
    this->write( work.hash.first, work.hash.second, work.data.mimeType, work.data.pixmapList );

    // done
    emit this->finished( work.fileName, work.data );
}
