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

/**
 * @brief The CacheSystem namespace
 */
namespace CacheSystem {
    const unsigned int Version = 1;
    const qint64 CacheBlockSize = 10485760;
}

/**
 * @brief The IndexEntry struct
 */
struct IndexEntry {
    quint32 hash;
    qint64 size;
    quint8 index;
    qint64 offset;
};

// read/write operators
inline static QDataStream &operator<<( QDataStream &out, const IndexEntry &e ) { out << e.hash << e.size << e.index << e.offset; return out; }
inline static QDataStream &operator>>( QDataStream &in, IndexEntry &e ) { in >> e.hash >> e.size >> e.index >> e.offset; return in; }

/**
 * @brief The DataEntry struct
 */
struct DataEntry {
    QString mimeType;
    QList<QPixmap> pixmapList;
};

// read/write operators
inline static QDataStream &operator<<( QDataStream &out, const DataEntry &e ) { out << e.mimeType << e.pixmapList; return out; }
inline static QDataStream &operator>>( QDataStream &in, DataEntry &e ) { in >> e.mimeType >> e.pixmapList; return in; }

/**
 * @brief The Cache class
 */
class Cache : public QObject {
    Q_OBJECT
    Q_PROPERTY( QString path READ path WRITE setPath )

public:
    Cache( const QString &path );
    ~Cache();
    QString path() const { return this->m_path; }
    int currentIndex() const { return this->m_currentIndex; }

public slots:
    void setPath( const QString &path ) { this->m_path = path;}
    bool write( quint32 hash, qint64 size, QString mimeType, QList<QPixmap>pixmapList );
    bool writeChecksums();
    bool isValid() const { return this->m_valid; }
    bool contains( quint32 hash, qint64 size ) const;
    IndexEntry indexAt( quint32 hash, qint64 size ) const;
    DataEntry indexData( IndexEntry index ) const;

private slots:
    bool touch();
    bool read();
    bool readChecksums();
    bool validateChecksums();
    void setCurrentIndex();
    QByteArray checksumForBlock( int blockId, bool &ok );
    bool setChecksumForBlock( int blockId = 0 );
    void setValid( bool valid ) { this->m_valid = valid; }
    void touchBlock( int blockId = 0 );

private:
    QString m_path;
    int m_currentIndex;
    QHash<QPair<quint32,qint64>, IndexEntry> indexHash;
    QList<QByteArray> checksumList;
    bool m_valid;
    QDir cacheDir;
};

#endif // CACHE_H
