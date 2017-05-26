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
#include <QAbstractListModel>
#include <QAbstractItemView>

//
// classes
//
class Bookmark;

/**
 * @brief The BookmarkModel class
 */
class BookmarkModel : public QAbstractListModel {
    Q_OBJECT

public:
    explicit BookmarkModel( QAbstractItemView *parent );
    ~BookmarkModel();

    // overrides
    int rowCount( const QModelIndex & = QModelIndex()) const;
    void reset() { this->beginResetModel(); this->endResetModel(); }
    QVariant data( const QModelIndex &index, int role ) const;
    Qt::ItemFlags flags( const QModelIndex &index ) const;
    Qt::DropActions supportedDropActions() const { return Qt::CopyAction | Qt::MoveAction; }
    Qt::DropActions supportedDragActions() const { return Qt::CopyAction | Qt::MoveAction; }
    QMimeData *mimeData( const QModelIndexList &indexes ) const;

    // parent widget
    QAbstractItemView *parent() const { return this->m_parent; }

    // bookmarks
    Bookmark *bookmarks() const { return this->m_bookmarks; }

private:
    QAbstractItemView *m_parent;
    Bookmark *m_bookmarks;
};
