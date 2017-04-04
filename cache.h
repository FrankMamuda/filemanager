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

#ifndef CACHE_H
#define CACHE_H

//
// includes
//
#include <QDataStream>
#include <QPixmap>
#include <QDir>
#include <QHash>
#include "filestream.h"

//
// classes
//
class Worker;
class Indexer;
struct Work;

/**
 * @brief The CacheSystem namespace
 */
namespace CacheSystem {
    static const quint8 Version = 2;
    static const QString IndexFilename( "index.cache" );
    static const QString DataFilename( "data.cache" );
}

/**
 * @brief Hash
 */
typedef QPair<quint32, qint64> Hash;
Q_DECLARE_METATYPE( Hash )

/**
 * @brief The IndexEntry struct
 */
struct IndexEntry {
    IndexEntry( quint32 h = 0, qint64 s = 0, qint64 o = 0 ) : hash( h ), size( s ), offset( o ) {}
    quint32 hash;
    qint64 size;
    qint64 offset;
};

// read/write operators
inline static QDataStream &operator<<( QDataStream &out, const IndexEntry &e ) { out << e.hash << e.size << e.offset; return out; }
inline static QDataStream &operator>>( QDataStream &in, IndexEntry &e ) { in >> e.hash >> e.size >> e.offset; return in; }

/**
 * @brief The DataEntry struct
 */
struct DataEntry {
    DataEntry( const QString &m = QString::null, QList<QPixmap> l = QList<QPixmap>()) : mimeType( m ), pixmapList( l ) {}
    QString mimeType;
    QList<QPixmap> pixmapList;
};
Q_DECLARE_METATYPE( DataEntry )

// read/write operators
inline static QDataStream &operator<<( QDataStream &out, const DataEntry &e ) { out << e.mimeType << e.pixmapList; return out; }
inline static QDataStream &operator>>( QDataStream &in, DataEntry &e ) { in >> e.mimeType >> e.pixmapList; return in; }

/**
 * @brief The Work struct
 */
struct Work {
    Work( const Hash &h = Hash(), const QString &f = QString::null, const DataEntry &d = DataEntry()) : hash( h ), fileName( f ), data( d ) {}
    Hash hash;
    QString fileName;
    DataEntry data;
};
Q_DECLARE_METATYPE( Work )

/**
 * @brief The Cache class
 */
class Cache : public QObject {
    Q_OBJECT
    Q_PROPERTY( QString path READ path )
    Q_PROPERTY( bool valid READ isValid )

public:
    Cache( const QString &path );
    ~Cache() { this->shutdown(); }
    static quint32 checksum( const char *data, size_t len );

public slots:
    void process( const QString &fileName );
    void process( const QStringList &fileList );
    void stop();

signals:
    void finished( const QString &fileName, const DataEntry &entry );

private slots:
    void setValid( bool valid ) { this->m_valid = valid; }
    void shutdown();
    void workDone( const Work &work );
    void indexingDone( const QString &fileName, const Hash &hash );

private:
    QString path() const { return this->m_path; }
    bool isValid() const { return this->m_valid; }
    bool write( quint32 hash, qint64 size, QString mimeType, QList<QPixmap> pixmapList = QList<QPixmap>());
    DataEntry cachedData( quint32 hash, qint64 size );
    DataEntry cachedData( const Hash &hash ) { return this->cachedData( hash.first, hash.second ); }
    bool contains( const Hash &hash ) const { return this->contains( hash.first, hash.second ); }
    bool contains( quint32 hash, qint64 size ) const { return this->hash.contains( Hash( hash, size )); }
    bool read();
    FileStream index;
    FileStream data;
    QString m_path;
    QHash<Hash, IndexEntry> hash;
    bool m_valid;
    QDir cacheDir;
    Worker *worker;
    Indexer *indexer;
};

#endif // CACHE_H
