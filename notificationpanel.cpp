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
#include "notificationpanel.h"
#include "ui_notificationpanel.h"
#include <QGraphicsOpacityEffect>
#include <QPainter>
#include <QApplication>
#include <QPalette>
#include <QTimer>
#include <QIcon>
#include <QPixmap>
#include <QDebug>

/**
 * @brief NotificationPanel::NotificationPanel
 * @param parent
 */
NotificationPanel::NotificationPanel( QWidget *parent ) : QWidget( parent ), ui( new Ui::NotificationPanel ) {
    this->ui->setupUi( this );
    this->setWindowFlags( Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint );
    this->opacityEffect = new QGraphicsOpacityEffect( this );
    this->opacityEffect->setOpacity( 0.75f );
    this->setGraphicsEffect( this->opacityEffect );
    this->setAutoFillBackground( true );

    this->ui->closeButton->setIcon( QIcon::fromTheme( "dialog-ok" ));
    this->ui->nextButton->setIcon( QIcon::fromTheme( "go-next" ));
    this->ui->prevButton->setIcon( QIcon::fromTheme( "go-previous" ));

    this->ui->messageLabel->setText( "has\nfailed\nmiserably\nyet\nagain" );

    this->timer.setSingleShot( true );
    this->timer.connect( &this->timer, SIGNAL( timeout()), this, SLOT( close()));
}

/**
 * @brief NotificationPanel::~NotificationPanel
 */
NotificationPanel::~NotificationPanel() {
    delete ui;
    this->opacityEffect->deleteLater();
}

/**
 * @brief NotificationPanel::setPixmap
 * @param pixmap
 */
void NotificationPanel::setPixmap( const QPixmap &pixmap ) {
    this->ui->pixmapLabel->setPixmap( pixmap );
}

/**
 * @brief NotificationPanel::paintEvent
 * @param event
 */
void NotificationPanel::paintEvent( QPaintEvent *event ) {
    QPainter painter( this );

    QRect rect;

    QPen pen;
    QColor color = qApp->palette().color( QPalette::Background );
    color = color.darker( 200 );
    pen.setColor( color );
    painter.save();
    painter.setPen( pen );

    rect = this->rect();
    rect.setWidth( rect.width() - 1 );
    rect.setHeight( rect.height() - 1 );
    painter.drawRect( rect );
    painter.restore();

    QWidget::paintEvent( event );
}

/**
 * @brief NotificationPanel::showNotifications
 * @param timeOut
 */
void NotificationPanel::showNotifications( int timeOut ) {
    if ( timeOut != -1 ) {
        this->timer.setInterval( timeOut );
        this->timer.start();
    }

    this->show();
}

/**
 * @brief NotificationPanel::pushNotification
 * @param msg
 * @param timeOut
 */
void NotificationPanel::pushNotification( Types type, const QString &title, const QString &msg, int timeOut ) {
    this->timer.stop();

    switch ( type ) {
    case Information:
        this->ui->pixmapLabel->setPixmap( QIcon::fromTheme( "dialog-information" ).pixmap( 16, 16 ));
        break;

    case Warning:
        this->ui->pixmapLabel->setPixmap( QIcon::fromTheme( "dialog-warning" ).pixmap( 16, 16 ));
        break;

    case Error:
        this->ui->pixmapLabel->setPixmap( QIcon::fromTheme( "dialog-error" ).pixmap( 16, 16 ));
        break;
    }

    this->ui->titleLabel->setText( title );
    this->ui->messageLabel->setText( msg );
    this->showNotifications( timeOut );
}

/**
 * @brief NotificationPanel::on_closeButton_clicked
 */
#include <QPropertyAnimation>
void NotificationPanel::on_closeButton_clicked() {

    qDebug() << "close";

    QPropertyAnimation animation( this, "geometry" );
    animation.setDuration( 3000 );
    animation.setStartValue( this->geometry());
    QRect end;
    end = this->geometry();
    end.setX( end.x() + end.width() + 11 );
    animation.setEndValue( end );

    animation.start();
}
