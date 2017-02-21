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
#include "containermodel.h"
#include "tableview.h"
#include <QPainter>
#include <QScrollBar>
#include <QDebug>
#include <QHeaderView>
#include "variable.h"
#include "main.h"

// TODO: remove dotted outline
//   transfer selection from QListView
//   fix jumpy scroll (move to top when cursor is outside widget?)
//   ignore custom selection event if modifier key is pressed

/**
 * @brief TableView::TableView
 * @param parent
 */
TableView::TableView( QWidget *parent ) : QTableView( parent ), m_model( new ContainerModel( this, ContainerModel::TableContainer, ContainerModel::FileMode, 16 )), m_selection( false ) {
    this->setModel( this->m_model );
    this->connect( this, SIGNAL( clicked( QModelIndex )), this->model(), SLOT( processItemOpen( QModelIndex )));
    this->connect( this->verticalScrollBar(), SIGNAL( valueChanged( int )), this->viewport(), SLOT( update()));
    this->connect( this->horizontalHeader(), SIGNAL( geometriesChanged()), this, SLOT( headerResized()));

    for ( int y = 0; y < this->model()->columnCount(); y++ ) {
        Variable *var = Variable::add( QString( "tableView/horizontalHeader/sectionSize_%1" ).arg( y ), m.settings, 100 );
        if ( var != NULL )
            this->horizontalHeader()->resizeSection( y, var->integer());
    }

    // enable mouse tracking
    this->setMouseTracking( true );
}

/**
 * @brief TableView::headerResized
 */
void TableView::headerResized() {
    int y = 0;

    for ( y = 0; y < this->model()->columnCount(); y++ )
        Variable::setValue( QString( "tableView/horizontalHeader/sectionSize_%1" ).arg( y ), this->horizontalHeader()->sectionSize( y ));
}

/**
 * @brief TableView::~TableView
 */
TableView::~TableView() {
    this->m_model->deleteLater();
}

/**
 * @brief TableView::setModel
 * @param model
 */
void TableView::setModel( ContainerModel *model ) {
    this->m_model = model;
    QTableView::setModel( model );
}

/**
 * @brief TableView::mouseReleaseEvent
 * @param e
 */
void TableView::mouseReleaseEvent( QMouseEvent *e ) {
    QModelIndex index;

    this->m_selection = false;
    this->viewport()->update();

    index = this->indexAt( e->pos());
    if ( e->button() == Qt::RightButton ) {
        if ( index.isValid())
            this->model()->processContextMenu( index, this->mapToGlobal( e->pos()));
        else
            this->selectionModel()->clear();
    }
    QTableView::mouseReleaseEvent( e );
}

/**
 * @brief TableView::mouseMoveEvent
 */
void TableView::mouseMoveEvent( QMouseEvent *e ) {
    QPoint point;

    point = this->viewport()->mapFromGlobal( QCursor::pos());

    // ugly scroll fix
    if ( point.y() < 32 )
        this->scrollToTop();
    else if ( point.y() > this->viewport()->height() + 32 )
        this->scrollToBottom();

    if ( this->m_selection ) {
        QRect rect;
        int x0, x1, y0, y1, y, k, delta;

        if ( this->m_startPoint.x() < point.x()) {
            x0 = this->m_startPoint.x();
            x1 = point.x();
        } else {
            x1 = this->m_startPoint.x();
            x0 = point.x();
        }

        if ( this->m_startPoint.y() < point.y()) {
            y0 = this->m_startPoint.y();
            y1 = point.y();
        } else {
            y1 = this->m_startPoint.y();
            y0 = point.y();
        }

        rect.setCoords( x0, y0, x1, y1 );
        this->m_selectionRect = rect;

        delta = this->m_scrollPosition - this->verticalOffset();
        if ( this->m_scrollPosition > this->verticalOffset())
            delta *= -1;

        rect.setY( rect.y() + delta );

        for ( y = 0; y < this->model()->rowCount(); y++ ) {
            for ( k = 0; k < this->model()->columnCount(); k++ ) {
                QModelIndex index;

                index = this->model()->index( y, k );
                if ( rect.intersects( this->visualRect( index ))) {
                    this->selectionModel()->select( index, QItemSelectionModel::Select | QItemSelectionModel::Rows );
                    break;
                } else
                    this->selectionModel()->select( index, QItemSelectionModel::Deselect );
            }
        }
    }

    // handle hover selections
    this->model()->processMouseMove( this->indexAt( e->pos()));

    this->viewport()->update();
    QTableView::mouseMoveEvent( e );
}

/**
 * @brief TableView::mousePressEvent
 */
void TableView::mousePressEvent( QMouseEvent *e ) {
    QPoint point;

    point = this->viewport()->mapFromGlobal( QCursor::pos());

    if ( e->button() == Qt::LeftButton && this->indexAt( point ).row() == -1 ) {
        this->selectionModel()->reset();
        this->viewport()->update();
        this->m_selection = true;
        this->m_startPoint = point;
        this->m_selectionRect = QRect();
        this->m_scrollPosition = this->verticalOffset();
    } else {
        this->m_selection = false;
    }

    QTableView::mousePressEvent( e );
}

/**
 * @brief TableView::selectionChanged
 * @param selected
 * @param deselected
 */
void TableView::selectionChanged( const QItemSelection &selected, const QItemSelection &deselected ) {
    this->model()->setSelection( this->selectionModel()->selectedIndexes());
    QTableView::selectionChanged( selected, deselected );
}

/**
 * @brief TableView::paintEvent
 * @param e
 */
void TableView::paintEvent( QPaintEvent *e ) {
    QTableView::paintEvent( e );

    if ( this->m_selection ) {
        int delta;
        QPainter painter( this->viewport());
        QRect rect( this->m_selectionRect );

        painter.setPen( QPen( QColor::fromRgbF( 0.1647, 0.4980, 0.8314, 1.00 )));
        painter.setBrush( QBrush( QColor::fromRgbF( 0.1647, 0.4980, 0.8314, 0.25 )));

        delta = this->m_scrollPosition - this->verticalOffset();
        if ( this->m_scrollPosition > this->verticalOffset())
            delta *= -1;

        rect.setY( rect.y() + delta );
        painter.drawRect( rect );
    }
}

/**
 * @brief TableView::dropEvent
 * @param e
 */
void TableView::dropEvent( QDropEvent *e ) {
    this->model()->processDropEvent( this->indexAt( e->pos()), this->mapToGlobal( e->pos()) );
    e->accept();
}

