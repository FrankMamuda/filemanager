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
#include "history.h"

/**
 * @brief History::addItem
 * @param item
 */
void History::addItem( const QVariant &item ) {
    int trim;

    // avoid duplicates
    if (( this->current() == item ) && ( this->count() > 0 ))
         return;

    if ( this->mode() == Trim ) {
        // trim list
        trim = this->count() - this->position() - 1;
        while ( trim ) {
            this->m_history.takeLast();
            trim--;
        }
    }

    // add a new entry
    this->m_history.append( item );
    this->setPosition( this->count() - 1 );

    // announce change
    emit this->changed();
}

/**
 * @brief History::itemAt
 * @param index
 * @return
 */
QVariant History::itemAt( int index ) const {
    if ( this->position() == -1 || this->isEmpty() || this->position() >= this->history().count())
        return QVariant();

    return this->history().at( index );
}

/**
 * @brief History::isBackEnabled
 * @return
 */
bool History::isBackEnabled() const {
    if ( this->position() <= 0 )
        return false;

    return true;
}

/**
 * @brief History::isForwardEnabled
 * @return
 */
bool History::isForwardEnabled() const {
    if (( this->position() < 0 ) || ( this->position() == this->count() - 1 ))
        return false;

    return true;
}

/**
 * @brief History::clear
 */
void History::clear() {
    this->m_history.clear();
    this->m_position = 0;
    emit this->reset();
}
