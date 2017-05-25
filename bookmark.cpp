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
#include "bookmark.h"
#include <QIcon>
#include <QDebug>
#include "pathutils.h"
#include "entry.h"
#include "pixmapcache.h"

/**
 * @brief Bookmark::Bookmark
 * @param path
 */
Bookmark::Bookmark(const QString &path) : m_path( path ), m_valid( true ) {
    this->bookmarkDir = QDir( this->path());

    // check if bookmark dir exists
    if ( !this->bookmarkDir.exists()) {
        qDebug() << this->tr( "Bookmark: creating non-existant bookmark dir" );
        this->bookmarkDir.mkpath( this->path());

        // additional failsafe
        if ( !this->bookmarkDir.exists()) {
            this->setValid( false );
            return;
        }
    }

    // set up data file
    this->data.setFilename( bookmarkDir.absolutePath() + "/" + BookMarkSystem::DataFilename );
    if ( !this->data.open()) {
        qDebug() << this->tr( "Bookmark: index data non-writable" );
        this->setValid( false );
        return;
    } else {
        if ( !this->data.size())
            this->data << BookMarkSystem::Version;
    }

    // read stored entries
    this->read();

    // add default bookmarks if none
    if ( !this->count()) {
        QFileInfoList infoList;
        this->add( "Root", "/", Bookmark::iconNameToPixmap( "folder-red" ), false );
        this->add( "Home", "/home", Bookmark::iconNameToPixmap( "user-home" ), false );

        infoList = QDir::drives();
        foreach ( QFileInfo driveInfo, infoList )
            this->add( PathUtils::toUnixPath( driveInfo.absolutePath()), PathUtils::toUnixPath( driveInfo.absolutePath()), Bookmark::iconNameToPixmap( Entry::getDriveIconName( driveInfo )), false );

        this->add( "Trash", "trash://", Bookmark::iconNameToPixmap( "user-trash" ), false );

        // write out
        this->write();
    }
}

/**
 * @brief Bookmark::add
 * @param entry
 */
void Bookmark::add( const BookmarkEntry &entry, bool writeOut ) {
    // failsafe
    if ( !this->isValid())
        return;

    // add item and commit
    this->list << entry;

    // write out
    if ( writeOut )
        this->write();
}

/**
 * @brief Bookmark::remove
 * @param pos
 */
void Bookmark::remove( int pos ) {
    // failsafe
    if ( !this->isValid())
        return;

    // reject invalid indexes
    if ( pos < 0 || pos >= this->list.count())
        return;

    // remove item and commit
    this->list.removeAt( pos );
    this->write();
}

/**
 * @brief Bookmark::iconNameToPixmap
 * @param iconName
 */
QPixmap Bookmark::iconNameToPixmap( const QString &iconName ) {
    return pixmapCache.findPixmap( iconName, 32 );// QIcon::fromTheme( iconName ).pixmap( 32, 32 );
}

/**
 * @brief Bookmark::value
 * @param index
 * @param field
 * @return
 */
QVariant Bookmark::value( int index, BookmarkData field ) {
    if ( index < 0 || index >= this->count())
        return QVariant();

    switch ( field ) {
    case Alias:
        return this->list.at( index ).alias;

    case Path:
        return this->list.at( index ).path;

    case Pixmap:
        return this->list.at( index ).pixmap;

    default:
        break;
    }

    return QVariant();
}

/**
 * @brief Bookmark::setValue
 * @param index
 * @param field
 * @param value
 */
void Bookmark::setValue( int index, Bookmark::BookmarkData field, const QVariant &value ) {
    BookmarkEntry entry;

    if ( index < 0 || index >= this->count())
        return;

    entry = this->list.at( index );

    switch ( field ) {
    case Alias:
        entry.alias = value.toString();
        break;

    case Path:
        entry.path = value.toString();
        break;

    case Pixmap:
        entry.pixmap = value.value<QPixmap>();
        break;

    default:
        return;
    }

    qDebug() << "replace" << this->list.at( index ).alias << "of index" << index << "with" << entry.alias;
    this->list.replace( index, entry );
    this->write();
}


/**
 * @brief Bookmark::write
 */
void Bookmark::write() {
    // failsafe
    if ( !this->isValid())
        return;

    // truncate bookmark file
    this->data.resize( 1 );
    this->data.toEnd();

    // write out
    foreach ( BookmarkEntry entry, this->list )
        this->data << entry;

    // flush to disk immediately
    this->data.sync();
}

/**
 * @brief Bookmark::read
 */
void Bookmark::read() {
    // failsafe
    if ( !this->isValid())
        return;

    // read index file version
    quint8 version;
    this->data.toStart();
    this->data >> version;

    // check version
    if ( version != BookMarkSystem::Version ) {
        qDebug() << this->tr( "Bookmark::read: version mismatch for data file" );
        this->setValid( false );
        return;
    }

    // read indexes
    while ( !this->data.atEnd()) {
        BookmarkEntry bookmarkEntry;
        this->data >> bookmarkEntry;
        this->list << bookmarkEntry;
    }

    // report
    qDebug() << this->tr( "Bookmark::read: found %1 bookmarks from file" ).arg( this->count());
}
