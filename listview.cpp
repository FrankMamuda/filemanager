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
#include "listview.h"
#include "containermodel.h"
#include <QStandardItemModel>
#include "entry.h"
#include <QApplication>

/**
 * @brief ListView::ListView
 * @param parent
 */
ListView::ListView( QWidget* parent ) : QListView( parent ), m_model( new ContainerModel( this, ContainerModel::ListContainer )) {
    // set model
    this->setModel( this->m_model );
    this->connect( this, SIGNAL( clicked( QModelIndex )), this->model(), SLOT( processItemOpen( QModelIndex )));

    // set view delegate
    this->setItemDelegate( new ListViewDelegate( this ));

    // enable mouse tracking
    this->setMouseTracking( true );
}

/**
 * @brief ListView::~ListView
 */
ListView::~ListView() {
    this->m_model->deleteLater();
}

/**
 * @brief ListView::setModel
 * @param model
 */
void ListView::setModel( ContainerModel *model ) {
    this->m_model = model;
    QListView::setModel( model );
}

/**
 * @brief ListView::switchDisplayMode
 */
void ListView::switchDisplayMode( ViewMode viewMode ) {
    int iconSize, horizontalSpacing, verticalSpacing;

    iconSize = this->model()->iconSize();
    this->setViewMode( viewMode );
    horizontalSpacing = 512 / iconSize;
    verticalSpacing = 128 / iconSize;

    if ( this->viewMode() == ListView::IconMode ) {
        this->setViewMode( ListView::IconMode );
        this->setFlow( QListView::LeftToRight );
        this->setGridSize( QSize( iconSize + horizontalSpacing, iconSize + this->fontMetrics().height() * 3 + verticalSpacing ));
    } else {
        this->setViewMode( ListView::ListMode );
        this->setFlow( QListView::TopToBottom );
        this->setGridSize( QSize( iconSize, iconSize ));
    }

    this->setIconSize( QSize( iconSize, iconSize ));
    this->model()->processEntries();
}

/**
 * @brief ListView::mouseReleaseEvent
 * @param e
 */
void ListView::mouseReleaseEvent( QMouseEvent *e ) {
    QModelIndex index;

    index = this->indexAt( e->pos());
    if ( e->button() == Qt::RightButton ) {
        if ( index.isValid())
            this->model()->processContextMenu( index, this->mapToGlobal( e->pos()) );
        else
            this->selectionModel()->clear();
    }
    QListView::mouseReleaseEvent( e );
}

/**
 * @brief ListView::selectionChanged
 * @param selected
 * @param deselected
 */
void ListView::selectionChanged( const QItemSelection &selected, const QItemSelection &deselected ) {
    this->model()->setSelection( this->selectionModel()->selectedIndexes());
    QListView::selectionChanged( selected, deselected );
}

/**
 * @brief ListView::dropEvent
 * @param e
 */
void ListView::dropEvent( QDropEvent *e ) {
    this->model()->processDropEvent( this->indexAt( e->pos()), this->mapToGlobal( e->pos()) );
    e->accept();
}

/**
 * @brief entered
 * @param index
 */
void ListView::mouseMoveEvent( QMouseEvent *event ) {
    this->model()->processMouseMove( this->indexAt( event->pos()));
    QAbstractItemView::mouseMoveEvent( event );
}