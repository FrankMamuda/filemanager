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

#pragma once

//
// includes
//
#include <QPixmap>
#include <QHash>
#include <QDir>
#include <QIcon>
#include "filestream.h"

/**
 * @brief The PixmapCacheSystem namespace
 */
namespace PixmapCacheSystem {
    static const quint8 Version = 1;
    static const QString IndexFilename( "pixmaps.index" );
}

/**
 * @brief The PixmapEntry struct
 */
struct PixmapEntry {
    PixmapEntry( const QString &c = QString::null,
                 const QString &f = QString::null
                 ) : cachedName( c ), fileName( f ) {}
    QString cachedName;
    QString fileName;
};
Q_DECLARE_METATYPE( PixmapEntry )

// read/write operators
inline static QDataStream &operator<<( QDataStream &out, const PixmapEntry &e ) { out << e.cachedName << e.fileName; return out; }
inline static QDataStream &operator>>( QDataStream &in, PixmapEntry &e ) { in >> e.cachedName >> e.fileName; return in; }

/**
 * @brief The IconMatch struct
 */
struct IconMatch {
    IconMatch( const QString &f = QString::null, int s = 0 ) : fileName( f ), scale( s ) {}
    QString fileName;
    int scale;
};

typedef QList<IconMatch> IconMatchList;

/**
 * @brief The PixmapCache class
 */
class PixmapCache : public QObject {
    Q_OBJECT

public:
    PixmapCache( const QString &path );
    ~PixmapCache() { this->shutdown(); }
    QPixmap pixmap( const QString &name, int scale, const QString themeName = QString::null, bool thumbnail = false );
    QIcon icon( const QString &name, int scale = 0, const QString themeName = QString::null );
    void buildIndex( const QString &themeName );
    int parseSVG( const QString &buffer );
    IconMatch readIconFile( const QString &buffer, bool &ok, int recursionLevel = 2 );
    IconMatchList getIconMatchList( const QString &name, const QString &themeName );

private slots:
    void setValid( bool valid ) { this->m_valid = valid; }
    void shutdown();

private:
    QIcon findIcon( const QString &name, int scale = 0, const QString &themeName = QString::null );
    QPixmap findPixmap( const QString &name, int scale, const QString &themeName = QString::null );
    QHash<QString, QPixmap> pixmapCache;
    QHash<QString, QIcon> iconCache;
    QHash<QString, QStringList> index;
    QString defaultTheme;

    Q_DISABLE_COPY( PixmapCache )
    QString path() const { return this->m_path; }
    bool isValid() const { return this->m_valid; }
    bool write( const QString &iconName, const QString &themeName, int iconScale, const QString &fileName );
    bool contains( const QString &cachedName ) const { return this->hash.contains( cachedName ); }
    bool read();
    FileStream indexFile;
    QString m_path;
    QHash<QString, PixmapEntry> hash;
    bool m_valid;
    QDir cacheDir;
};
