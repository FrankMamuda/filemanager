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

#ifndef MAIN_H
#define MAIN_H

//
// includes
//
#include <QList>
#include <QSettings>

//
// classes
//
class MainWindow;
class Variable;
class Bookmark;
class NotificationPanel;

/**
 * @brief The Main class
 */
class Main {
public:
    Main();
    ~Main();
    MainWindow *gui() const { return this->m_gui; }
    NotificationPanel *notifications() { return this->m_notifications; }
    void setGui( MainWindow *gui ) { this->m_gui = gui; }
    void setNotifications( NotificationPanel *notify ) { this->m_notifications = notify; }
    QList <Variable*>varList;
    QSettings *settings;

private:
    MainWindow *m_gui;
    NotificationPanel *m_notifications;
};

extern class Main m;

#endif // MAIN_H
