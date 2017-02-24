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

#ifndef BOOKMARKMODEL_H
#define BOOKMARKMODEL_H

//
// includes
//
#include <QAbstractListModel>
#include <QAbstractItemView>

/**
 * @brief The BookmarkModel class
 */
class BookmarkModel : public QAbstractListModel {
    Q_OBJECT

public:
    explicit BookmarkModel( QAbstractItemView *parent ) : m_listParent( parent ) {}

    // overrides
    int rowCount( const QModelIndex & = QModelIndex()) const;
    void reset();
    QVariant data( const QModelIndex &index, int role ) const;
    QAbstractItemView *listParent() const { return this->m_listParent; }

public slots:
    void processItemOpen( const QModelIndex &index );

private:
    QAbstractItemView *m_listParent;
};

#endif // BOOKMARKMODEL_H
