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
#include "cache.h"
#include <QDebug>
#include <QCryptographicHash>

/*
  The Cache Subsystem

  OVERVIEW:
    precaches file thumbnails, icons and mimetypes and writes to disk

  DETAIL:
    contains 3 different file types:
      index.cache - holds file hashes, sizes and offsets in corresponding data files
      data_N.cache - (where N is block number) thumbnail and mimetype cache in blocks of 10 mb
      checksums.cache - data_N.cache file checksums

  TODOs:
    add support for bulk writes
    optimize code
    implement in app
*/

/**
 * @brief Cache::Cache
 * @param path
 */
Cache::Cache( const QString &path ) : m_path( path ), m_currentIndex( 0 ), m_valid( true ) {
    this->cacheDir = QDir( this->path());

    // check if cache dir exists
    if ( !this->cacheDir.exists()) {
        qDebug() << this->tr( "Cache::read: creating non-existant cache dir" );
        this->cacheDir.mkpath( this->path());

        // additional failsafe
        if ( !this->cacheDir.exists()) {
            this->setValid( false );
            return;
        }
    }

    // init structure and read data
    if ( this->touch()) {
        if ( this->readChecksums()) {
            if ( !this->read()) {
                qDebug() << this->tr( "Cache: failed to read cache" );
                this->setValid( false );
                return;
            }
        }
    }
}

/**
 * @brief Cache::~Cache
 */
Cache::~Cache() {
    this->writeChecksums();
}

/**
 * @brief Cache::read
 * @return
 */
bool Cache::read() {
    // failsafe
    if ( !this->isValid())
        return false;

    // check if index file exists
    QFile indexFile( cacheDir.absolutePath() + "/index.cache" );
    if ( !indexFile.exists()) {
        qDebug() << this->tr( "Cache::read: index file non-existant" );
        this->setValid( false );
        return false;
    }

    // read index file
    if ( indexFile.open( QFile::ReadOnly )) {
        QDataStream stream( &indexFile );
        quint8 version;

        // read index file
        stream >> version;

        // check version
        if ( version != CacheSystem::Version ) {
            qDebug() << this->tr( "Cache::read: version mismatch for index file" );
            indexFile.close();
            this->setValid( false );
            return false;
        }

        // read indexes
        while ( !stream.atEnd()) {
            IndexEntry indexEntry;
            stream >> indexEntry;

            QPair<int,int>pair;
            pair.first = indexEntry.hash;
            pair.second = indexEntry.size;

            this->indexHash[pair] = indexEntry;
        }

        // close index file
        indexFile.close();
    } else {
        qDebug() << this->tr( "Cache::read: index file non-readable" );
        this->setValid( false );
        return false;
    }

    // set current index
    this->setCurrentIndex();

    // return success
    return true;
}

/**
 * @brief Cache::touch
 * @return
 */
bool Cache::touch() {
    // failsafe
    if ( !this->isValid())
        return false;

    // check if index file exists
    QFile indexFile( cacheDir.absolutePath() + "/index.cache" );
    if ( !indexFile.exists()) {
        // create index file
        if ( indexFile.open( QFile::WriteOnly | QFile::Truncate )) {
            QDataStream stream( &indexFile );
            quint8 version = 1;

            // write out
            stream << version;

            // announce
            qDebug() << this->tr( "Cache::touch: writing empty index file" );

            // close index file
            indexFile.close();
        } else {
            qDebug() << this->tr( "Cache::touch: index file non-writable" );
            this->setValid( false );
            return false;
        }
    }

    // create checksum file
    QFile checksumFile( cacheDir.absolutePath() + "/checksums.cache" );
    if ( !checksumFile.exists()) {
        if ( checksumFile.open( QFile::WriteOnly | QFile::Truncate )) {
            // announce
            qDebug() << this->tr( "Cache::touch: writing empty checksum file" );

            // close checksum file
            checksumFile.close();
        } else {
            qDebug() << this->tr( "Cache::touch: checksum file non-writable" );
            this->setValid( false );
            return false;
        }
    }

    return true;
}

/**
 * @brief Cache::write
 * @param hash
 * @param size
 * @param mimeType
 * @param pixmapList
 */
bool Cache::write( quint32 hash, qint64 size, QString mimeType, QList<QPixmap> pixmapList ) {
    // failsafe
    if ( !this->isValid())
        return false;

    // check hash
    if ( hash == 0 ) {
        qDebug() << this->tr( "Cache::write: invalid hash" );
        this->setValid( false );
        return false;
    }

    // check file size
    if ( size == 0 ) {
        qDebug() << this->tr( "Cache::write: invalid file size" );
        this->setValid( false );
        return false;
    }

    // check for duplicates
    if ( this->contains( hash, size ))
        return true;

    // check mimetype
    if ( mimeType.isEmpty()) {
        qDebug() << this->tr( "Cache::write: invalid mimeType" );
        this->setValid( false );
        return false;
    }

    // failsafe
    QFile dataFile( cacheDir.absolutePath() + QString( "/data_%1.cache" ).arg( this->currentIndex()));
    if ( !dataFile.exists()) {
        this->setValid( false );
        return false;
    }

    // first append to index
    QFile indexFile( cacheDir.absolutePath() + "/index.cache" );
    if ( indexFile.open( QFile::WriteOnly | QFile::Append )) {
        QDataStream stream( &indexFile );

        IndexEntry indexEntry;
        indexEntry.hash = hash;
        indexEntry.size = size;
        indexEntry.offset = dataFile.size();
        indexEntry.index = this->currentIndex();
        stream << indexEntry;

        // close index file
        indexFile.close();
    } else {
        qDebug() << this->tr( "Cache::write: could not write to index file" );
        this->setValid( false );
        return false;
    }

    // write to data file
    if ( dataFile.open( QFile::WriteOnly | QFile::Append )) {
        QDataStream stream( &dataFile );

        DataEntry dataEntry;
        dataEntry.mimeType = mimeType;
        dataEntry.pixmapList = pixmapList;
        stream << dataEntry;

        // close index file
        dataFile.close();
    } else {
        qDebug() << this->tr( "Cache::write: could not write to index file" );
        this->setValid( false );
        return false;
    }

    // check whether we need to open a new block
    this->setCurrentIndex();

    return true;
}

/**
 * @brief Cache::readChecksums
 * @return
 */
bool Cache::readChecksums() {
    // failsafe
    if ( !this->isValid())
        return false;

    // announce
    qDebug() << this->tr( "Cache::readChecksums: reading checksums" );

    // read checksums
    QFile checksumFile( cacheDir.absolutePath() + "/checksums.cache" );
    if ( checksumFile.open( QFile::ReadOnly )) {
        QDataStream stream( &checksumFile );

        // read checksums
        while ( !stream.atEnd()) {
            QByteArray checksum;
            stream >> checksum;
            this->checksumList << checksum;
        }

        // close index file
        checksumFile.close();
    } else {
        qDebug() << this->tr( "Cache::readChecksums: could not read checksum file" );
        this->setValid( false );
        return false;
    }

    // validate data files
    return this->validateChecksums();
}

/**
 * @brief Cache::writeChecksums
 * @return
 */
bool Cache::writeChecksums() {
    // failsafe
    if ( !this->isValid())
        return false;

    // regenerate checksum of the last block
    if ( !this->setChecksumForBlock( this->m_currentIndex )) {
        qDebug() << this->tr( "Cache::writeChecksums: could not generate checksum for the last block" );
        this->setValid( false );
        return false;
    }

    // write checksums
    QFile checksumFile( cacheDir.absolutePath() + "/checksums.cache" );
    if ( checksumFile.open( QFile::WriteOnly | QFile::Truncate )) {
        QDataStream stream( &checksumFile );

        // announce
        qDebug() << this->tr( "Cache::writeChecksums: writing checksums" );

        // read checksums
        foreach ( QByteArray checksum, this->checksumList )
            stream << checksum;

        // close index file
        checksumFile.close();
    } else {
        qDebug() << this->tr( "Cache::writeChecksums: could not read checksum file" );
        this->setValid( false );
        return false;
    }

    return true;
}

/**
 * @brief Cache::contains
 * @param hash
 * @param size
 * @return
 */
bool Cache::contains( quint32 hash, qint64 size ) const {
    QPair<quint32,qint64>pair;
    pair.first = hash;
    pair.second = size;

    if ( this->indexHash.contains( pair ))
        return true;

    return false;
}

/**
 * @brief Cache::indexAt
 * @param hash
 * @param size
 * @return
 */
IndexEntry Cache::indexAt( quint32 hash, qint64 size ) const {
    QPair<quint32,qint64>pair;

    if ( !this->isValid())
        return IndexEntry();

    pair.first = hash;
    pair.second = size;

    return this->indexHash[pair];
}

/**
 * @brief Cache::indexData
 * @param index
 * @return
 */
DataEntry Cache::indexData( IndexEntry index ) const {
    DataEntry entry;
    QFile dataFile;

    if ( !this->isValid())
        return entry;

    dataFile.setFileName( cacheDir.absolutePath() + QString( "/data_%1.cache" ).arg( index.index ));
    if ( dataFile.open( QFile::ReadOnly )) {
        QDataStream stream( &dataFile );

        if ( !stream.skipRawData( index.offset ))
            return entry;

        stream >> entry;
        dataFile.close();
    } else {
        return entry;
    }

    return entry;
}

/**
 * @brief Cache::validateChecksums
 * @return
 */
bool Cache::validateChecksums() {
    int y = 0;

    // failsafe
    if ( !this->isValid())
        return false;

    // go through all checksums
    foreach ( QByteArray checksum, this->checksumList ) {
        QByteArray dataChecksum;
        bool ok;

        dataChecksum = this->checksumForBlock( y, ok );
        if ( !ok ) {
            this->setValid( false );
            return false;
        }

        if ( dataChecksum != checksum ) {
            qDebug() << this->tr( "Cache::validateChecksums checksum mismatch" ) << dataChecksum.toHex() << checksum.toHex();
            this->setValid( false );
            return false;
        }
        y++;
    }

    // announce
    qDebug() << this->tr( "Cache::validateChecksums: checksums ok" );

    // return success
    return true;
}

/**
 * @brief Cache::checksumForBlock
 * @param ok
 * @return
 */
QByteArray Cache::checksumForBlock( int blockId, bool &ok ) {
    QByteArray checksum;
    QFile dataFile( cacheDir.absolutePath() + QString( "/data_%1.cache" ).arg( blockId ));

    // failsafe
    if ( !this->isValid())
        return checksum;

    ok = false;
    if ( dataFile.open( QFile::ReadOnly )) {
        checksum = QCryptographicHash::hash( dataFile.readAll(), QCryptographicHash::Md5 );
        dataFile.close();
        ok = true;
    }

    return checksum;
}

/**
 * @brief Cache::setCurrentIndex
 * @param index
 */
void Cache::setCurrentIndex() {
    QFile dataFile;

    // check if cache dir exists
    if ( !this->isValid())
        return;

    // create first block if non-existant
    if ( this->checksumList.count() == 0 ) {
        this->touchBlock();
        if ( !this->setChecksumForBlock()) {
            qDebug() << this->tr( "Cache::setCurrentIndex: could not generate checksum for first block" );
            this->setValid( false );
            return;
        }
    }

    // set current cache file index
    this->m_currentIndex = this->checksumList.count() - 1;

    // start a new block if the current exceeds limit
    dataFile.setFileName( cacheDir.absolutePath() + QString( "/data_%1.cache" ).arg( this->currentIndex()));
    if ( dataFile.size() > CacheSystem::CacheBlockSize ) {
        // generate checksum for the last block
        if ( !this->setChecksumForBlock( this->m_currentIndex )) {
            qDebug() << this->tr( "Cache::setCurrentIndex: could not generate checksum for the last block" );
            this->setValid( false );
            return;
        }

        // announce
        qDebug() << this->tr( "Cache::setCurrentIndex: starting new block %1" ).arg( this->m_currentIndex + 1 );

        // advance
        this->m_currentIndex++;
        this->touchBlock( this->currentIndex());
    }
}

/**
 * @brief Cache::touchBlock
 * @param blockId
 */
void Cache::touchBlock( int blockId ) {
    if ( !this->isValid())
        return;

    qDebug() << "  touch block" << blockId;

    QFile dataFile( cacheDir.absolutePath() + QString( "/data_%1.cache" ).arg( blockId ));
    if ( dataFile.open( QFile::WriteOnly | QFile::Truncate )) {
        qDebug() << this->tr( "Cache::touchBlock: block %1 written" ).arg( blockId );
        dataFile.close();
        this->checksumList << QByteArray();
    } else {
        qDebug() << this->tr( "Cache::touchBlock: block write error" );
        this->setValid( false );
    }
}

/**
 * @brief setChecksumForBlock
 */
bool Cache::setChecksumForBlock( int blockId ) {
    QByteArray checksum;

    // generate checksum for the last block
    bool ok;
    checksum = this->checksumForBlock( blockId, ok );
    if ( ok ) {
        if ( blockId < this->checksumList.count())
            this->checksumList.replace( blockId, checksum );
        else
            this->checksumList.append( checksum );
    } else {
        qDebug() << this->tr( "Cache::setChecksumForBlock: could not generate checksum for the last block" );
        this->setValid( false );
        return false;
    }

    return true;
}
