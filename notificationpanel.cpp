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
#include "main.h"
#include "mainwindow.h"
#include "history.h"
#include "pixmapcache.h"

//
// includes
//
//#define SLIDE_OUT_ANIMATION

/**
 * @brief NotificationPanel::NotificationPanel
 * @param parent
 */
NotificationPanel::NotificationPanel( QWidget *parent ) : QWidget( parent ), ui( new Ui::NotificationPanel ), m_opacity( 0.75f ) {
    // set up ui
    this->ui->setupUi( this );
    this->setWindowFlags( Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint );

    // set up window opacity
    this->opacityEffect = new QGraphicsOpacityEffect( this );
    this->opacityEffect->setOpacity( this->opacity());
    this->setGraphicsEffect( this->opacityEffect );
    this->setAutoFillBackground( true );

    // set up buttons
    this->ui->closeButton->setIcon( pixmapCache.fromTheme( /*QIcon::fromTheme(*/ "dialog-ok" ));
    this->ui->nextButton->setIcon( pixmapCache.fromTheme( /*QIcon::fromTheme(*/ "go-next" ));
    this->ui->prevButton->setIcon( pixmapCache.fromTheme( /*QIcon::fromTheme(*/ "go-previous" ));

    // set up animation timer
    this->timer.setSingleShot( true );
    this->timer.connect( &this->timer, SIGNAL( timeout()), this, SLOT( on_closeButton_clicked()));

    // set up message history
    this->m_historyManager = new History( History::Insert );
    this->connect( this->historyManager(), SIGNAL( changed()), this, SLOT( checkHistoryPosition()));
    this->checkHistoryPosition();

    // set up animation (either slide or fade out)
#ifdef SLIDE_OUT_ANIMATION
    this->m_animation = new QPropertyAnimation( this, "geometry" );
#else
    this->m_animation = new QPropertyAnimation( this, "opacity" );
    this->connect( this->animation(), SIGNAL( finished()), this, SLOT( hide()));
    this->animation()->setEasingCurve( QEasingCurve::Linear );
#endif
    this->animation()->setDuration( 250 );
}

/**
 * @brief NotificationPanel::~NotificationPanel
 */
NotificationPanel::~NotificationPanel() {
    delete ui;
    this->opacityEffect->deleteLater();

    if ( this->historyManager() != nullptr )
        delete this->m_historyManager;

    if ( this->animation() != nullptr )
        delete this->m_animation;
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
    QColor color;

    // create a color slightly darker than current background
    color = qApp->palette().color( QPalette::Background );
    color = color.darker( 200 );

    // equip painter
    pen.setColor( color );
    painter.save();
    painter.setPen( pen );

    // draw border
    rect = this->rect();
    rect.setWidth( rect.width() - 1 );
    rect.setHeight( rect.height() - 1 );
    painter.drawRect( rect );

    // restore and draw the rest
    painter.restore();
    QWidget::paintEvent( event );
}

/**
 * @brief NotificationPanel::raise
 * @param timeOut
 */
void NotificationPanel::raise( int timeOut ) {
    if ( timeOut != -1 ) {
        this->timer.setInterval( timeOut );
        this->timer.start();
    } else {
        this->timer.stop();
    }

    // abort on no messages
    if ( !this->historyManager()->count())
        return;

    // move back into place
    this->move( m.gui()->geometry().width() - this->geometry().width() - 11,
                m.gui()->geometry().height() - this->geometry().height() - 26 );

    // show if hidden
    if ( this->isHidden())
        this->show();

    // restore opacity
#ifndef SLIDE_OUT_ANIMATION
    this->setOpacity( 0.75f );
#endif
}

/**
 * @brief NotificationPanel::push
 * @param msg
 * @param timeOut
 */
void NotificationPanel::push( Types type, const QString &title, const QString &msg, int timeOut, bool fromHistory ) {
    this->timer.stop();

    // determine message type
    switch ( type ) {
    case Information:
        //this->ui->pixmapLabel->setPixmap( QIcon::fromTheme( "dialog-information" ).pixmap( 16, 16 ));
        this->ui->pixmapLabel->setPixmap( pixmapCache.findPixmap( "dialog-information", 16 ));
        break;

    case Warning:
        //this->ui->pixmapLabel->setPixmap( QIcon::fromTheme( "dialog-warning" ).pixmap( 16, 16 ));
        this->ui->pixmapLabel->setPixmap( pixmapCache.findPixmap( "dialog-warning", 16 ));
        break;

    case Error:
        //this->ui->pixmapLabel->setPixmap( QIcon::fromTheme( "dialog-error" ).pixmap( 16, 16 ));
        this->ui->pixmapLabel->setPixmap( pixmapCache.findPixmap( "dialog-error", 16 ));
        break;

    case Progress:
    default:
        break;
    }

    // update labels
    this->ui->titleLabel->setText( title );
    this->ui->messageLabel->setText( msg );

    // add to message history if required
    if ( !fromHistory ) {
        Notification item( type, title, msg, timeOut );
        QVariant v;
        v.setValue( item );
        this->historyManager()->addItem( v );
    }

    // raise notification panel
    this->raise( timeOut );
}

/**
 * @brief NotificationPanel::on_closeButton_clicked
 */
void NotificationPanel::on_closeButton_clicked() {
#ifdef SLIDE_OUT_ANIMATION
    QRect end;

    end = this->geometry();
    end.moveTop( end.y() + end.height() + 26 );

    this->animation()->setStartValue( this->geometry());
    this->animation()->setEndValue( end );
    this->animation()->start();
#else
    this->animation()->setStartValue( 0.75f );
    this->animation()->setEndValue( 0.0f );
    this->animation()->start();
#endif
}

/**
 * @brief NotificationPanel::on_prevButton_clicked
 */
void NotificationPanel::on_prevButton_clicked() {
    Notification notification;

    this->historyManager()->back();
    this->timer.stop();

    notification = qvariant_cast<Notification>( this->historyManager()->current());
    this->push( notification.type(), notification.title(), notification.message(), notification.timeout(), true );
}

/**
 * @brief NotificationPanel::on_nextButton_clicked
 */
void NotificationPanel::on_nextButton_clicked() {
    Notification notification;

    this->historyManager()->forward();
    this->timer.stop();

    notification = qvariant_cast<Notification>( this->historyManager()->current());
    this->push( notification.type(), notification.title(), notification.message(), notification.timeout(), true );
}

/**
 * @brief NotificationPanel::checkHistoryPosition
 */
void NotificationPanel::checkHistoryPosition() {
    this->ui->prevButton->setEnabled( this->historyManager()->isBackEnabled());
    this->ui->nextButton->setEnabled( this->historyManager()->isForwardEnabled());
}
