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
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QDir>
#include "pathutils.h"

//
// class: PathUtils
//
class PathUtils pathUtils;

/**
 * @brief PathUtils::isUnixPath
 * @param path
 * @return
 */
bool PathUtils::isUnixPath( const QString &path ) {
    QRegularExpression re( "^([A-Z]{1}):" );
    QRegularExpressionMatch m;

    m = re.match( path );
    if ( m.hasMatch())
        return false;

    return true;
}

/**
 * @brief PathUtils::isWindowsDevicePath
 * @param path
 * @return
 */
bool PathUtils::isWindowsDevicePath( const QString &path ) {
    QRegularExpression re( "^([A-Z]{1}):[\\/|\\\\]?$" );
    QRegularExpressionMatch m;

    m = re.match( path );
    if ( m.hasMatch())
        return true;

    return false;
}

/**
 * @brief PathUtils::windowsDevicePath
 * @param path
 * @return
 */
QString PathUtils::windowsDevicePath( const QString &path ) {
    QRegularExpression re( "^([A-Z]{1}:[\\/|\\\\]).+$" );
    QRegularExpressionMatch m;

    m = re.match( path );
    if ( m.hasMatch())
        return m.captured( 1 );

    return "";
}

/**
 * @brief PathUtils::toUnixPath
 * @param path
 * @return
 */
QString PathUtils::toUnixPath(const QString &path, bool homePath ) {
    QRegularExpression re( "^([A-Z]{1}):(.+)" );
    QRegularExpressionMatch m;
    QString mount, dir, out, home;

    if ( PathUtils::isUnixPath( path ))
        return path;

    m = re.match( path );
    if ( m.hasMatch()) {
        mount = m.captured( 1 ).toLower();
        dir = m.captured( 2 ).replace( '\\', '/' );

        if ( !dir.startsWith( '/' ))
            dir = dir.prepend( '/' );

        out = QString( "/%1%2" ).arg( mount ).arg( dir );

        if ( !homePath ) {
            home = PathUtils::toUnixPath( QDir::home().absolutePath(), true );
            if ( out.startsWith( home ))
                out.replace( home, "/home" );
        }

        return out;
    }

    return path;
}

/**
 * @brief PathUtils::toWindowsPath
 * @param path
 * @return
 */
QString PathUtils::toWindowsPathE( const QString &path ) {
    QRegularExpression re( "^\\/([A-z]+)[\\/|\\\\]?(.+)?" );
    QRegularExpressionMatch m;
    QString mount, dir;

    if ( !PathUtils::isUnixPath( path ))
        return path;

    // handle special cases
    if ( !QString::compare( path, "/" ))
        return path;

    m = re.match( path );
    if ( m.hasMatch()) {
        mount = m.captured( 1 );
        dir = m.captured( 2 );

        // letter drives
        if ( mount.length() == 1 ) {
            mount = mount.toUpper();
        } else {
            // TODO: in future compare mount points in some kind of virtual fstab?
            // NOTE: not sure about drives and user paths:
            //       option1: /home/<username>; /mnt/<drivename>
            //       option1: /home = user home; /? = mountpoint for drives
            if ( !QString::compare( mount, "home" ))
                return QString( "%1/%2" ).arg( QDir::home().absolutePath()).arg( dir ).replace( '/', '\\' );
            else
                return path;
        }

        if ( !dir.startsWith( '/' ))
            dir = dir.prepend( '/' );

        return QString( "%1:%2" ).arg( mount ).arg( dir.replace( '/', '\\' ));
    }

    return path;
}
