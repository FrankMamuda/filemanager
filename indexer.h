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

#ifndef INDEXER_H
#define INDEXER_H

//
// includes
//
#include <QThread>
#include <QMutex>
#include <QDebug>
#include "cache.h"

/**
 * @brief The Indexer class
 */
class Indexer : public QThread {
    Q_OBJECT

public:
    Indexer() {}

public slots:
    void addWork( const QString &fileName ) { this->workList << fileName; }
    void addWork( const QStringList &fileList ) { this->workList << fileList; }
    void clear() { this->workList.clear(); }

signals:
    void workDone( const QString &fileName, const Hash & );

private:
    void run();
    Hash work( const QString &fileName );
    QStringList workList;
};

#endif // INDEXER_H
