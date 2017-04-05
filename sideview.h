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

#ifndef STORAGEVIEW_H
#define STORAGEVIEW_H

//
// includes
//
#include <QListView>
#include <QDropEvent>
#include <QDragLeaveEvent>
#include <QMouseEvent>
#include <QDropEvent>

//
// classes
//
class ListViewDelegate;
class BookmarkModel;
class ContainerStyle;

/**
 * @brief The DeviceView class
 */
class SideView : public QListView {
    Q_OBJECT

public:
    SideView( QWidget * );
    ~SideView();
    BookmarkModel *model() const { return this->m_model; }
    ListViewDelegate *delegate() const { return this->m_delegate; }

public slots:
    void setModel( BookmarkModel * );

private slots:
    void processItemOpen( const QModelIndex &index );
    void processContextMenu( const QModelIndex &index, const QPoint &pos );
    void renameBookmark();
    void changeBookmarkTarget();
    void removeBookmark();
    void changeBookmarkIcon();

protected:
    // overrides
    void mouseReleaseEvent( QMouseEvent * );
    void dropEvent( QDropEvent * );

private:
    BookmarkModel *m_model;
    QModelIndex index;
    ContainerStyle *m_style;
    ListViewDelegate *m_delegate;
};

#endif // STORAGEVIEW_H

