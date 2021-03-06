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
#include <QTableView>
#include <QDropEvent>
#include <QDragLeaveEvent>
#include <QPaintEvent>
#include <QWheelEvent>
#include <QScrollArea>
#include <QResizeEvent>

//
// classes
//
class ContainerModel;
class ContainerStyle;

/**
 * @brief The TableView class
 */
class TableView : public QTableView {
    Q_OBJECT

public:
    TableView( QWidget * );
    ~TableView();
    ContainerModel *model() const { return this->m_model; }

public slots:
    void setModel( ContainerModel * );

private slots:
    void headerResized();
    void updateRubberBand();

protected:
    // overrides
    void dropEvent( QDropEvent *e );
    void dragLeaveEvent( QDragLeaveEvent *e ) { e->accept(); }
    void mousePressEvent( QMouseEvent *e );
    void mouseReleaseEvent( QMouseEvent *e );
    void mouseMoveEvent( QMouseEvent *e );
    void selectionChanged( const QItemSelection &, const QItemSelection & );
    void resizeEvent( QResizeEvent *e );

private:
    ContainerModel *m_model;
    ContainerStyle *m_style;
};
