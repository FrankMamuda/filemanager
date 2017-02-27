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

#ifndef BOOKMARK_H
#define BOOKMARK_H

//
// includes
//
#include "variable.h"
#include <QStringList>

/**
 * @brief The BookMark class
 */
class Bookmark {
public:
    enum BookmarkData {
        Alias = 0,
        Path,
        IconName
    };

    static int count();
    static void add( const QString &alias = QString::null, const QString &path = QString::null, const QString &iconName = QString::null, int insert = -1 );
    static void setValue( int index, Bookmark::BookmarkData field, const QString &value );
    static QString value( int index, Bookmark::BookmarkData field );
};

#endif // BOOKMARK_H
