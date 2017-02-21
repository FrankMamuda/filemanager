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
#include "sideview.h"
#include "containermodel.h"

/**
 * @brief SideView::SideView
 * @param parent
 */
SideView::SideView( QWidget* parent ) : QListView( parent ), m_model( new ContainerModel( this, ContainerModel::ListContainer, ContainerModel::SideMode, 32 )) {
    this->setModel( this->m_model );
    this->connect( this, SIGNAL( clicked( QModelIndex )), this->model(), SLOT( processItemOpen( QModelIndex )));
    this->setStyleSheet( "background-color: transparent;" );
    this->model()->buildList();
}

/**
 * @brief SideView::~SideView
 */
SideView::~SideView() {
    this->m_model->deleteLater();
}

/**
 * @brief SideView::setModel
 * @param model
 */
void SideView::setModel( ContainerModel *model ) {
    this->m_model = model;
    QListView::setModel( model );
}