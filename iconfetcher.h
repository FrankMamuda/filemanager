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

#ifndef ICONFETCHER_H
#define ICONFETCHER_H

//
// includes
//
#include <QThread>
#include <QMutex>
#include <QDebug>
#include <QPixmap>
#include "iconcache.h"

/**
 * @brief The IconFetcher class
 */
class IconFetcher : public QThread {
    Q_OBJECT

public slots:
    void clear() { QMutexLocker( &this->m_mutex ); this->workList.clear(); }
    void addWork( const QString &iconName, quint8 iconScale );

signals:
    void workDone( const QString &, quint8, const QPixmap & );

private:
    void run();
    QList<IconIndex> workList;
    mutable QMutex m_mutex;
};

#endif // ICONFETCHER_H
