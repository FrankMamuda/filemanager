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
#include <QDebug>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "pathutils.h"
#include "entry.h"
#include "containermodel.h"
#include <QMenu>
#include "variable.h"
#include "main.h"

/*
GOALS:
    +/- unix-y file paths with mount points such as /c/smth/smth /d/spam/eggs
    minimalistic/configurable ui
    +/- mime type detection, not reliance extension
      (only if unknown or text/plain to determine if cpp etc.)
      (sort of a hybrid between mime/extension)
      (must periodically check for missing apps)
    text file previews/in-app editing
    integrated console (custom or bash)
    bookmarks to tools (as folders, single launchers)
    .desktop file support
    + icon pack support
    scripted right-click context menus
    usage of cvars
    homedir can be set (pseudo-mounted) at /home (can point to anything)
    tabbled and multiview
    notifications on copy/link/delete etc. (for now use system?)
      custom copy/move dialogs (as notifications)
    +/- thumbnail cache and cleanup
      (cache preview size limit)
      (cache preview store by md5 hash?)
    text previews just read the first few bytes
    compressed file browsing
    on first run match:
        HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.1\OpenWithList
        to
        with mime type database and create a list of default programs
    implement QFileSystemWatcher for file updates
    tooltips
    fix mimetype deletection performance regression
  */

/**
 * @brief MainWindow::MainWindow
 * @param parent
 */

MainWindow::MainWindow( QWidget *parent ) : QMainWindow( parent ), ui( new Ui::MainWindow ), m_currentPath( QDir::currentPath()) {
    this->ui->setupUi( this );

    // set current path
    this->setCurrentPath();

    // trigger size change
    Variable::add( "mainWindow/iconSize/sliderPosition", m.settings, 3 );
    this->ui->horizontalSlider->setValue( Variable::integer( "mainWindow/iconSize/sliderPosition" ));
    this->on_horizontalSlider_valueChanged( Variable::integer( "mainWindow/iconSize/sliderPosition" ));

    // this->ui->tableView->setSelectionModel( this->ui->listView->selectionModel());
    this->checkPosition();

    // set icons here (some bug resets icons, when defined in form)
    this->ui->actionBack->setIcon( QIcon::fromTheme( "go-previous" ));
    this->ui->actionUp->setIcon( QIcon::fromTheme( "go-up" ));
    this->ui->actionForward->setIcon( QIcon::fromTheme( "go-next" ));

    // setup view mode list
    this->viewModeMenu = new QMenu();
    this->menuStyle = new MenuStyle;
    this->viewModeMenu->setStyle( this->menuStyle );
    this->actionViewGrid = new QAction( QIcon::fromTheme( "preferences-desktop-icons" ), "Grid view" );
    this->actionViewList = new QAction( QIcon::fromTheme( "view-media-playlist" ), "List view" );
    this->actionViewDetails = new QAction( QIcon::fromTheme( "x-office-spreadsheet" ), "Detail view" );
    this->viewModeMenu->addAction( this->actionViewGrid );
    this->viewModeMenu->addAction( this->actionViewList );
    this->viewModeMenu->addAction( this->actionViewDetails );
    this->ui->actionViewMode->setMenu( this->viewModeMenu );
    this->ui->actionViewMode->setIcon( QIcon::fromTheme( "preferences-system-windows-actions" ));
    this->connect( this->actionViewGrid, SIGNAL( triggered( bool )), this, SLOT( setGridView()));
    this->connect( this->actionViewList, SIGNAL( triggered( bool )), this, SLOT( setListView()));
    this->connect( this->actionViewDetails, SIGNAL( triggered( bool )), this, SLOT( setDetailView()));

    //   this->ui->tableView->setSelectionModel( new QItemSelectionModel());

    Variable::add( "mainWindow/viewMode", m.settings, 0 );
    int value = Variable::integer( "mainWindow/viewMode" );

    if ( value == 0 )
        this->setGridView();
    else if ( value == 1 )
        this->setListView();
    else if ( value == 2 )
        this->setDetailView();
}

/**
 * @brief MainWindow::setCurrentPath
 * @param path
 * @param changeDir
 */
// TODO: make static
void MainWindow::setCurrentPath( const QString &path ) {
    QDir directory;
    QString windowsPath, unixPath;

    // build special file list for root
    if ( !QString::compare( path, "/" )) {
        this->ui->listView->model()->setMode( ContainerModel::SideMode );
        this->ui->tableView->model()->setMode( ContainerModel::SideMode );
        this->ui->pathEdit->setText( "/" );
        this->m_currentPath = "/";

        // HISTORY
        /*if ( QString::compare( this->history.at( this->m_position ), "/" )) {
            qDebug() << "insert"  << "/" << "at" << this->position();
            this->history.insert( this->m_position++, "/" );
        }*/
        return;
    } else if ( !QString::compare( path, "trash://" )) {
        this->ui->pathEdit->setText( path );
        this->m_currentPath = "trash://";
        return;
    }

    windowsPath = PathUtils::toWindowsPath( path );
    directory.cd( windowsPath );

    if ( !directory.exists( windowsPath )) {
        qDebug() << path << "doesn't exist";
        return;
    }

    unixPath = PathUtils::toUnixPath( path );
    this->m_currentPath = path;
    this->ui->pathEdit->setText( unixPath );

    this->ui->listView->model()->setMode( ContainerModel::FileMode );
    this->ui->tableView->model()->setMode( ContainerModel::FileMode );

    this->ui->statusBar->setText( QString( "%1 items" ).arg( this->ui->listView->model()->numItems()));

    // HISTORY
    /*if ( QString::compare( this->history.at( this->m_position ), unixPath )) {
        qDebug() << "insert"  << unixPath << "at" << this->position();
        this->history.insert( this->m_position++, unixPath );
    }*/
}

/**
 * @brief MainWindow::~MainWindow
 */
MainWindow::~MainWindow() {
    delete ui;

    this->viewModeMenu->deleteLater();
    this->menuStyle->deleteLater();
    this->actionViewGrid->deleteLater();
    this->actionViewList->deleteLater();
    this->actionViewDetails->deleteLater();
    this->disconnect( this->actionViewGrid, SIGNAL( triggered( bool )));
    this->disconnect( this->actionViewList, SIGNAL( triggered( bool )));
    this->disconnect( this->actionViewDetails, SIGNAL( triggered( bool )));
}

/**
 * @brief MainWindow::resizeEvent
 */
void MainWindow::resizeEvent( QResizeEvent * ) {
    this->ui->listView->model()->reset();
    this->ui->tableView->model()->reset();
}

/**
 * @brief MainWindow::on_actionUp_triggered
 */
void MainWindow::on_actionUp_triggered() {
    QDir directory( PathUtils::toWindowsPath( this->currentPath()));

    // go up one level
    if ( !directory.cdUp()) {
        if ( PathUtils::isWindowsDevicePath( directory.absolutePath())) {
            this->setCurrentPath( "/" );
            return;
        }
        return;
    }

    this->setCurrentPath( directory.absolutePath());
}

/**
 * @brief MainWindow::on_pathEdit_returnPressed
 */
void MainWindow::on_pathEdit_returnPressed() {
    this->setCurrentPath( this->ui->pathEdit->text());
}

/**
 * @brief MainWindow::on_horizontalSlider_valueChanged
 * @param value
 */
void MainWindow::on_horizontalSlider_valueChanged( int value ) {
    Variable::setValue( "mainWindow/iconSize/sliderPosition", value );

    value *= 16;
    this->ui->listView->model()->setIconSize( value );
    this->ui->listView->switchDisplayMode( this->ui->listView->viewMode());
}

/**
 * @brief MainWindow::on_actionBack_triggered
 */
void MainWindow::on_actionBack_triggered() {
    if ( this->position() <= 0 || this->history.size() < 1 )
        return;

    this->m_position--;
    this->setCurrentPath( this->history.at( this->position()));
    this->checkPosition();
}

/**
 * @brief MainWindow::on_actionForward_triggered
 */
void MainWindow::on_actionForward_triggered() {
    if ( this->position() == this->history.size() - 1 || this->history.size() < 1 )
        return;

    this->m_position++;
    this->setCurrentPath( this->history.at( this->position()));
    this->checkPosition();
}

/**
 * @brief MainWindow::checkPosition
 */
void MainWindow::checkPosition() {
    /*if ( this->position() <= 0 )
        this->ui->actionBack->setEnabled( false );
    else if ( this->position() > 0 && this->position() < this->history.size())
        this->ui->actionBack->setEnabled( true );

    if ( this->position() == this->history.size() - 1 )
        this->ui->actionForward->setEnabled( false );
    else if ( this->position() < this->history.size() - 1 && this->position() > 0 )
        this->ui->actionForward->setEnabled( true );*/
}

/**
 * @brief MainWindow::on_actionViewMode_triggered
 */
void MainWindow::on_actionViewMode_triggered() {
    this->ui->actionViewMode->menu()->exec( QCursor::pos());
}

/**
 * @brief MainWindow::setGridView
 */
void MainWindow::setGridView() {
    if ( this->ui->stackedWidget->currentIndex() == 1 )
        this->ui->listView->selectionModel()->select( this->ui->tableView->selectionModel()->selection(), QItemSelectionModel::Select );

    this->ui->stackedWidget->setCurrentIndex( 0 );
    this->ui->listView->switchDisplayMode( QListView::IconMode );
    this->ui->horizontalSlider->setEnabled( true );

    Variable::setValue( "mainWindow/viewMode", 0 );
}

/**
 * @brief MainWindow::setListView
 */
void MainWindow::setListView() {
    if ( this->ui->stackedWidget->currentIndex() == 1 )
        this->ui->listView->selectionModel()->select( this->ui->tableView->selectionModel()->selection(), QItemSelectionModel::Select );

    this->ui->stackedWidget->setCurrentIndex( 0 );
    this->ui->listView->switchDisplayMode( QListView::ListMode );
    this->ui->horizontalSlider->setEnabled( true );

    Variable::setValue( "mainWindow/viewMode", 1 );
}

/**
 * @brief MainWindow::setDetailView
 */
void MainWindow::setDetailView() {
    if ( this->ui->stackedWidget->currentIndex() == 0 )
        this->ui->tableView->selectionModel()->select( this->ui->listView->selectionModel()->selection(), QItemSelectionModel::Select | QItemSelectionModel::Rows );

    this->ui->stackedWidget->setCurrentIndex( 1 );
    this->ui->horizontalSlider->setEnabled( false );

    Variable::setValue( "mainWindow/viewMode", 2 );
}
