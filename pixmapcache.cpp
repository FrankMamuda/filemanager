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
#include "pixmapcache.h"
#include <QIcon>
#include <QFileInfo>

//
// class: PixmapCache
//
class PixmapCache pixmapCache;

/**
 * @brief PixmapCache::pixmap
 * @param name
 * @return
 */
QPixmap PixmapCache::pixmap( const QString &name, int scale, bool thumbnail ) {
    QPixmap pixmap;
    QString cache;

    // make unique pixmaps for different sizes
    cache = QString( "%1_%2" ).arg( name ).arg( scale );

    // search in hash table
    if ( !this->pixmapCache.contains( cache )) {
        // jpeg/png/etc. generate square thunbnails from the actual images
        // other file use icons based on the mime-type
        if ( !thumbnail )
            pixmap = QIcon::fromTheme( name ).pixmap( scale, scale );
        else {
            QFileInfo info( name );

            if ( info.isSymLink())
                pixmap.load( info.symLinkTarget());
            else
                pixmap.load( name );
        }

        // handle missing icons
        if ( pixmap.width() == 0 ) {
            pixmap = QIcon::fromTheme( "application-x-zerosize" ).pixmap( scale, scale );

            // failsafe, in case something doesn't work as intended
            // NOTE: for some reason some icons fail to load
            if ( pixmap.width() == 0 )
                return QPixmap();
        }

        // generate thumbnail if necessary
        if ( thumbnail ) {
            QRect rect;

            // crop and scale down if required
            if ( pixmap.height() > scale || pixmap.width() > scale ) {
                if ( pixmap.width() > pixmap.height())
                    rect = QRect( pixmap.width() / 2 - pixmap.height() / 2, 0, pixmap.height(), pixmap.height());
                else if ( pixmap.width() < pixmap.height())
                    rect = QRect( 0, pixmap.height() / 2 - pixmap.width() / 2, pixmap.width(), pixmap.width());

                pixmap = pixmap.copy( rect );

                // fast downsizing if necessary
                if ( pixmap.width() >= scale * 2.0f )
                    pixmap = pixmap.scaled( scale * 2.0f, scale * 2.0f, Qt::IgnoreAspectRatio, Qt::FastTransformation );

                pixmap = pixmap.scaled( scale, scale, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
            }
        }

        // add pixmap to cache
        this->pixmapCache[cache] = pixmap;
    }

    // retrieve and return the pixmap
    pixmap = this->pixmapCache[cache];
    return pixmap;
}
