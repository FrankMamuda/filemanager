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

#ifndef ICONMODEL_H
#define ICONMODEL_H

//
// includes
//
#include <QAbstractListModel>
#include <QListView>
#include <QHash>

/**
 * @brief The IconModel class
 */
class IconModel : public QAbstractListModel {
    Q_OBJECT

public:
    explicit IconModel( QObject *listView = 0 );
    ~IconModel();
    int rowCount( const QModelIndex & = QModelIndex()) const override { return this->pixmapList.count(); }
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    QString nameForIndex( const QModelIndex &index ) const { if ( index.isValid()) return this->iconNames.at( index.row()); return QString::null; }
    QPixmap pixmapForIndex( const QModelIndex &index ) const { if ( index.isValid()) return this->pixmapList.at( index.row()); return QPixmap(); }
    void addIcon( const QString &iconName, const QPixmap &pixmap );

public slots:
    void clear();
    void reset();
    void fetch();

private slots:
    void iconFetched( const QString &iconName, quint8 iconScale, const QPixmap & );

private:
    QListView *parent;
    QStringList iconNames;
    QList<QPixmap>pixmapList;
};

#endif // ICONMODEL_H
