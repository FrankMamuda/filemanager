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
#include <QtWin>
#include <QSysInfo>
#include "entry.h"
#include "pixmapcache.h"
#include "containermodel.h"
#include "pathutils.h"
#include <commctrl.h>
#include <commoncontrols.h>
#include <shellapi.h>

/**
 * @brief Entry::extractIcon
 */
/*
void Entry::extractIcon() {
    SHFILEINFO shellInfo;

    memset( &shellInfo, 0, sizeof( SHFILEINFO ));
    if ( SUCCEEDED( SHGetFileInfo( reinterpret_cast<const wchar_t *>( PathUtils::toWindowsPath( this->path()).utf16()), 0, &shellInfo, sizeof( SHFILEINFO ), SHGFI_ICON | SHGFI_SYSICONINDEX | SHGFI_ICONLOCATION | SHGFI_USEFILEATTRIBUTES | SHGFI_LARGEICON ))) {
        if ( shellInfo.hIcon ) {
            if ( QSysInfo::windowsVersion() >= QSysInfo::WV_VISTA ) {
                IImageList *imageList = NULL;

                if ( SUCCEEDED( SHGetImageList( 0x2, { 0x46eb5926, 0x582e, 0x4017, { 0x9f, 0xdf, 0xe8, 0x99, 0x8d, 0xaa, 0x9, 0x50 }}, reinterpret_cast<void **>( &imageList )))) {
                    HICON hIcon;

                    if ( SUCCEEDED( imageList->GetIcon( shellInfo.iIcon, ILD_TRANSPARENT, &hIcon ))) {
                        QPixmap pixmap;
                        pixmap = QtWin::fromHICON(hIcon);
                        DestroyIcon( hIcon );

                        if ( !pixmap.isNull()) {
                            this->icon = pixmap;
                            return;
                        }
                    }
                }
            }

            this->icon = QtWin::fromHICON( shellInfo.hIcon );
            DestroyIcon( shellInfo.hIcon );
        }
    }
}
*/

/**
 * @brief Entry::Entry
 * @param type
 * @param parent
 * @param fileInfo
 */
Entry::Entry( EntryTypes type, const QFileInfo &fileInfo, ContainerModel *parent ) : m_parent( parent ), m_fileInfo( fileInfo ), m_type( type ), m_cut( false ) {
    this->reset();

    //if ( fileInfo.fileName().endsWith( ".exe" )) {
    //    this->extractIcon();

        //this->shellIcons = extractlIcon( fileInfo.absoluteFilePath());
        //qDebug() << "extracted" << this->shellIcons.count() << "shell icons";

        //foreach ( PixmapEntry pe, this->shellIcons )
        //   qDebug() << "  " << pe.name;
        //
   // }

}

/**
 * @brief reset
 */
void Entry::reset() {
    //QMimeDatabase m;

    // first pass (by extension)
    // TOO SLOW, we just cannot affort this
    this->m_mimeType = QMimeType();
    //this->setMimeType( m.mimeTypeForFile( this->info(), QMimeDatabase::MatchExtension ));

    if ( this->isDirectory()) {
        switch ( this->type()) {
        case FileFolder:
            this->setIconName( "inode-directory" );
            break;

        case HardDisk:
            this->setIconName( "drive-harddisk" );
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
    //if ( this->info().fileName().endsWith( ".exe" ) && !this->icon.isNull())
    //    return this->icon;

    if ( this->type() == Entry::Executable )
        return this->iconPixmap();

    return pixmapCache.pixmap( this->iconName(), scale );
}
