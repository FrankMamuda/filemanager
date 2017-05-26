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
#include "listviewdelegate.h"
#include "listview.h"
#include "containermodel.h"
#include "sideview.h"

// TODO: remove ugly drag/drop black borders both for List/Table views

/**
 * @brief ListViewDelegate::ListViewDelegate
 * @param parent
 */
ListViewDelegate::ListViewDelegate( QListView *parent ) {
    this->setParent( qobject_cast<QObject*>( parent ));
}

/**
 * @brief ListViewDelegate::sizeHint
 * @param option
 * @param index
 * @return
 */
QSize ListViewDelegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const {
    QSize size;
    ContainerItem item;
    QListView::ViewMode viewMode;

    // get view mode
    viewMode = qobject_cast<QListView*>( this->parent())->viewMode();

    // calculate proper size for multi-line text
    item = qvariant_cast<ContainerItem>( index.model()->data( index, Qt::UserRole + ContainerModel::DisplayItem ));
    size = QStyledItemDelegate::sizeHint( option, index );

    if ( viewMode != QListView::ListMode )
        size.setHeight( option.decorationSize.height() + item.lines.count() * item.textHeight );

    return size;
}

/**
 * @brief ListViewDelegate::paint
 * @param painter
 * @param option
 * @param index
 */
void ListViewDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const {
    QListView::ViewMode viewMode;
    bool bookMarkView = false;

    // get view mode
    viewMode = qobject_cast<QListView*>( this->parent())->viewMode();
    bookMarkView = qobject_cast<SideView*>( this->parent()) != nullptr;

    //
    // STAGE 0: display hilight
    //
    QBrush hilightBrush;

    // save painter state & get hilight brush
    painter->save();
    hilightBrush = option.palette.highlight();
    painter->setPen( Qt::NoPen );

    // selected item
    // NOTE: ignoring selection rect with SideView container
    // ultimately, we should use a separate delegate to allow drop insertion between items
    if ( option.state & QStyle::State_Selected && !bookMarkView ) {
        hilightBrush.setColor( QColor::fromRgbF( hilightBrush.color().redF(), hilightBrush.color().greenF(), hilightBrush.color().blueF(), 0.50f ));
        painter->fillRect( option.rect, hilightBrush );
    }

    // mouseOver/hover item
    if ( option.state & QStyle::State_MouseOver && !( option.state & QStyle::State_Selected )) {
        hilightBrush.setColor( QColor::fromRgbF( hilightBrush.color().redF(), hilightBrush.color().greenF(), hilightBrush.color().blueF(), 0.25f ));
        painter->fillRect( option.rect, hilightBrush );
    }

    // restore painter state
    painter->restore();

    //
    // STAGE 1: display pixmap
    //
    int width, height, offset;
    bool isCut;

    if ( viewMode == QListView::IconMode ) {
        QRect pixmapRect;
        QPixmap pixmap;

        // get pixmap and its dimensions
        pixmap = qvariant_cast<QPixmap>( index.model()->data( index, Qt::DecorationRole ));
        isCut = index.model()->data( index, Qt::UserRole + 1 ).toBool();
        pixmapRect = option.rect;
        width = option.decorationSize.width();
        height = option.decorationSize.height();

        // properly position pixmap
        if ( width < pixmapRect.width()) {
            if ( viewMode == QListView::IconMode ) {
                offset = pixmapRect.width() - width;
                pixmapRect.setX( pixmapRect.x() + offset / 2 );
            }

            pixmapRect.setWidth( width );
        }

        if ( isCut )
            painter->setOpacity( 0.5f );

        // draw pixmap
        pixmapRect.setHeight( height );
        painter->drawPixmap( pixmapRect, pixmap );

        if ( isCut )
            painter->setOpacity( 1.0f );
    }

    //
    // STAGE 3: display text
    //
    if ( viewMode == QListView::IconMode ) {
        int y;
        QRect textRect;
        QTextOption to;
        ContainerItem item;

        // get pre-calculated display item
        item = qvariant_cast<ContainerItem>( index.model()->data( index, Qt::UserRole + ContainerModel::DisplayItem ));
        to.setAlignment( Qt::AlignHCenter );

        // init text rectangle
        textRect = option.rect;
        textRect.setY( textRect.y() + height - item.textHeight );

        // display multi-line text
        for ( y = 0; y < item.lines.count(); y++ ) {
            textRect.setX( textRect.x() + ( textRect.width() - item.lineWidths.at( y )) / 2 );
            textRect.setY( textRect.y() + item.textHeight );
            textRect.setHeight( item.textHeight );
            textRect.setWidth( item.lineWidths.at( y ));
            painter->drawText( textRect, item.lines.at( y ), to );
        }
    } else {
        QStyleOptionViewItem optionNoSelection;
        QStyle::State state;

        // remove hover/selection flags
        state = option.state;
        state = state & ( ~QStyle::State_MouseOver );
        state = state & ( ~QStyle::State_Selected );
        state = state & ( ~QStyle::State_HasFocus );
        state = state & ( ~QStyle::State_Active );

        optionNoSelection = option;
        optionNoSelection.state = state;

        // paint it exactly the same as before, yet ignoring selections
        QStyledItemDelegate::paint( painter, optionNoSelection, index );
    }

    // the in-between line
#if 0
    if ( bookMarkView ) {
        int lineY = qobject_cast<SideView*>( this->parent())->mapFromGlobal( QCursor::pos()).y();
        painter->drawLine( 0, lineY, option.widget->width(), lineY );
    }
#endif
}
