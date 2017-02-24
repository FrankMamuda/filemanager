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

#ifndef VARIABLE_H
#define VARIABLE_H

//
// includes
//
#include <QString>
#include <QSettings>
#include "main.h"

/**
 * @brief The Variable class
 */
class Variable {
public:
    template<typename T>
    static T value( const QString &key, bool defaultValue = false ) { if ( !m.settings->contains( key ) && m.settings->contains( key + "/default" )) defaultValue = true; return qvariant_cast<T>( m.settings->value( defaultValue ? key + "/default" : key )); }
    template<typename T>
    static T defaultValue( const QString &key ) { return Variable::value<T>( key, true ); }
    static int integer( const QString &key, bool defaultValue = false ) { return Variable::value<int>( key, defaultValue ); }
    static bool isEnabled( const QString &key, bool defaultValue = false ) { return Variable::value<bool>( key, defaultValue ); }
    static bool isDisabled( const QString &key, bool defaultValue = false ) { return !Variable::isEnabled( key, defaultValue ); }
    static QString string( const QString &key, bool defaultValue = false ) { return Variable::value<QString>( key, defaultValue ); }
    template<typename T>
    static void setValue( const QString &key, const T &value ) { if ( !m.settings->contains( key )) m.settings->setValue( key + "/default", value ); m.settings->setValue( key, value ); }
    static void setString( const QString &key, const QString &string ) { Variable::setValue<QString>( key, string ); }
    static void setInteger( const QString &key, int value ) { Variable::setValue<int>( key, value ); }
    static void enable( const QString &key ) { Variable::setValue<bool>( key, true ); }
    static void disable( const QString &key ) { Variable::setValue<bool>( key, false ); }
    template<typename T>
    static void setDefaultValue( const QString &key, const T &value ) { m.settings->setValue( key + "/default", value ); }
    template<typename T>
    static void add( const QString &key, const T &value ) { Variable::setDefaultValue<T>( key, value ); }
    static void reset( const QString &key ) { if ( m.settings->contains( key )) m.settings->setValue( key, m.settings->value( key + "/default" )); }
};

#endif // VARIABLE_H
