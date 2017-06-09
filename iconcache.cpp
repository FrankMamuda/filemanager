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
#include <QIcon>
#include "iconcache.h"
#include "iconfetcher.h"
#include "pixmapcache.h"
#include "main.h"

/**
 * @brief IconCache::IconCache
 * @param path
 */
IconCache::IconCache( const QString &path ) : m_path( path ), m_valid( true ), m_updateRequested( false ) {
    this->cacheDir = QDir( this->path());

    // check if cache dir exists
    if ( !this->cacheDir.exists()) {
        qDebug() << this->tr( "IconCache: creating non-existant cache dir" );
        this->cacheDir.mkpath( this->path());

        // additional failsafe
        if ( !this->cacheDir.exists()) {
            this->shutdown();
            return;
        }
    }

    // set up index file
    this->index.setFilename( cacheDir.absolutePath() + "/" + IconCacheSystem::IndexFilename );
    if ( !this->index.open()) {
        qDebug() << this->tr( "IconCache: index file non-writable" );
        this->shutdown();
        return;
    } else {
        if ( !this->index.size())
            this->index << IconCacheSystem::Version;
    }

    // set up data file
    this->data.setFilename( cacheDir.absolutePath() + "/" + IconCacheSystem::DataFilename );
    if ( !this->data.open()) {
        qDebug() << this->tr( "IconCache: data file non-writable" );
        this->shutdown();
        return;
    }

    // reead data
    if ( !this->read()) {
        qDebug() << this->tr( "IconCache: failed to read cache" );
        this->shutdown();
        return;
    }
}

/**
 * @brief IconCache::read
 * @return
 */
bool IconCache::read() {
    QMutexLocker( &this->m_mutex );

    // failsafe
    if ( !this->isValid())
        return false;

    // read index file version
    quint8 version;
    this->index.toStart();
    this->index >> version;

    // check version
    if ( version != IconCacheSystem::Version ) {
        qDebug() << this->tr( "IconCache::read: version mismatch for index file" );
        this->shutdown();
        return false;
    }

    // read indexes
    while ( !this->index.atEnd()) {
        IconEntry iconEntry;
        this->index >> iconEntry;
        this->hash[IconIndex( iconEntry.iconName,iconEntry.scale )] = iconEntry;
    }

    // report
    qDebug() << this->tr( "IconCache::read: found %1 entries in index file" ).arg( this->hash.count());

    // return success
    return true;
}

/**
 * @brief IconCache::write
 * @param iconName
 * @param iconScale
 * @param pixmap
 * @return
 */
bool IconCache::write( const QString &iconName, quint8 iconScale, const QPixmap &pixmap ) {
    QMutexLocker( &this->m_mutex );

    // failsafe
    if ( !this->isValid())
        return false;

    // check hash
    if ( iconName.isEmpty() || iconScale > IconCacheSystem::IconScales[0] || iconScale < IconCacheSystem::IconScales[IconCacheSystem::NumIconScales-1] || pixmap.isNull()) {
        qDebug() << this->tr( "IconCache::write: invalid iconName or pixmap" );
        return false;
    }

    // check for duplicates
    if ( this->contains( iconName, iconScale ))
        return true;

    // create new icon entry
    IconEntry iconEntry( iconName, iconScale, this->data.size());
    this->index.seek( FileStream::End );
    this->index << iconEntry;

    // add new enty to list
    this->hash[IconIndex( iconEntry.iconName, iconEntry.scale )] = iconEntry;

    // create new data entry
    if ( this->data.seek( FileStream::End )) {
        this->data << pixmap;

        // flush to disk immediately
        this->data.sync();

        // return success
        return true;
    }

    return false;
}

/**
 * @brief IconCache::pixmap
 * @param iconName
 * @param iconScale
 * @return
 */
QPixmap IconCache::pixmap( const QString &iconName, quint8 iconScale ) {
    QPixmap pm;

    QMutexLocker( &this->m_mutex );


    if ( !this->isValid() || !this->contains( iconName, iconScale ))
        return pm;

    if( this->data.seek( FileStream::Set, this->hash[IconIndex( iconName, iconScale )].offset ))
        this->data >> pm;

    return pm;
}

/**
 * @brief IconCache::run
 */
void IconCache::run() {
    // enter event loop
    while ( !this->isInterruptionRequested()) {
        QMutexLocker( &this->m_mutex );

        // LIFO - prioritizing most recent entries
        if ( !this->workList.isEmpty()) {
            IconIndex index;
            QPixmap pixmap;
            QString iconName;
            quint8 iconScale;

            index = this->workList.takeLast();
            iconName = index.first;
            iconScale = index.second;

            if ( this->contains( iconName, iconScale )) {
                emit this->finished( iconName, iconScale, this->pixmap( iconName, iconScale ));
                continue;
            }
            pixmap = m.pixmapCache->pixmap( iconName, iconScale );

            if ( !pixmap.isNull() && pixmap.width()) {
                // cache to disk
                this->write( iconName, iconScale, pixmap );
                emit this->finished( iconName, iconScale, pixmap );
            }
        } else {
            if ( this->updateRequested()) {
                this->m_updateRequested = false;
                emit this->update();
            }

            msleep( 100 );
        }
    }
}

/**
 * @brief IconCache::shutdown
 */
void IconCache::shutdown() {
    QMutexLocker( &this->m_mutex );

    this->setValid( false );
    this->index.close();
    this->data.close();
}

/**
 * @brief IconCache::process
 * @param iconName
 */
void IconCache::process( const QString &iconName, quint8 iconScale ) {
    QMutexLocker( &this->m_mutex );
    IconIndex index;

    if ( iconName.isEmpty())
        return;

    index = IconIndex( iconName, iconScale );
    if ( !this->workList.contains( index ))
        this->workList << index;
}

/**
 * @brief IconCache::clear
 */
void IconCache::clear() {
    QMutexLocker( &this->m_mutex );
    this->workList.clear();
}
