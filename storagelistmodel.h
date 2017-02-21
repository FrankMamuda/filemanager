#ifndef STORAGELISTMODEL_H
#define STORAGELISTMODEL_H

//
// includes
//
#include <QAbstractListModel>
#include "mainwindow.h"

//
// classes
//
class MainWindow;
class StorageListModel;

/**
 * @brief The StorageListModel class
 */
class StorageListModel : public QAbstractListModel {
public:
    explicit StorageListModel( MainWindow *gui ) : m_iconSize( 32 ) { this->gui = gui; }
    int rowCount( const QModelIndex &parent ) const;
    QVariant data( const QModelIndex &index, int role ) const;
    void reset( int iconSize = 32 );
    int iconSize() { return this->m_iconSize; }

private:
    MainWindow *gui;
    int m_iconSize;
};

#endif // STORAGELISTMODEL_H
