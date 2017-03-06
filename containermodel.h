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
#include <QPainter>
#include <QTimer>
#include <QMouseEvent>
#include <QRubberBand>
#include <QPoint>
#include <QFileInfo>
#include <QMimeType>

//
// classes
//
class ListView;
class Entry;

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
 * @brief The ASyncWorker class thread safe mime type detection implementation
 */
class ASyncWorker {
public:
    ASyncWorker( const QFileInfo &fileInfo, int index, int iconSize )
        : m_info( fileInfo ), m_index( index ), m_iconSize( iconSize ), m_update( false ) {}

    QFileInfo info() const { return this->m_info; }
    int index() const { return this->m_index; }
    QMimeType mimeType() const { return this->m_mimeType; }
    QString iconName() const { return this->m_iconName; }
    int iconSize() const { return this->m_iconSize; }
    bool update() const { return this->m_update; }

    void setMimeType( const QMimeType &mimeType ) { this->m_mimeType = mimeType; }
    void setIconName( const QString &iconName ) { this->m_iconName = iconName; }
    void scheduleUpdate() { this->m_update = true; }

private:
    QFileInfo m_info;
    int m_index;
    QMimeType m_mimeType;
    QString m_iconName;
    int m_iconSize;
    bool m_update;
};

/**
 * @brief The ContainerModel class
 */
class ContainerModel : public QAbstractTableModel {
    Q_OBJECT
    Q_PROPERTY( int iconSize READ iconSize WRITE setIconSize )
    Q_PROPERTY( Containers container READ container )
    Q_PROPERTY( Modes mode READ mode WRITE setMode )
    Q_PROPERTY( int verticalOffset READ verticalOffset WRITE setVerticalOffset )

public:
    enum Modes {
        NoMode = -1,
        FileMode,
        SideMode
    };
    Q_ENUMS( Modes )

    enum Containers {
        NoConatainer = -1,
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
    explicit ContainerModel( QAbstractItemView *parent, Containers container = ListContainer, Modes mode = FileMode, int iconSize = 64 );
    ~ContainerModel();

    // overrides
    int rowCount( const QModelIndex & = QModelIndex()) const { return this->numItems(); }
    int columnCount( const QModelIndex & = QModelIndex()) const;
    void reset( bool force = false );    
    QVariant data( const QModelIndex &index, int role ) const;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    Qt::DropActions supportedDropActions() const { if ( this->mode() == FileMode ) return Qt::CopyAction; return Qt::IgnoreAction; }
    Qt::DropActions supportedDragActions() const { if ( this->mode() == FileMode ) return Qt::CopyAction; return Qt::IgnoreAction; }
    Qt::ItemFlags flags( const QModelIndex &index ) const;
    QMimeData *mimeData( const QModelIndexList &indexes ) const;

    // properties
    int iconSize() const { return this->m_iconSize; }
    Containers container() const { return this->m_container; }
    Modes mode() const { return this->m_mode; }
    int verticalOffset() const { return this->m_verticalOffset; }

    // custom functions
    Entry *indexToEntry( const QModelIndex &index ) const;
    int numItems() const { return this->list.count(); }
    QAbstractItemView *listParent() const { return this->m_listParent; }
    QRubberBand *rubberBand() const { return this->m_rubberBand; }

signals:
    void stop();

public slots:
    // properties
    void setIconSize( int iconSize = 64 );
    void setMode( Modes mode = FileMode );
    void setVerticalOffset( int offset ) { this->m_verticalOffset = offset; }

    // mime type detection related
    void mimeTypeDetected( int index );

    // custom slots
    void buildList( const QString &path = QString::null );
    void setSelection( const QModelIndexList &selection ) { this->selectionTimer.stop(); this->selection = selection; }
    void processEntries();
    void updateRubberBand();

    // conatiner event handlers
    void processDropEvent( const QModelIndex &index, const QPoint &pos );
    void processContextMenu( const QModelIndex &index, const QPoint &pos );
    void processItemOpen( const QModelIndex &index );
    void processMousePress( QMouseEvent *e );
    void processMouseRelease( QMouseEvent *e );
    void processMouseMove( QMouseEvent *e );

private slots:
    // mime type detection related
    void determineMimeTypes();

    // other slots
    void displayProperties();
    void selectCurrent();
    void deselectCurrent();
    void quit();

private:
    QModelIndexList selection;
    QAbstractItemView *m_listParent;
    QList<Entry*> list;
    QModelIndex currentIndex;
    QTimer selectionTimer;
    QRubberBand *m_rubberBand;
    QPoint selectionOrigin;
    QPoint currentMousePos;

    // mime type detection related
    QList<ASyncWorker*> workList;
    QList<ContainerItem>displayList;
    static ASyncWorker *determineMimeTypeAsync( ASyncWorker *worker );
    QFuture<ASyncWorker*> future;
    QFutureWatcher<ASyncWorker*> futureWatcher;

    // properties
    Containers m_container;
    Modes m_mode;
    int m_iconSize;
    int m_verticalOffset;
};

Q_DECLARE_METATYPE( ContainerModel::Modes )
Q_DECLARE_METATYPE( ContainerModel::Containers )
Q_DECLARE_METATYPE( ContainerModel::Data )
Q_DECLARE_METATYPE( ContainerModel::Sections )

#endif // CONTAINERMODEL_H
