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
#include "filestream.h"
#include <QDir>
#include <QPixmap>

//
// classes
//
class SideView;

/**
 * @brief The BookMarkSystem namespace
 */
namespace BookMarkSystem {
    static const quint8 Version = 1;
    static const QString DataFilename( "bookmarks" );
}

/**
 * @brief The BookmarkEntry struct
 */
struct BookmarkEntry {
    BookmarkEntry( const QString &a = QString::null, const QString &p = QString::null, const QPixmap &pm = QPixmap()) : alias( a ), path( p ), pixmap( pm ) {}
    QString alias;
    QString path;
    QPixmap pixmap;
};
Q_DECLARE_METATYPE( BookmarkEntry )

// read/write operators
inline static QDataStream &operator<<( QDataStream &out, const BookmarkEntry &e ) { out << e.alias << e.path << e.pixmap; return out; }
inline static QDataStream &operator>>( QDataStream &in, BookmarkEntry &e ) { in >> e.alias >> e.path >> e.pixmap; return in; }

/**
 * @brief The Bookmark class
 */
class Bookmark : public QObject {
    Q_OBJECT
    Q_PROPERTY( QString path READ path )
    Q_PROPERTY( bool valid READ isValid )
    friend class BookmarkModel;

public:
    enum BookmarkData {
        Alias = 0,
        Path,
        Pixmap
    };
    Q_ENUMS( BookmarkData )

    Bookmark( const QString &path );
    ~Bookmark() {}
    int count() { return this->list.count(); }
    void add( const QString &alias, const QString &path, const QPixmap &pixmap = QPixmap(), bool writeOut = true ) { this->add( BookmarkEntry( alias, path, pixmap ), writeOut ); }
    void add( const BookmarkEntry &entry, bool writeOut = true );
    void remove( int pos );
    void shutdown() { this->setValid( false ); this->data.close(); }
    static QPixmap iconNameToPixmap( const QString &iconName );
    QVariant value( int index, BookmarkData field );
    void setValue( int index, BookmarkData field, const QVariant &value );

private slots:
    void write();
    void read();
    void setValid( bool valid ) { this->m_valid = valid; }

private:
    Q_DISABLE_COPY( Bookmark )

    bool isValid() const { return this->m_valid; }
    QString path() const { return this->m_path; }

    QList<BookmarkEntry> list;
    QString m_path;
    bool m_valid;
    QDir bookmarkDir;
    FileStream data;
};
