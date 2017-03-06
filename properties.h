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

#ifndef PROPERTIES_H
#define PROPERTIES_H

//
// includes
//
#include <QDialog>

//
// namespace: Ui
//
namespace Ui {
class Properties;
}

//
// classes
//
class Entry;

class Properties : public QDialog {
    Q_OBJECT

public:
    explicit Properties( QWidget *parent = 0 );
    ~Properties();

public slots:
    void setEntry( Entry *entry );
    void setEntries( QList<Entry *> entries );

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void setDeviceUsage( const QString &path );

private:
    Ui::Properties *ui;
    Entry *entry;
};

#endif // PROPERTIES_H
