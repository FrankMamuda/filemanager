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

//
// includes
//
#include "textutils.h"
#include <QObject>

/**
 * @brief Properties::sizeToText
 * @param bytes
 * @return
 */
QString TextUtils::sizeToText( const quint64 bytes ) {
    const quint64 kb = 1024;
    const quint64 mb = 1024 * kb;
    const quint64 gb = 1024 * mb;
    const quint64 tb = 1024 * gb;

    if ( bytes >= tb )
        return QObject::tr( "%1 TB" ).arg( QString::number( qreal( bytes ) / tb, 'f', 3 ));

    if ( bytes >= gb )
        return QObject::tr( "%1 GB" ).arg( QString::number( qreal( bytes ) / gb, 'f', 2 ));

    if ( bytes >= mb )
        return QObject::tr( "%1 MB" ).arg( QString::number( qreal( bytes ) / mb, 'f', 1 ));

    if ( bytes >= kb )
        return QObject::tr( "%1 KB" ).arg( QString::number( bytes / kb ));

    return QObject::tr( "%1 byte(s)" ).arg( QString::number( bytes ));
}
