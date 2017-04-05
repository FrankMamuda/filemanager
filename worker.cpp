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
#include <QtConcurrent>
#include <QtWin>
#include <QSysInfo>
#include <commctrl.h>
#include <commoncontrols.h>
#include <shellapi.h>
#include "worker.h"
#include "cache.h"
#include <Winuser.h>
#include <QRgb>

/**
 * @brief Worker::extractPixmap
 * @param path
 * @return
 */
QPixmap Worker::extractPixmap( const QString &path, bool jumbo = false ) {
    SHFILEINFO shellInfo;
    QPixmap pixmap;

    memset( &shellInfo, 0, sizeof( SHFILEINFO ));
    if ( SUCCEEDED( SHGetFileInfo( reinterpret_cast<const wchar_t *>( QDir::toNativeSeparators( path ).utf16()), 0, &shellInfo, sizeof( SHFILEINFO ), SHGFI_ICON | SHGFI_SYSICONINDEX | SHGFI_ICONLOCATION | SHGFI_USEFILEATTRIBUTES | SHGFI_LARGEICON ))) {
        if ( shellInfo.hIcon ) {
            if ( QSysInfo::windowsVersion() >= QSysInfo::WV_VISTA ) {
                IImageList *imageList = NULL;
                int index = 0x2;

                if ( jumbo )
                    index = 0x4;

                if ( SUCCEEDED( SHGetImageList( index, { 0x46eb5926, 0x582e, 0x4017, { 0x9f, 0xdf, 0xe8, 0x99, 0x8d, 0xaa, 0x9, 0x50 }}, reinterpret_cast<void **>( &imageList )))) {
                    HICON hIcon;

                    if ( SUCCEEDED( imageList->GetIcon( shellInfo.iIcon, ILD_TRANSPARENT, &hIcon ))) {
                        pixmap = QtWin::fromHICON( hIcon );
                        DestroyIcon( hIcon );

                        if ( jumbo ) {
                            // NOTE: ugly hack
                            //
                            //        reasoning behind this is that jumbo icon returns
                            //        256x256 icon even if the actual icon is 16x16
                            //
                            //        this essentially checks whether most 3/4 of the
                            //        icon is blank
                            //
                            int y, k;
                            bool bad = true;
                            QImage image = pixmap.toImage();
                            for ( y = 64; y < 256; y++ ) {
                                for ( k = 64; k < 256; k++ ) {
                                    if ( image.pixelColor( y, k ).alphaF() > 0.0f )
                                        bad = false;
                                }
                            }

                            if ( bad )
                                return QPixmap();
                        }


                        if ( !pixmap.isNull())
                            return pixmap;
                    }
                }
            }

            pixmap = QtWin::fromHICON( shellInfo.hIcon );
            DestroyIcon( shellInfo.hIcon );
        }
    }
    return pixmap;
}

/**
 * @brief Worker::generateThumbnail
 * @param path
 * @param scale
 * @param ok
 * @return
 */
QPixmap Worker::generateThumbnail( const QString &path, int scale, bool &ok ) {
    QRect rect;
    QPixmap pixmap;

    ok = false;

    if ( !pixmap.load( path ))
        return pixmap;

    if ( pixmap.isNull() && !pixmap.width())
        return pixmap;

    if ( pixmap.height() > scale || pixmap.width() > scale ) {
        if ( pixmap.width() > pixmap.height())
            rect = QRect( pixmap.width() / 2 - pixmap.height() / 2, 0, pixmap.height(), pixmap.height());
        else if ( pixmap.width() < pixmap.height())
            rect = QRect( 0, pixmap.height() / 2 - pixmap.width() / 2, pixmap.width(), pixmap.width());

        pixmap = pixmap.copy( rect );

        if ( pixmap.width() >= scale * 2.0f )
            pixmap = pixmap.scaled( scale * 2.0f, scale * 2.0f, Qt::IgnoreAspectRatio, Qt::FastTransformation );

        pixmap = pixmap.scaled( scale, scale, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
    }

    ok = true;
    return pixmap;
}

/**
 * @brief Worker::scalePixmap
 * @param path
 * @param scale
 * @param ok
 * @return
 */
QPixmap Worker::scalePixmap( const QPixmap &pixmap, float scale ) {
    if ( pixmap.isNull() && !pixmap.width())
        return pixmap;

    return pixmap.scaled( pixmap.width() * scale, pixmap.height() * scale, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
}

/**
 * @brief Worker::work
 * @param fileName
 * @return
 */
DataEntry Worker::work( const QString &fileName ) {
    DataEntry data;
    QPixmap pixmap, jumbo;
    QMimeDatabase db;
    QFileInfo info( fileName );

    // files larger than the current 10MB get handled differently:
    //   - no thumbnail caching;
    //   - checksum is generated for the first 10MB
    //   - icon is extracted anyway
    if ( info.size() > CacheSystem::MaxFileSize ) {
        data.mimeType = db.mimeTypeForFile( info, QMimeDatabase::MatchExtension ).name();
    } else {
        data.mimeType = db.mimeTypeForFile( info, QMimeDatabase::MatchContent ).name();
        if ( data.mimeType.startsWith( "image/" )) {
            bool ok;
            int pixmapSizes[4] = { 64, 48, 32, 16 };
            //int y;

            // TODO: regenerate from the largest, not the original image

            //for ( y = 0; y < 4; y++ ) {
            // FIXME: ugly code
            QPixmap pixmap = Worker::generateThumbnail( info.absoluteFilePath(), pixmapSizes[0], ok );
            if ( ok ) {
                data.pixmapList << pixmap; // 64
                data.pixmapList << Worker::scalePixmap( data.pixmapList.last(), 0.75f ); // 48
                data.pixmapList << Worker::scalePixmap( data.pixmapList.last(), 0.75f ); // 32
                data.pixmapList << Worker::scalePixmap( data.pixmapList.last(), 0.75f ); // 16
            }
            //}
        }
    }

    // TODO: generate mipmap levels from jumbo icon
    //
    // ALL ICONS SHOULD ULTIMATELY HAVE 4 scaling levels 64 48 32 16
    // FileInfo pane must extract full scale thumbnail on its own
    //
    if ( !QString::compare( data.mimeType, "application/x-ms-dos-executable" ) && fileName.endsWith( ".exe", Qt::CaseInsensitive )) {
        pixmap = Worker::extractPixmap( info.absoluteFilePath());
        if ( !pixmap.isNull() && pixmap.width())
            data.pixmapList << pixmap;

        jumbo = Worker::extractPixmap( info.absoluteFilePath(), true );
        if ( !jumbo.isNull() && jumbo.width() > pixmap.width() && jumbo.height() > pixmap.height())
            data.pixmapList << jumbo;
    }

    return data;
}

/**
 * @brief Worker::run
 */
void Worker::run() {
    // enter event loop
    while ( !this->isInterruptionRequested()) {
        // LIFO - prioritizing most recent entries
        if ( !this->workList.isEmpty()) {
            Work work;
            work = this->workList.takeLast();
            work.data = this->work( work.fileName );
            emit this->workDone( work );

            // TODO: if this is the last one, emit finished and FLUSH TO DISK!!!

        } else {
            msleep( 100 );
          // qDebug() << "worker sleeping";
        }
    }
}
