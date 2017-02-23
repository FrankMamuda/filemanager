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

#ifndef TABLEVIEWDELEGATE_H
#define TABLEVIEWDELEGATE_H

//
// includes
//
#include <QStyledItemDelegate>

/**
 * @brief The TableViewDelegate class
 */
class TableViewDelegate : public QStyledItemDelegate {
public:
    TableViewDelegate( QObject *parent = NULL ) : QStyledItemDelegate( parent ) {}
    ~TableViewDelegate() {}

protected:
    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
};

#endif // TABLEVIEWDELEGATE_H