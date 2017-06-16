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
#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QStyleFactory>
#include <QThread>
#include <QDebug>
#include <QDesktopWidget>
#include "main.h"
#include "mainwindow.h"
#include "pixmapcache.h"
#include "notificationpanel.h"
#include "cache.h"
#include "iconcache.h"
#include "fileutils.h"

//
// classes
//
class Main m;

//
// defines
//
#define THREADED_CACHE
struct Work;


/**
 * @brief qMain
 * @param argc
 * @param argv
 * @return
 */
int main( int argc, char *argv[] ) {
    QApplication a( argc, argv );

    // report thread
    qDebug() << "Strating app on thread" << QThread::currentThreadId();

    // register metatypes
    qRegisterMetaType<Hash>( "Hash" );
    qRegisterMetaType<DataEntry>( "DataEntry" );
    qRegisterMetaType<Work>( "Work" );
    qRegisterMetaType<IconEntry>( "IconEntry" );
    qRegisterMetaType<IconIndex>( "IconIndex" );
    qRegisterMetaType<PixmapEntry>( "PixmapEntry" );

    // set up icon theme
#ifdef Q_OS_WIN32
    QDir iconDir( QDir::currentPath() + "/icons" );
#else
    QDir iconDir( "/usr/share/icons" );
#endif
    QIcon::setThemeSearchPaths( QStringList( iconDir.absolutePath()));
    QIcon::setThemeName( Ui::lightIconTheme );

    // build pixmap index
    m.pixmapCache = new PixmapCache( QDir::homePath() + "/.filemanager/cache" );
    m.pixmapCache->buildIndex( Ui::lightIconTheme );
    m.pixmapCache->buildIndex( Ui::darkIconTheme );

    // init cache, run it from a separate thread
    m.cache = new Cache( QDir::homePath() + "/.filemanager/cache" );
    m.iconCache = new IconCache( QDir::homePath() + "/.filemanager/cache" );
#ifdef THREADED_CACHE
    QThread thread;

    m.cache->moveToThread( &thread );
    thread.connect( qApp, SIGNAL( aboutToQuit()), SLOT( quit()));
    thread.start();
    m.iconCache->connect( qApp, SIGNAL( aboutToQuit()), SLOT( quit()));
    m.iconCache->start();
#endif

    // style app
    QApplication::setStyle( QStyleFactory::create( "Fusion" ));

    // create main window
    MainWindow w;

    // display app
    m.setGui( &w );
    w.show();

    // create notification widget
    NotificationPanel *notify;
    QDesktopWidget desktop;
    notify = new NotificationPanel( &desktop );// &w );
    notify->hide();
    m.setNotifications( notify );

    // failsafe
    if ( !iconDir.exists())
        QMessageBox::warning( &w, "Warning", "Icon directory does not exist", QMessageBox::Ok );

    // default icon
    if ( m.pixmapCache->pixmap( "application-x-zerosize", 48 ).width() == 0 )
        QMessageBox::warning( &w, QObject::tr( "Warning" ), QObject::tr( "Invalid icon theme" ), QMessageBox::Ok );

    // return success
    return a.exec();
}

/**
 * @brief Main::Main
 */
Main::Main() {
    this->settings = new QSettings( QDir::homePath() + "/.filemanager/settings.conf", QSettings::IniFormat );
}

/**
 * @brief Main::~Main
 */
Main::~Main() {
    delete this->settings;
    this->cache->deleteLater();
    this->iconCache->deleteLater();
    this->pixmapCache->deleteLater();
}
