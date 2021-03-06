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
#include <QString>
#include <QDir>
#include <QFileSystemWatcher>

/**
 * @brief The PathUtils class
 */
class PathUtils {
public:
    PathUtils() {}
    static bool isUnixPath( const QString &path );
    static QString toUnixPath( const QString &path, bool homePath = false );
    static QString toWindowsPathE( const QString &path );
#ifdef Q_OS_WIN32
    static QString toWindowsPath( const QString &path ) { return QDir::toNativeSeparators( PathUtils::toWindowsPathE( path )); }
#else
    static QString toWindowsPath( const QString &path ) { return PathUtils::toUnixPath( path ); }
#endif
    static bool isWindowsDevicePath( const QString &path );
    static QString windowsDevicePath( const QString &path );
    QString currentPath;
    QFileSystemWatcher watcher;
};

extern class PathUtils pathUtils;
