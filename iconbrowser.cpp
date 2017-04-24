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
#include "iconbrowser.h"
#include "ui_iconbrowser.h"
#include "iconmodel.h"
#include <QFileDialog>

/**
 * @brief IconBrowser::IconBrowser
 * @param parent
 */
IconBrowser::IconBrowser( QWidget *parent ) : QDialog( parent ), ui( new Ui::IconBrowser ) {
    this->ui->setupUi( this );
    this->iconModel = new IconModel( this->ui->iconView );
    this->ui->iconView->setModel( this->iconModel );
   // this->ui->iconView->reset();
}

/**
 * @brief IconBrowser::~IconBrowser
 */
IconBrowser::~IconBrowser() {
    if ( this->iconModel != NULL )
        this->iconModel->deleteLater();

    delete ui;
}

/**
 * @brief IconBrowser::selectedPixmap
 * @return
 */
QPixmap IconBrowser::selectedPixmap() const {
    return this->iconModel->pixmapForIndex( this->currentIndex );
}

/**
 * @brief IconBrowser::on_iconView_clicked
 * @param index
 */
void IconBrowser::on_iconView_clicked( const QModelIndex &index ) {
    this->currentIndex = index;
    this->ui->iconName->setText( this->iconModel->nameForIndex( index ));
}

/**
 * @brief IconBrowser::resizeEvent
 * @param e
 */
void IconBrowser::resizeEvent( QResizeEvent *e ) {
    this->iconModel->reset();
    QDialog::resizeEvent( e );
}

/**
 * @brief IconBrowser::on_pushButton_clicked
 */
void IconBrowser::on_pushButton_clicked() {
    QPixmap pixmap;

    pixmap.load( QFileDialog::getOpenFileName( this, this->tr( "Open Image" ), QDir::currentPath(), this->tr( "Image Files (*.png *.jpg *.bmp)" )));
    if ( !pixmap.isNull()) {
        this->iconModel->clear();
        this->iconModel->addIcon( "scaled to height", pixmap.scaledToHeight( 32, Qt::SmoothTransformation ));
        this->iconModel->addIcon( "scaled to width", pixmap.scaledToWidth( 32, Qt::SmoothTransformation ));

        QRect rect;
        if ( pixmap.width() > pixmap.height())
            rect = QRect( pixmap.width() / 2 - pixmap.height() / 2, 0, pixmap.height(), pixmap.height());
        else if ( pixmap.width() < pixmap.height())
            rect = QRect( 0, pixmap.height() / 2 - pixmap.width() / 2, pixmap.width(), pixmap.width());

        pixmap = pixmap.copy( rect );
        this->iconModel->addIcon( "cropped to square", pixmap.scaled( 32, 32, Qt::IgnoreAspectRatio, Qt::SmoothTransformation ));
    }
}
