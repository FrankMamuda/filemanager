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
#include <QStorageInfo>
#include "properties.h"
#include "ui_properties.h"
#include "entry.h"
#include "pixmapcache.h"
#include "pathutils.h"
#include "textutils.h"
#include <QTimer>

/**
 * @brief Properties::Properties
 * @param parent
 */
Properties::Properties( QWidget *parent ) : QDialog( parent ), ui( new Ui::Properties ) {
    this->ui->setupUi( this );

    // FIXME: ultra-ugly
    QTimer::singleShot( 50, this, SLOT( resizeMe()));
}

/**
 * @brief Properties::~Properties
 */
Properties::~Properties() {
    delete ui;
}

/**
 * @brief Properties::setDeviceUsage
 */
void Properties::setDeviceUsage( const QString &path ) {
    QStorageInfo info( PathUtils::windowsDevicePath( path ));

    this->ui->deviceUsage->setValue( qreal( info.bytesTotal() - info.bytesAvailable()) / info.bytesTotal() * 100 );
}

/**
 * @brief Properties::setEntry
 * @param entry
 */
void Properties::setEntry( Entry *entry ) {
    if ( entry == NULL )
        return;

    this->entry = entry;

    this->setWindowIcon( QIcon( entry->pixmap( 48 )));
    this->ui->labelIcon->setPixmap( entry->pixmap( 48 ));
    this->ui->size->setText( TextUtils::sizeToText( entry->info().size()), false );
    this->ui->path->setText( PathUtils::toUnixPath( entry->info().absolutePath()), false);
    this->ui->type->setText( entry->mimeType().iconName(), false );
    this->ui->fileName->setText( entry->info().fileName());

    this->setDeviceUsage( entry->path());
}

/**
 * @brief Properties::setEntries
 * @param entries
 */
void Properties::setEntries( QList<Entry *> entries ) {
    quint64 size = 0;
    QIcon icon;

    icon = pixmapCache.fromTheme( /*QIcon::fromTheme(*/ "document-multiple" );

    foreach ( Entry *entry, entries )
        size += entry->info().size();

    this->setWindowIcon( icon );
    this->ui->path->setText( PathUtils::toUnixPath( entries.first()->path()), false );
    this->ui->labelIcon->setPixmap( pixmapCache.pixmap( "document-multiple", 48 ));
    this->ui->type->setText( "multiple entries", false );
    this->ui->fileName->setText( QString( "%1 item(s)" ).arg( entries.count()));
    this->ui->size->setText( TextUtils::sizeToText( size ), false );

    this->setDeviceUsage( entries.first()->path());
}

void Properties::resizeMe()
{
    this->resize( this->width(), this->minimumSizeHint().height());
}

/**
 * @brief Properties::on_buttonBox_accepted
 */
void Properties::on_buttonBox_accepted() {
    this->close();
}

/**
 * @brief Properties::on_buttonBox_rejected
 */
void Properties::on_buttonBox_rejected() {
    this->close();
}
