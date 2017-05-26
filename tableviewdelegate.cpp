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
#include "tableviewdelegate.h"
#include <QPainter>
#include <QTableView>

/**
 * @brief TableViewDelegate::paint - similiar behaviour to ListViewDelegate::paint, tailored to QTableView
 * @param painter
 * @param option
 * @param index
 */
void TableViewDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const {
    QBrush hilightBrush;
    QStyleOptionViewItem optionNoSelection;
    QRect rect;
    QStyle::State state;
    QTableView *tableParent;
    int rowWidth = 0, y;

    // get parent container
    tableParent = qobject_cast<QTableView*>( this->parent());
    if ( tableParent == nullptr )
        return;

    // save painter state & get hilight brush
    painter->save();
    hilightBrush = option.palette.highlight();
    painter->setPen( Qt::NoPen );
    rect = option.rect;
    rect.setX( 0 );

    // get hilight width
    for ( y = 0; y < tableParent->model()->columnCount(); y++ )
        rowWidth += tableParent->columnWidth( y );
    rect.setWidth( rowWidth );

    // selected item
    if ( option.state & QStyle::State_Selected ) {
        hilightBrush.setColor( QColor::fromRgbF( hilightBrush.color().redF(), hilightBrush.color().greenF(), hilightBrush.color().blueF(), 0.50f ));
        painter->fillRect( option.rect, hilightBrush );
    }

    // mouseOver/hover item
    if ( option.state & QStyle::State_MouseOver && !( option.state & QStyle::State_Selected )) {
        hilightBrush.setColor( QColor::fromRgbF( hilightBrush.color().redF(), hilightBrush.color().greenF(), hilightBrush.color().blueF(), 0.25f ));
        painter->fillRect( rect, hilightBrush );
    }

    // restore painter state
    painter->restore();

    // remove hover/selection/focus flags, because we use custom hilights
    state = option.state;
    state = state & ( ~QStyle::State_MouseOver );
    state = state & ( ~QStyle::State_Selected );
    state = state & ( ~QStyle::State_HasFocus );
    state = state & ( ~QStyle::State_FocusAtBorder );

    optionNoSelection = option;
    optionNoSelection.state = state;

    // paint it exactly the same as before, yet ignoring selections
    QStyledItemDelegate::paint( painter, optionNoSelection, index );
}
