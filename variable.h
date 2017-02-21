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
#include <QObject>
#include <QSettings>
#include <QTime>

/**
 * @brief The Variable class
 */
class Variable : public QObject {
    Q_OBJECT
    Q_PROPERTY( QString key READ key WRITE setKey )
    Q_PROPERTY( QVariant defaultValue READ defaultValue WRITE setDefaultValue )
    Q_PROPERTY( QVariant value READ value WRITE setValue )
    Q_PROPERTY( int integer READ integer )
    Q_PROPERTY( bool isEnabled READ isEnabled )
    Q_PROPERTY( float floatValue READ floatValue() )
    Q_PROPERTY( QString string READ string )
    Q_PROPERTY( QTime time READ time )
    Q_PROPERTY( QString timeString READ timeString )
    Q_CLASSINFO( "description", "Variable" )

public:
    explicit Variable( const QString &key, QSettings *settingsPtr, const QVariant &defaultValue ) { this->setKey( key ); this->s = settingsPtr; this->setDefaultValue( defaultValue ); }
    QString key() const { return this->m_key; }
    QVariant defaultValue() const { return this->m_defaultValue; }
    QVariant value() const { if ( this->s == NULL ) return QVariant(); return this->s->value( this->key(), this->defaultValue()); }
    int integer() const { return this->value().toInt(); }
    bool isEnabled() const { return this->value().toBool(); }
    bool isDisabled() const { return !this->isEnabled(); }
    float floatValue() const { return this->value().toFloat(); }
    QString string() const { return this->value().toString(); }
    QTime time() const { return this->value().toTime(); }
    QString timeString() const { return this->value().toTime().toString( "hh:mm" ); }

    // static functions
    static Variable *find( const QString &key );
    static QVariant defaultValue( const QString &key ) { Variable *cvar = Variable::find( key ); if ( cvar == NULL ) return QVariant(); return cvar->defaultValue(); }
    static QVariant value( const QString &key )        { Variable *cvar = Variable::find( key ); if ( cvar == NULL ) return QVariant(); return cvar->value(); }
    static int integer( const QString &key )           { Variable *cvar = Variable::find( key ); if ( cvar == NULL ) return 0; return cvar->integer(); }
    static bool isEnabled( const QString &key )        { Variable *cvar = Variable::find( key ); if ( cvar == NULL ) return false; return cvar->isEnabled(); }
    static bool isDisabled( const QString &key )       { Variable *cvar = Variable::find( key ); if ( cvar == NULL ) return true; return cvar->isDisabled(); }
    static float floatValue( const QString &key )      { Variable *cvar = Variable::find( key ); if ( cvar == NULL ) return 0.0f; return cvar->floatValue(); }
    static QString string( const QString &key )        { Variable *cvar = Variable::find( key ); if ( cvar == NULL ) return QString::null; return cvar->string(); }
    static QTime time( const QString &key )            { Variable *cvar = Variable::find( key ); if ( cvar == NULL ) return QTime(); return cvar->time(); }
    static QString timeString( const QString &key )    { Variable *cvar = Variable::find( key ); if ( cvar == NULL ) return QString::null; return cvar->timeString(); }
    static void setValue( const QString &key, const QVariant &value ) { Variable *cvar = Variable::find( key ); if ( cvar == NULL ) return; cvar->s->setValue( key, value ); emit cvar->changed(); }
    static Variable *add( const QString &key, QSettings *settingsPtr, const QVariant &defaultValue = QVariant());

signals:
    void changed();

public slots:
    void setKey( const QString &key ) { this->m_key = key; }
    void setDefaultValue( const QVariant &value ) { this->m_defaultValue = value; }
    void setValue( const QVariant &value ) { this->s->setValue( this->key(), value ); emit this->changed(); }

private:
    QString m_key;
    QVariant m_defaultValue;
    QSettings *s;
};

#endif // VARIABLE_H
