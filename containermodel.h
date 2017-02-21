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

#ifndef CONTAINERMODEL_H
#define CONTAINERMODEL_H

//
// includes
//
#include <QAbstractTableModel>
#include <QFutureWatcher>
#include <QAbstractItemView>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QTimer>

//
// classes
//
class ListView;
class Entry;

/**
 * @brief The ListViewDelegate class
 */
class ListViewDelegate : public QStyledItemDelegate {
public:
    ListViewDelegate( ListView *parent );
    ~ListViewDelegate() {}
    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
    QSize sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const;
};

/**
 * @brief The ContainerItem class
 */
class ContainerItem {
public:
    QStringList lines;
    QList<int> lineWidths;
    int textHeight;
};
Q_DECLARE_METATYPE( ContainerItem )

/**
 * @brief The ContainerModel class
 */
class ContainerModel : public QAbstractTableModel {
    Q_OBJECT

public:
    enum Modes {
        NoMode = -1,
        FileMode,
        SideMode
    };

    enum Containers {
        NoConatainer = -1,
        ListContainer,
        TableContainer
    };

    enum Data {
        DisplayItem = 0
    };

    enum Sections {
        SectionName = 0,
        SectionDate,
        SectionMimetype,
        SectionSize
    };

    // constructor
    explicit ContainerModel( QAbstractItemView *parent, Containers container = ListContainer, Modes mode = FileMode, int iconSize = 64 );
    ~ContainerModel() {}

    // overrides
    int rowCount( const QModelIndex & = QModelIndex()) const { return this->numItems(); }
    int columnCount( const QModelIndex & = QModelIndex()) const;
    void reset();
    int iconSize() const { return this->m_iconSize; }
    QVariant data( const QModelIndex &index, int role ) const;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    Qt::DropActions supportedDropActions() const { if ( this->mode() == FileMode ) return Qt::MoveAction; return Qt::IgnoreAction; }
    Qt::DropActions supportedDragActions() const { if ( this->mode() == FileMode ) return Qt::MoveAction; return Qt::IgnoreAction; }
    Qt::ItemFlags flags( const QModelIndex &index ) const;

    // custom functions
    Containers container() const { return this->m_container; }
    Modes mode() const { return this->m_mode; }
    Entry *indexToEntry( const QModelIndex &index ) const;
    int numItems() const { return this->list.count(); }
    QAbstractItemView *listParent() const { return this->m_listParent; }

signals:
    void stop();

public slots:
    void setIconSize( int iconSize = 64 );
    void setMode( Modes mode = FileMode );
    void buildList( const QString &path = QString::null );
    void mimeTypeDetected( int index );
    void processDropEvent( const QModelIndex &index, const QPoint &pos );
    void processContextMenu( const QModelIndex &index, const QPoint &pos );
    void processItemOpen( const QModelIndex &index );
    void setSelection( const QModelIndexList &selection ) { this->selectionTimer.stop(); this->selection = selection; }
    void processEntries();
    void processMouseMove( const QModelIndex &index );

private slots:
    void determineMimeTypes();
    void displayProperties();
    void selectCurrent();
    void deselectCurrent();

private:
    QModelIndexList selection;
    QAbstractItemView *m_listParent;
    Containers m_container;
    Modes m_mode;
    int m_iconSize;
    QList<Entry*> list;
    QList<ContainerItem>displayList;
    static Entry *determineMimeTypeAsync( Entry *entry );
    QFuture<Entry*> future;
    QFutureWatcher<Entry*> futureWatcher;
    QModelIndex currentIndex;
    QTimer selectionTimer;
};

#endif // CONTAINERMODEL_H
