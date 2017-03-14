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
#include "filenamelabel.h"
#include <QPainter>
#include <QDockWidget>
#include <QDebug>

/**
 * @brief FilenameLabel::paintEvent
 * @param event
 */
void FilenameLabel::paintEvent( QPaintEvent *e ) {
    QPainter painter( this );

   // QLabel::paintEvent( e );

   // QWidget *w = qobject_c this->parent();

  //  e
//

   // QFrame::paintEvent( e );


    QRect rect;
    rect = this->contentsRect();
    QTextOption option;
    option.setWrapMode( QTextOption::WrapAnywhere );

#if 0
    QString text = this->text();
    int y;
    int lineBreak = 0;
    for ( y = 0; y < text.length(); y++ ) {
        QString left;

        left = text.mid( lineBreak, y - lineBreak );
        qDebug() << "left" << left << this->fontMetrics().width( left );
        if ( this->fontMetrics().width( left ) > this->contentsRect().width()) {
           // qDenug() << "  break at" << left;
            qDebug() << "breal at left" << left << this->fontMetrics().width( left );

            text.insert( y, " " );
            y++;
            lineBreak = y;
        }
    }
#endif

    painter.drawText( this->contentsRect(), this->text(), option );
}

/**
 * @brief FilenameLabel::sizeHint
 * @return
 */
QSize FilenameLabel::sizeHint() const {
#if 0
    QDockWidget *dock;
    QSize size;

    size = QLabel::sizeHint();
    /*dock = qobject_cast<QDockWidget*>( this->parent()->parent());
    if ( dock != NULL ) {
        qDebug() << "valid parent";
        size.setWidth( dock->geometry().width() - 18 );
    }*/




    int maximum = 0;
    QString text = this->text();
    int y;
    int lineBreak = 0;
    for ( y = 0; y < text.length(); y++ ) {
        QString left;

        left = text.mid( lineBreak, y - lineBreak );
        qDebug() << "left" << left << this->fontMetrics().width( left );
        if ( this->fontMetrics().width( left ) > this->contentsRect().width()) {
           // qDenug() << "  break at" << left;
            qDebug() << "breal at left" << left << this->fontMetrics().width( left );

            text.insert( y, " " );
            y++;
            lineBreak = y;

            maximum = std::max( maximum, this->fontMetrics().width( left ));

        }
    }
    qDebug() << "maximum" << maximum << this->contentsRect().width();
    size.setWidth( maximum );

#endif
    QSize size;
    size = QLabel::sizeHint();
    size.setWidth( 30 );

    return size;
}

/**
 * @brief FilenameLabel::minimumSizeHint
 * @return
 */
QSize FilenameLabel::minimumSizeHint() const {
    QSize size;


    QDockWidget *widget = qobject_cast<QDockWidget*>( this->parent()->parent());

    size = QLabel::minimumSizeHint();
    size.setWidth( widget->width() - 18 );

    return size;
}
