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
#include "main.h"

/**
 * @brief Bookmark::value
 * @param index
 * @param field
 * @return
 */
QString Bookmark::value( int index, Bookmark::BookmarkData field ) {
    QStringList values;

    values = Variable::value<QStringList>( QString( "bookmarks/bookmark_%1" ).arg( index ));
    if ( field < 0 || field >= values.count())
        return QString::null;

    return values.at( field );
}

/**
 * @brief Bookmark::count
 * @return
 */
int Bookmark::count() {
    int count;

    m.settings->beginGroup( "bookmarks" );
    count = m.settings->childKeys().count();
    m.settings->endGroup();

    return count;
}

/**
 * @brief Bookmark::add
 * @param alias
 * @param path
 * @param iconName
 */
void Bookmark::add( const QString &alias, const QString &path, const QString &iconName, int insert ) {
    int count;
    QStringList values;

    Q_UNUSED( insert )

    count = Bookmark::count();
    values << alias << path << iconName;
    Variable::setValue<QStringList>( QString( "bookmarks/bookmark_%1" ).arg( count ), values );
}

/**
 * @brief Bookmark::setValue
 * @param index
 * @param field
 * @param value
 */
void Bookmark::setValue( int index, Bookmark::BookmarkData field, const QString &value ) {
    QStringList values;

    values = Variable::value<QStringList>( QString( "bookmarks/bookmark_%1" ).arg( index ));
    if ( field < 0 || field >= values.count())
        return;

    values.replace( field, value );
    Variable::setValue<QStringList>( QString( "bookmarks/bookmark_%1" ).arg( index ), values );
}
