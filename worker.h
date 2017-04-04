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

#ifndef WORKER_H
#define WORKER_H

//
// includes
//
#include <QThread>
#include <QMutex>
#include <QDebug>
#include <QMimeType>
#include "cache.h"

/**
 * @brief The Worker class
 */
class Worker : public QThread {
    Q_OBJECT

public:

public slots:
    void addWork( const Work &work ) { this->workList << work; }
    void addWork( QList<Work> list ) { this->workList << list; }
    void clear() { this->workList.clear(); }

signals:
    void workDone( const Work & );

private:
    void run();
    DataEntry work( const QString &fileName );
    QList<Work> workList;
    static QPixmap generateThumbnail( const QString &path, int scale, bool &ok );
    static QPixmap extractPixmap( const QString &path , bool jumbo );
    static QPixmap scalePixmap( const QPixmap &pixmap, float scale );
};

#endif // WORKER_H
