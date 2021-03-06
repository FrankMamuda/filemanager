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
#include <QAbstractTableModel>
#include <QFutureWatcher>
#include <QAbstractItemView>
#include <QPainter>
#include <QTimer>
#include <QMouseEvent>
#include <QRubberBand>
#include <QPoint>
#include <QFileInfo>
#include <QMimeType>
#include <QItemSelectionModel>
#include <QMultiHash>
#include "common.h"
#include "cache.h"

//
// classes
//
class ListView;
class TableView;
class Entry;

/**
 * @brief The SpecialDirectory struct
 */
struct SpecialDirectory {
    enum Types {
        NoType = -1,
        General,
#ifdef Q_OS_WIN32
        Root,
#endif
        Trash,
        Bookmarks
    };
    SpecialDirectory( Types t, const QString &p ) : type( t ), path( p ) {}
    Types type;
    QString path;
    static Types pathToType( const QString &path );
};

/**
 * @brief The ContainerNamespace class
 */
namespace ContainerNamespace {
static const QList<SpecialDirectory> SpecialDirectories =
        ( QList<SpecialDirectory>() <<
#ifdef Q_OS_WIN32
          SpecialDirectory( SpecialDirectory::Root, "/" ) <<
#endif
          SpecialDirectory( SpecialDirectory::Trash, "trash://" ) <<
          SpecialDirectory( SpecialDirectory::Bookmarks, "bookmarks://" ));
}

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
    Q_PROPERTY( int iconSize READ iconSize WRITE setIconSize )
    Q_PROPERTY( Containers container READ container )
    Q_PROPERTY( int verticalOffset READ verticalOffset WRITE setVerticalOffset )
    Q_PROPERTY( bool selectionLocked READ selectionLocked )

public:
    enum Containers {
        NoContainer = -1,
        ListContainer,
        TableContainer
    };
    Q_ENUMS( Containers )

    enum Data {
        DisplayItem = 0
    };
    Q_ENUMS( Data )

    enum Sections {
        SectionName = 0,
        SectionDate,
        SectionMimetype,
        SectionSize
    };
    Q_ENUMS( Sections )

    // constructor
    explicit ContainerModel( QAbstractItemView *view, Containers container = ListContainer );
    ~ContainerModel();

    // overrides
    int rowCount( const QModelIndex & = QModelIndex()) const { return this->numItems(); }
    int columnCount( const QModelIndex & = QModelIndex()) const;
    void reset( bool force = false );
    void softReset();
    QVariant data( const QModelIndex &index, int role ) const;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    Qt::DropActions supportedDropActions() const;
    Qt::DropActions supportedDragActions() const;
    Qt::ItemFlags flags( const QModelIndex &index ) const;
    QMimeData *mimeData( const QModelIndexList &indexes ) const;

    // properties
    int iconSize() const;
    Containers container() const { return this->m_container; }
    int verticalOffset() const { return this->m_verticalOffset; }
    bool selectionLocked() const { return this->m_selectionLocked; }

    // custom functions
    Entry *indexToEntry( const QModelIndex &index ) const;
    int numItems() const { return this->list.count(); }
    QAbstractItemView *parent() const { return this->m_parent; }
    QRubberBand *rubberBand() const { return this->m_rubberBand; }
    QItemSelectionModel *selectionModel() { return this->parent()->selectionModel(); }

    // TODO: make private?
    QList<Entry*> selectionList;

signals:
    void stop();

public slots:
    // properties
    void setIconSize( int iconSize = Common::DefaultListIconSize );
    void setVerticalOffset( int offset ) { this->m_verticalOffset = offset; }

    // custom slots
    void buildList( const QString &path = QString::null );
    void setSelection( const QModelIndexList &selection );
    void processEntries();
    void updateRubberBand();
    void determineMimeTypes();
    void populate();

    // conatiner event handlers
    void processDropEvent( const QModelIndex &index, const QPoint &pos );
    void processContextMenu( const QModelIndex &index, const QPoint &pos );
    void processItemOpen( const QModelIndex &index );
    void processMousePress( QMouseEvent *e );
    void processMouseRelease( QMouseEvent *e );
    void processMouseMove( QMouseEvent *e );

private slots:
    // other slots
    void displayProperties();
    void copy();
    void paste();
    void openWith();
    void cut();
    void open();
    void rename();
    void recycle();
    void selectCurrent();
    void deselectCurrent();
    void restoreSelection();
    void mimeTypeDetected( const QString &fileName, const DataEntry &entry );

private:
    QModelIndexList selection;
    QAbstractItemView *m_parent;
    QList<Entry*> list;
    QModelIndex currentIndex;
    QTimer selectionTimer;
    QRubberBand *m_rubberBand;
    QPoint selectionOrigin;
    QPoint currentMousePos;

    // properties
    int m_iconSize;
    int m_verticalOffset;
    bool m_selectionLocked;
    Containers m_container;
    QList<ContainerItem>displayList;

    QMultiHash<QString, QModelIndex> fileHash;
};

Q_DECLARE_METATYPE( ContainerModel::Containers )
Q_DECLARE_METATYPE( ContainerModel::Data )
Q_DECLARE_METATYPE( ContainerModel::Sections )
