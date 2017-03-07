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
#include <QPropertyAnimation>

/**
 *
 */
namespace Ui {
class NotificationPanel;
}

// classes
class History;

/**
 * @brief The NotificationPanel class
 */
class NotificationPanel : public QWidget {
    Q_OBJECT
    Q_PROPERTY( float opacity READ opacity WRITE setOpacity )

public:
    // constructor/destructor
    explicit NotificationPanel( QWidget *parent = 0 );
    ~NotificationPanel();

    // other members
    void setPixmap( const QPixmap &pixmap );

    enum Types {
        Information = 0,
        Warning,
        Error,
        Progress
    };
    Q_ENUMS( Types )

    History *historyManager() const { return this->m_historyManager; }
    QPropertyAnimation *animation() const { return this->m_animation; }

    // properties
    float opacity() const { return this->m_opacity; }

protected:
    // overrides
    void paintEvent( QPaintEvent *event );

public slots:
    void raise( int timeOut = -1 );
    void push( Types type, const QString &title, const QString &msg, int timeOut = 5000 , bool fromHistory = false );
    void setOpacity( float opacity ) { this->m_opacity = opacity; this->opacityEffect->setOpacity( opacity ); }

private slots:
    void on_closeButton_clicked();
    void on_prevButton_clicked();
    void on_nextButton_clicked();
    void checkHistoryPosition();

private:
    Ui::NotificationPanel *ui;
    QGraphicsOpacityEffect *opacityEffect;
    QTimer timer;
    QPropertyAnimation *m_animation;

    // TODO: separate info/warning/error messages
    History *m_historyManager;

    // properties
    float m_opacity;
};
Q_DECLARE_METATYPE( NotificationPanel::Types )

/**
 * @brief The Notification class
 */
class Notification {
public:
    Notification( NotificationPanel::Types type = NotificationPanel::Information,
                  const QString &title = QString::null, const QString &message = QString::null,
                  int timeout = 5000 ) :
        m_type( type ), m_title( title ), m_message( message ), m_timeout( timeout ) {}

    // properties
    NotificationPanel::Types type() const { return this->m_type; }
    QString title() const { return this->m_title; }
    QString message() const { return this->m_message; }
    int timeout() const { return this->m_timeout; }

private:
    NotificationPanel::Types m_type;
    QString m_title;
    QString m_message;
    int m_timeout;
};
Q_DECLARE_METATYPE( Notification )

#endif // NOTIFICATIONPANEL_H
