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

#ifndef NOTIFICATIONPANEL_H
#define NOTIFICATIONPANEL_H

//
// includes
//
#include <QWidget>
#include <QGraphicsOpacityEffect>
#include <QPaintEvent>
#include <QTimer>

/**
 *
 */
namespace Ui {
class NotificationPanel;
}

/*
class Notification {
    Types type;
    QString title;
    QString message;
};*/

/**
 * @brief The NotificationPanel class
 */
class NotificationPanel : public QWidget {
    Q_OBJECT

public:
    explicit NotificationPanel( QWidget *parent = 0 );
    ~NotificationPanel();
    void setPixmap( const QPixmap &pixmap );

    enum Types {
        Information = 0,
        Warning,
        Error
    };

protected:
    void paintEvent( QPaintEvent *event );

public slots:
    void showNotifications( int timeOut = -1 );
    void pushNotification( Types type, const QString &title, const QString &msg, int timeOut = 5000 );

private slots:
    void on_closeButton_clicked();

private:
    Ui::NotificationPanel *ui;
    QGraphicsOpacityEffect *opacityEffect;
    QTimer timer;

    // use message stack in future
    //QList<Notification*> messages;
};

#endif // NOTIFICATIONPANEL_H
