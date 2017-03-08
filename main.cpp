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
#include "main.h"
#include "mainwindow.h"
#include "pixmapcache.h"
#include "notificationpanel.h"
#include <QStyleFactory>

//
// classes
//
class Main m;

//
// defines
//
//#define DARK_PALETTE

/**
 * @brief qMain
 * @param argc
 * @param argv
 * @return
 */
int main( int argc, char *argv[] ) {
    QApplication a( argc, argv );
    MainWindow w;
    NotificationPanel *notify;
    QDir iconDir( QDir::currentPath() + "/icons" );

    // set up icon theme
    QIcon::setThemeSearchPaths( QStringList( iconDir.absolutePath()));
    QIcon::setThemeName( "oxygen" );

    // style app
    QApplication::setStyle( QStyleFactory::create("Fusion"));
#ifdef DARK_PALETTE
    QPalette darkPalette;
    darkPalette.setColor( QPalette::Window, QColor( 53,53,53 ));
    darkPalette.setColor( QPalette::WindowText, Qt::white );
    darkPalette.setColor( QPalette::Base, QColor( 25,25,25 ));
    darkPalette.setColor( QPalette::AlternateBase, QColor( 53,53,53 ));
    darkPalette.setColor( QPalette::ToolTipBase, Qt::white );
    darkPalette.setColor( QPalette::ToolTipText, Qt::white );
    darkPalette.setColor( QPalette::Text, Qt::white );
    darkPalette.setColor( QPalette::Button, QColor( 53,53,53 ));
    darkPalette.setColor( QPalette::ButtonText, Qt::white );
    darkPalette.setColor( QPalette::BrightText, Qt::red );
    darkPalette.setColor( QPalette::Link, QColor( 42, 130, 218 ));

    darkPalette.setColor( QPalette::Highlight, QColor( 42, 130, 218 ));
    darkPalette.setColor( QPalette::HighlightedText, Qt::black );

    qApp->setPalette( darkPalette );
    qApp->setStyleSheet( "QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }" );
#endif

    // display app
    m.setGui( &w );
    w.show();

    // create notification widget
    notify = new NotificationPanel( &w );
    notify->hide();
    m.setNotifications( notify );

    // failsafe
    if ( !iconDir.exists())
        QMessageBox::warning( &w, "Warning", "Icon directory does not exist", QMessageBox::Ok );

    // default icon
    if ( pixmapCache.pixmap( "application-x-zerosize", 48 ).width() == 0 )
        QMessageBox::warning( &w, "Warning", "Invalid icon theme", QMessageBox::Ok );

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
}
