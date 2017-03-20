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

#ifndef TEXTLABEL_H
#define TEXTLABEL_H

//
// defines
//
#include <QTextEdit>
#include <QPaintEvent>
#include <QPainter>
#include <QAbstractTextDocumentLayout>

class TextLabel : public QTextEdit {
    Q_PROPERTY( QPixmap pixmap READ pixmap WRITE setPixmap )
    Q_PROPERTY( bool pixmapSet READ pixmap )

public:
    TextLabel( QWidget *parent = 0 ) : QTextEdit( parent ), m_pixmapSet( false ) {
        this->setStyleSheet( "QTextEdit { background-color: transparent; text-align: center; }" );
        this->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum );
        this->setInputMethodHints( Qt::ImhNone );
        this->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
        this->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
        this->setFrameShape( QFrame::NoFrame );
        this->setTextInteractionFlags( Qt::NoTextInteraction );
        this->setSizeAdjustPolicy( QAbstractScrollArea::AdjustToContents );
        this->setReadOnly( true );
        this->setUndoRedoEnabled( false );
        this->setAcceptDrops( false );
        this->setDisabled( true );
    }

    QPixmap pixmap() const { return this->m_pixmap; }
    bool isPixmapSet() const { return this->m_pixmapSet; }

public slots:
    void setPixmap( const QPixmap &pixmap ) {
        this->m_pixmapSet = true;
        this->m_pixmap = pixmap;
        this->update();
        this->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );
    }
    void setText( const QString &text, bool centered = true ) {
        if ( centered )
            this->setHtml( QString( "<center>%1</center>" ).arg( text ));
        else
            this->setHtml( text );
    }

protected:
    void paintEvent(QPaintEvent *) {
        QAbstractTextDocumentLayout::PaintContext context;
        QPainter painter( this->viewport());

        if ( !this->isPixmapSet()) {
            painter.translate( 0, ( this->contentsRect().height() - this->document()->size().height()) / 2 );
            this->document()->documentLayout()->draw( &painter, context );
            this->setMinimumHeight( this->document()->size().height());
            this->setMaximumHeight( this->document()->size().height());
        } else {
            QPixmap pm;
            int horizontalOffset, verticalOffset;

            this->setMinimumHeight( this->contentsRect().height());
            if ( this->pixmap().isNull())
                return;

            pm = this->pixmap().scaled( std::min( this->contentsRect().size().width(), this->pixmap().width()), std::min( this->contentsRect().size().height(), this->pixmap().height()), Qt::KeepAspectRatio, Qt::SmoothTransformation );
            horizontalOffset = ( this->contentsRect().width() - pm.width()) / 2;
            verticalOffset = ( this->contentsRect().height() - pm.height()) / 2;

            painter.drawPixmap( horizontalOffset, verticalOffset, pm.size().width(), pm.size().height(), pm );
            this->setMinimumHeight( pm.size().height());
            this->setMaximumHeight( 16777215 );
        }
    }
private:
    bool m_pixmapSet;
    QPixmap m_pixmap;
};


#endif // TEXTLABEL_H
