/*
 * Copyright (C) 2013-2016 Avotu Briezhaudzetava
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
#include "variable.h"
#include "main.h"

// TODO: delete vars on exit

/**
 * @brief Variable::add
 * @param key
 * @param settingsPtr
 * @param defaultValue
 * @return
 */
Variable *Variable::add( const QString &key, QSettings *settingsPtr, const QVariant &defaultValue ) {
    // avoid duplicates
    if ( Variable::find( key ) != NULL )
        return NULL;

    m.varList << new Variable( key, settingsPtr, defaultValue );
    return m.varList.last();
}

/**
 * @brief find
 * @param key
 * @return
 */
Variable *Variable::find( const QString &key ) {
    foreach ( Variable *varPtr, m.varList ) {
        if ( !QString::compare( varPtr->key(), key ))
            return varPtr;
    }
    return NULL;
}
