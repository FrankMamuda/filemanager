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

/**
 * @brief TableView::TableView
 * @param parent
 */
TableView::TableView( QWidget *parent ) : QTableView( parent ), m_model( new ContainerModel( this, ContainerModel::TableContainer, ContainerModel::FileMode, 16 ))/*, m_selection( false )*/ {
    this->setModel( this->m_model );
    this->connect( this, SIGNAL( clicked( QModelIndex )), this->model(), SLOT( processItemOpen( QModelIndex )));
    this->connect( this->horizontalHeader(), SIGNAL( geometriesChanged()), this, SLOT( headerResized()));

    for ( int y = 0; y < this->model()->columnCount(); y++ ) {
        Variable *var = Variable::add( QString( "tableView/horizontalHeader/sectionSize_%1" ).arg( y ), m.settings, 100 );
        if ( var != NULL )
            this->horizontalHeader()->resizeSection( y, var->integer());
    }

    // enable mouse tracking
    this->setMouseTracking( true );

    // update selection rectangle on scroll bar changes
    this->connect( this->verticalScrollBar(), SIGNAL( valueChanged( int )), this, SLOT( updateRubberBand()));
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
 * @brief TableView::mousePressEvent
 */
void TableView::mousePressEvent( QMouseEvent *e ) {
    this->model()->processMousePress( e );
    QTableView::mousePressEvent( e );
}

/**
 * @brief TableView::mouseReleaseEvent
 * @param e
 */
void TableView::mouseReleaseEvent( QMouseEvent *e ) {    
    this->model()->processMouseRelease( e );
    QTableView::mouseReleaseEvent( e );
}

/**
 * @brief TableView::mouseMoveEvent
 */
void TableView::mouseMoveEvent( QMouseEvent *e ) {
    this->model()->processMouseMove( e );
    QTableView::mouseMoveEvent( e );
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
 * @brief TableView::dropEvent
 * @param e
 */
void TableView::dropEvent( QDropEvent *e ) {
    this->model()->processDropEvent( this->indexAt( e->pos()), this->mapToGlobal( e->pos()) );
    e->accept();
}

/**
 * @brief TableView::updateRubberBand
 */
void TableView::updateRubberBand() {
    this->model()->setVerticalOffset( this->verticalOffset());
    this->model()->updateRubberBand();
}
