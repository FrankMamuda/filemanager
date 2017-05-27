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
#include <QFileIconProvider>
#include <QDebug>
#include <QtGui/QIconEngine>
#include <QDir>
#include <QDomDocument>

//
// class: PixmapCache
//
class PixmapCache pixmapCache;

//
// add support for forced icon size (not pixmap)
// add support for multiple themes (for example sideview need dark icons)
// TODO: use FALLBACK!!!

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
            pixmap = this->findPixmap( name, scale );// QIcon::fromTheme( name ).pixmap( scale, scale );
        else {
            QFileInfo info( name );

            if ( info.isSymLink())
                pixmap.load( info.symLinkTarget());
            else
                pixmap.load( name );
        }

        // handle missing icons
        if ( pixmap.isNull() || !pixmap.width()) {
            pixmap = this->findPixmap( "application-x-zerosize", scale ); //QIcon::fromTheme( "application-x-zerosize" ).pixmap( scale, scale );

            // failsafe, in case something doesn't work as intended
            // NOTE: for some reason some icons fail to load
            if ( pixmap.width() == 0 ) {
                // try one more time with QFileIconProvider
                QFileIconProvider p;
                if ( pixmap.isNull() || !pixmap.width())
                    pixmap = p.icon( QFileInfo( name )).pixmap( scale, scale );

                if ( pixmap.isNull() || !pixmap.width())
                    return QPixmap();
            }
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

/**
 * @brief PixmapCache::buildIndex
 */
void PixmapCache::buildIndex( const QString &themeName ) {
    QString iconPath, buffer;
    QFile index;
    QRegExp rx;
    QStringList dirList;
    int pos;

    // set first as default theme
    if ( this->defaultTheme.isEmpty())
        this->defaultTheme = themeName;

    // currently get the first theme search path
    iconPath = QIcon::themeSearchPaths().first() + "/" + themeName;//QIcon::themeName();

    // open theme index file
    index.setFileName( iconPath + "/" + "index.theme" );
    if ( !index.open( QFile::ReadOnly ))
        return;

    // read index file
    buffer = QString( index.readAll().constData());

    // set pattern to extract icon directories
    rx.setPattern( "Directories=(.+)\\s" );

    // extract icon directories
    pos = 0;
    while (( pos = rx.indexIn( buffer, pos )) != -1 ) {
        dirList << rx.cap( 1 );
        pos += rx.matchedLength();
    }

    // abort on no directorues
    if ( !dirList.count())
        return;

    // append directories to index
    dirList = dirList.first().split( "," );
    foreach ( QString dir, dirList )
        this->index[themeName] << iconPath + "/" + dir;

    // close index file
    index.close();
}

/**
 * @brief PixmapCache::parseSVG
 * @param buffer
 * @return
 */
int PixmapCache::parseSVG( const QString &buffer ) {
    QDomDocument doc;
    QDomNodeList svgNodes;
    int width = 0, height = 0, scale = 0;

    // parse svg as an XML document
    doc.setContent( buffer );

    // get root node
    svgNodes = doc.elementsByTagName( "svg" );
    if ( svgNodes.size()) {
        QDomElement element;

        // convert node to element
        element = svgNodes.at( 0 ).toElement();

        // get width directly
        if ( element.hasAttribute( "width" ))
            width = element.attribute( "width" ).toInt();

        // get height directly
        if ( element.hasAttribute( "height" ))
            height = element.attribute( "height" ).toInt();

         // extract width and height from viewBox (override)
        if ( element.hasAttribute( "viewBox" )) {
            QStringList parms;

            parms = element.attribute( "viewBox" ).split( " " );
            if ( parms.count() == 4 ) {
                width = parms.at( 2 ).toInt();
                height = parms.at( 3 ).toInt();
            }
        }
    }

    // done parsing
    doc.clear();

    // store size
    if ( width && height )
        scale = width;

    return scale;
}

/**
 * @brief PixmapCache::readIconFile
 * @param fileName
 * @return
 */
IconMatch PixmapCache::readIconFile( const QString &fileName, bool &ok, int recursionLevel ) {
    QString buffer;
    bool binary = false;
    QFile file( fileName );
    QByteArray data;
    IconMatch iconMatch( fileName );

    // bad icon by default
    ok = false;

    // attempt to read icon file
    if ( !file.open( QFile::ReadOnly ))
        return iconMatch;

    // read whole file
    data = file.readAll();

    // avoid recursions
    recursionLevel--;
    if ( recursionLevel < 0 )
        return iconMatch;

    // convert to text in case it is an svg or a symbolic link
    buffer = QString( data.constData());

    // test if file is binary
    // TODO: optimize this, no need to read the whole file, first 100 bytes should be enough
    for ( QString::ConstIterator y = buffer.constBegin(), end = buffer.constEnd(); y != end; y++ ) {
        if ( y->unicode() > 127 )  {
            binary = true;
            break;
        }
    }

    if ( !binary ) {
        // test if svg
        if ( buffer.startsWith( "<svg" ) || buffer.startsWith( "<?xml" )) {
            iconMatch.scale = this->parseSVG( buffer );
        } else {
            QFileInfo info( file );
            QDir dir;
            QString link;
            int pos = 0, numCdUps = 0;

            // construct filename from symlink
            dir.setPath( info.absolutePath());
            link = buffer;
            while (( pos = buffer.indexOf( "../", pos )) != -1 ) {
                link = buffer.mid( pos + 3, buffer.length() - pos - 3 );
                dir.cdUp();
                pos++;
                numCdUps++;
            }
            link = dir.absolutePath() + "/" + link;

            // recursively read symlink target
            iconMatch.fileName = link;
            return this->readIconFile( link, ok, recursionLevel );
        }
    } else {
        QPixmap pixmap;

        // get size directly from pixmap
        if ( pixmap.load( fileName ))
            iconMatch.scale = pixmap.width();
    }

    // clean up
    data.clear();
    buffer.clear();
    file.close();

    // check scale
    if ( iconMatch.scale > 0 )
        ok = true;

    // return icon match
    return iconMatch;
}

/**
 * @brief PixmapCache::findIcon
 * @param name
 * @return
 */
// FIXME: precache icons???
QPixmap PixmapCache::findPixmap( const QString &name, int scale, const QString &themeName ) {
    int y = 0, bestIndex = 0, bestScale = 0;
    IconMatchList matchList;

    // get icon match list
    matchList = this->getIconMatchList( name, themeName );

    // don't bother if no matches
    if ( matchList.isEmpty())
        return QPixmap();

    // go through all matches
    foreach ( IconMatch iconMatch, matchList ) {
        if ( iconMatch.scale == scale ) {
            bestIndex = y;
            bestScale = iconMatch.scale;
            break;
        } else if ( iconMatch.scale > bestScale ) {
            bestScale = iconMatch.scale;
            bestIndex = y;
        }

        y++;
    }

    return QPixmap( matchList.at( bestIndex ).fileName ).scaled( scale, scale, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
}

/**
 * @brief PixmapCache::fromTheme some icon themes from have folders full of symlinks to avoid duplicate icons
 * unfortunately QIcon does not handle these symlinks (plain text files). Moreover, since we cannot reliably
 * get a filename from QIcon::fromTheme, we must manually find the best matching icon here while resolving all
 * symlinks
 * @param name
 * @return
 */

// TODO: early quit if matching size found?
QIcon PixmapCache::findIcon( const QString &name, int scale, const QString &themeName ) {
    int y = 0, bestIndex = 0, bestScale = 0;
    IconMatchList matchList;

    // get icon match list
    matchList = this->getIconMatchList( name, themeName );

    // don't bother if no matches
    if ( matchList.isEmpty())
        return QIcon();

    // go through all matches
    // TODO: optimize code
    if ( scale == 0 ) {
        foreach ( IconMatch iconMatch, matchList ) {
            if ( iconMatch.scale > bestScale ) {
                bestScale = iconMatch.scale;
                bestIndex = y;
            }

            y++;
        }
    } else {
        foreach ( IconMatch iconMatch, matchList ) {
            if ( iconMatch.scale == scale ) {
                bestIndex = y;
                bestScale = iconMatch.scale;
                break;
            } else if ( iconMatch.scale > bestScale ) {
                bestScale = iconMatch.scale;
                bestIndex = y;
            }

            y++;
        }
    }

    return QIcon( matchList.at( bestIndex ).fileName );
}

/**
 * @brief PixmapCache::getIconMatchList
 * @param name
 * @return
 */
IconMatchList PixmapCache::getIconMatchList( const QString &name, const QString &themeName ) {
    QDir dir;
    QStringList iconPathList;
    IconMatchList matchList;
    int recursionLevel = 2;
    QString theme;

    // revert to default if no theme is specified
    if ( themeName.isEmpty())
        theme = this->defaultTheme;
    else
        theme = themeName;

    // check theme name
    if ( !this->index.contains( theme ))
        return matchList;

    // set directory filter to find png and svg icons only
    dir.setNameFilters( QStringList() << ( name + ".png" ) << ( name + ".svg" ));

    // go through all directories
    foreach ( QString path, this->index[theme] ) {
        dir.setPath( path );

        // append all matched paths
        foreach ( QString match, dir.entryList())
            iconPathList << ( dir.absolutePath() + "/" + match );
    }

    // don't bother if no matches
    if ( iconPathList.isEmpty())
        return matchList;

    // go through all filenames
    foreach ( QString iconPath, iconPathList ) {
        bool ok;
        IconMatch iconMatch;

        // TODO: set real filename!!!
        iconMatch = this->readIconFile( iconPath, ok, recursionLevel );
        if ( ok )
            matchList << iconMatch;
    }

    return matchList;
}
