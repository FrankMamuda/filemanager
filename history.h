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
#include <QObject>
#include <QVariant>

/**
 * @brief The History class
 */
class History : public QObject {
    Q_OBJECT
    Q_PROPERTY( int position READ position WRITE setPosition )
    Q_PROPERTY( bool empty READ isEmpty )
    Q_PROPERTY( bool backEnabled READ isBackEnabled )
    Q_PROPERTY( bool forwardEnabled READ isForwardEnabled )
    Q_PROPERTY( int count READ count )
    Q_PROPERTY( Modes mode READ mode )

public:
    enum Modes {
        Trim,
        Insert
    };
    Q_ENUMS( Modes )

    // constructor/destructor
    History( Modes mode = Trim ) : m_position( -1 ), m_mode( Trim ) { this->m_mode = mode; }
    ~History() {}

    // properties
    int position() const { return this->m_position; }
    bool isEmpty() const { return this->history().isEmpty(); }
    bool isBackEnabled() const;
    bool isForwardEnabled() const;
    int count() const { return this->history().count(); }
    Modes mode() const { return this->m_mode; }

    // other functions
    QVariantList history() const { return this->m_history; }
    QVariant current() const { return this->itemAt( this->position()); }
    QVariant itemAt( int index ) const;

signals:
    void reset();
    void changed();

public slots:
    // properties
    void setPosition( int pos ) { this->m_position = pos; }

    // other functions
    void clear();
    void addItem( const QVariant &item );
    void back() { if ( this->m_position > 0 ) { this->m_position--; emit this->changed(); }}
    void forward() { if ( this->m_position < this->count() - 1 ) { this->m_position++; emit this->changed(); }}

private:
    // properties
    int m_position;
    Modes m_mode;

    // other members
    QVariantList m_history;
};

Q_DECLARE_METATYPE( History::Modes )
