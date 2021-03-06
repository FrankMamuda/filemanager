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
#include <QMimeDatabase>
#ifdef Q_OS_WIN32
#include <QtWin>
#endif
#include <QSysInfo>
#include "entry.h"
#include "pixmapcache.h"
#include "containermodel.h"
#include "pathutils.h"
#include "main.h"

/**
 * @brief Entry::Entry
 * @param type
 * @param parent
 * @param fileInfo
 */
Entry::Entry( EntryTypes type, const QFileInfo &fileInfo, ContainerModel *parent ) : m_parent( parent ), m_fileInfo( fileInfo ), m_type( type ), m_cut( false ), m_updated( false ) {
    this->reset();
}

/**
 * @brief Entry::getDriveIconName
 * @param info
 * @return
 */
QString Entry::getDriveIconName( const QFileInfo &info ) {
#ifdef Q_OS_WIN32
    UINT type;

    type = GetDriveType(( wchar_t * )info.absoluteFilePath().utf16());
    switch ( type ) {
    case DRIVE_REMOVABLE:
        return "drive-removable-media";
        break;

    case DRIVE_REMOTE:
        return "network-workgroup";
        break;

    case DRIVE_CDROM:
        return "media-optical";
        break;

    case DRIVE_RAMDISK:
        return "media-flash";
        break;

    case DRIVE_FIXED:
        return "drive-harddisk";
        break;

    case DRIVE_UNKNOWN:
    case DRIVE_NO_ROOT_DIR:
    default:
        return "drive-removable-media";
    }
#else
    return "drive-removable-media";
#endif
}

/**
 * @brief reset
 */
void Entry::reset() {
    this->m_mimeType = QMimeType();

    if ( this->isDirectory()) {
        switch ( this->type()) {
        case FileFolder:
            this->setIconName( "folder" );
            break;

        case HardDisk:
            this->setIconName( Entry::getDriveIconName( this->info()));
            break;

        case Root:
            this->setIconName( "folder-red" );
            break;

        case Home:
            this->setIconName( "user-home" );
            break;

        case Trash:
            this->setIconName( "user-trash" );
            break;

        default:
            break;
        }
    }
}

/**
 * @brief Entry::alias
 * @return
 */
QString Entry::alias() const {
    if ( this->type() == Root )
        return "Root";
    else if ( this->type() == Home )
        return "Home";
    else if ( this->type() == Trash )
        return "Trash";

    if ( this->info().isRoot())
        return PathUtils::toUnixPath( this->info().absolutePath());

    return this->info().fileName();
}

/**
 * @brief Entry::path
 * @return
 */
QString Entry::path() const {
    if ( this->type() == Root )
        return "/";
    else if ( this->type() == Home )
        return "/home/";
    else if ( this->type() == Trash )
        return "trash://";

    if ( this->info().isSymLink()) {
        QFileInfo target( this->info().symLinkTarget());

        if ( !target.isSymLink())
            return target.absoluteFilePath();
    }

    return this->info().absoluteFilePath();
}

/**
 * @brief Entry::isDirectory
 * @return
 */
bool Entry::isDirectory() const {
    if ( this->type() == Root || this->type() == Home || this->type() == Trash )
        return true;

    return this->info().isDir();
}

/**
 * @brief Entry::pixmap
 * @param scale
 * @return
 */
QPixmap Entry::pixmap( int scale ) const {
    QPixmap pixmap;

    if ( this->type() == Thumbnail || this->type() == Executable )
        pixmap = this->iconPixmap();
    else
        pixmap = m.pixmapCache->pixmap( this->iconName(), scale );

    if ( pixmap.isNull() || !pixmap.width())
        return m.pixmapCache->pixmap( this->info().absoluteFilePath(), scale );

    return pixmap;
}
