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

#ifndef ICONCACHE_H
#define ICONCACHE_H

//
// includes
//
#include <QDataStream>
#include <QPixmap>
#include <QDir>
#include <QHash>
#include <QThread>
#include <QMutex>
#include "filestream.h"

//
// classes
//
class IconFetcher;

/**
 * @brief The IconCacheSystem namespace
 */
namespace IconCacheSystem {
    static const quint8 Version = 1;
    static const QString IndexFilename( "icons.index" );
    static const QString DataFilename( "icons.data" );
    static const quint8 NumIconScales = 4;
    static const quint8 IconScales[NumIconScales] = { 64, 48, 32, 16 };
}

/**
 * @brief IconIndex
 */
typedef QPair<QString, qint8> IconIndex;
Q_DECLARE_METATYPE( IconIndex )

/**
 * @brief The IconEntry struct
 */
struct IconEntry {
    IconEntry( const QString &n = QString::null, quint8 s = IconCacheSystem::IconScales[0], qint64 o = 0 ) : iconName( n ), scale( s ), offset( o ) {}
    QString iconName;
    quint8 scale;
    qint64 offset;
};

// read/write operators
inline static QDataStream &operator<<( QDataStream &out, const IconEntry &e ) { out << e.iconName << e.scale << e.offset; return out; }
inline static QDataStream &operator>>( QDataStream &in, IconEntry &e ) { in >> e.iconName >> e.scale >> e.offset; return in; }

/**
 * @brief The IconCache class
 */
class IconCache : public QThread {
    Q_OBJECT
    Q_PROPERTY( QString path READ path )
    Q_PROPERTY( bool valid READ isValid )

public:
    IconCache( const QString &path );
    ~IconCache() { this->shutdown(); }

    bool updateRequested() const { QMutexLocker( &this->m_mutex ); return this->m_updateRequested; }

public slots:
    void process( const QString &iconName, quint8 iconScale );
    void requestUpdate() { QMutexLocker( &this->m_mutex ); this->clear(); this->m_updateRequested = true; }
    void clear();

signals:
    void finished( const QString &iconName, quint8 iconScale, const QPixmap &pixmap );
    void update();

private slots:
    void setValid( bool valid ) { this->m_valid = valid; }
    void shutdown();

private:
    Q_DISABLE_COPY( IconCache )
    QString path() const { return this->m_path; }
    bool isValid() const { return this->m_valid; }
    bool write( const QString &iconName, quint8 iconScale, const QPixmap &pixmap );
    bool contains( const QString &iconName, quint8 iconScale ) const { QMutexLocker( &this->m_mutex ); return this->hash.contains( IconIndex( iconName, iconScale )); }
    bool read();
    FileStream index;
    FileStream data;
    QString m_path;
    QHash<IconIndex, IconEntry> hash;
    QPixmap pixmap( const QString &iconName, quint8 iconScale );

    bool m_valid;
    QDir cacheDir;
    IconFetcher *iconFetcher;

    void run();
    QList<IconIndex> workList;

    mutable QMutex m_mutex;

    bool m_updateRequested;
};

#endif // ICONCACHE_H


