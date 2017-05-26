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
class PixmapCache {
public:
    PixmapCache() {}
    QPixmap pixmap( const QString &name, int scale, bool thumbnail = false );
    QIcon fromTheme( const QString &name );
    void buildIndex();
    int parseSVG( const QString &buffer );
    IconMatch readIconFile( const QString &buffer, bool &ok, int recursionLevel = 2 );
    QPixmap findPixmap( const QString &name, int scale );
    IconMatchList getIconMatchList( const QString &name );

private:
    QHash<QString, QPixmap> pixmapCache;
    QStringList index;
};

extern class PixmapCache pixmapCache;
