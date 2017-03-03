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
#include "bookmark.h"
#include "notificationpanel.h"
#include <QPropertyAnimation>
#include <QStatusBar>
#include <QDockWidget>
#include "history.h"

/*
GOALS:
    +/- unix-y file paths with mount points such as /c/smth/smth /d/spam/eggs
    +/- minimalistic/configurable ui
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
    +/- usage of cvars
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
    +/- fix mimetype deletection performance regression
    fix crash on app exit while detecting mimetypes
    notifications in categories - warning, error, info as selectables in status bar
  */

/**
 * @brief MainWindow::MainWindow
 * @param parent
 */

MainWindow::MainWindow( QWidget *parent ) : QMainWindow( parent ), ui( new Ui::MainWindow ), m_currentPath( QDir::currentPath()), m_historyPosition( -1 ) {
    ViewModes viewMode;

    // set up ui
    this->ui->setupUi( this );

    // make history!
    this->m_historyManager = new History();
    this->connect( this->historyManager(), SIGNAL( changed()), this, SLOT( checkHistoryPosition()));
    this->checkHistoryPosition();

    // set current path
    this->setCurrentPath();

    // trigger size change
    Variable::add( "mainWindow/iconSize/sliderPosition", Medium );
    this->ui->horizontalSlider->setValue( Variable::integer( "mainWindow/iconSize/sliderPosition" ));
    this->on_horizontalSlider_valueChanged( Variable::integer( "mainWindow/iconSize/sliderPosition" ));

    // set icons here (some bug resets icons, when defined in form)
    this->ui->actionBack->setIcon( QIcon::fromTheme( "go-previous" ));
    this->ui->actionUp->setIcon( QIcon::fromTheme( "go-up" ));
    this->ui->actionForward->setIcon( QIcon::fromTheme( "go-next" ));

    // get bookmarks
    if ( !Bookmark::count() ) {
        QFileInfoList infoList;
        Bookmark::add( "Root", "/", "folder-red" );
        Bookmark::add( "Home", "/home", "user-home" );

        infoList = QDir::drives();
        foreach ( QFileInfo driveInfo, infoList )
            Bookmark::add( PathUtils::toUnixPath( driveInfo.absolutePath()), PathUtils::toUnixPath( driveInfo.absolutePath()), "drive-harddisk" );

        Bookmark::add( "Trash", "trash://", "user-trash" );
    }

    // setup view mode list
    this->viewModeMenu = new QMenu();
    this->menuStyle = new MenuStyle;
    this->viewModeMenu->setStyle( this->menuStyle );
    this->actionViewGrid = new QAction( QIcon::fromTheme( "preferences-desktop-icons" ), this->tr( "Grid view" ));
    this->actionViewList = new QAction( QIcon::fromTheme( "view-media-playlist" ), this->tr( "List view" ));
    this->actionViewDetails = new QAction( QIcon::fromTheme( "x-office-spreadsheet" ), this->tr( "Detail view" ));
    this->viewModeMenu->addAction( this->actionViewGrid );
    this->viewModeMenu->addAction( this->actionViewList );
    this->viewModeMenu->addAction( this->actionViewDetails );
    this->ui->actionViewMode->setMenu( this->viewModeMenu );
    this->ui->actionViewMode->setIcon( QIcon::fromTheme( "preferences-system-windows-actions" ));
    this->connect( this->actionViewGrid, SIGNAL( triggered( bool )), this, SLOT( setGridView()));
    this->connect( this->actionViewList, SIGNAL( triggered( bool )), this, SLOT( setListView()));
    this->connect( this->actionViewDetails, SIGNAL( triggered( bool )), this, SLOT( setDetailView()));

    // restore view mode
    viewMode = Variable::value<ViewModes>( "mainWindow/viewMode", IconMode );

    if ( viewMode == IconMode )
        this->setGridView();
    else if ( viewMode == ListMode )
        this->setListView();
    else if ( viewMode == DetailMode )
        this->setDetailView();

    // notification icons
    this->ui->notificationInfo->setIcon( QIcon::fromTheme( "dialog-information" ));
    this->ui->notificationWarn->setIcon( QIcon::fromTheme( "dialog-warning" ));
    this->ui->notificationError->setIcon( QIcon::fromTheme( "dialog-error" ));

    // notification panel
    this->panel = new NotificationPanel( this );
    this->panel->hide();

    // TODO: QDockWidget for sideView?
}

/**
 * @brief MainWindow::resizeEvent
 */
void MainWindow::resizeEvent( QResizeEvent *e ) {
    this->ui->listView->model()->reset();
    this->ui->tableView->model()->reset();
    this->panel->move( this->geometry().width() - this->panel->geometry().width() - 11, this->geometry().height() - this->panel->geometry().height() - this->ui->statusBar->height() - 4 );
    QMainWindow::resizeEvent( e );
}

/**
 * @brief MainWindow::setCurrentPath
 * @param path
 * @param changeDir
 */
// TODO: make static
void MainWindow::setCurrentPath( const QString &path, bool saveToHistory ) {
    QDir directory;
    QString windowsPath, unixPath;

    // build special file list for root
    if ( !QString::compare( path, "/" )) {
        this->ui->listView->model()->setMode( ContainerModel::SideMode );
        this->ui->tableView->model()->setMode( ContainerModel::SideMode );
        this->ui->pathEdit->setText( "/" );
        this->m_currentPath = "/";
        this->ui->actionUp->setDisabled( true );
    } else if ( !QString::compare( path, "trash://" )) {
        this->ui->pathEdit->setText( path );
        this->m_currentPath = "trash://";
        this->ui->actionUp->setDisabled( true );
    } else if ( !QString::compare( path, "bookmarks://" )) {
        this->ui->pathEdit->setText( path );
        this->m_currentPath = "bookmarks://";
        this->ui->actionUp->setDisabled( true );
    } else {
        this->ui->actionUp->setEnabled( true );


        windowsPath = PathUtils::toWindowsPath( path );

        // handle symlinks
        QFileInfo info( windowsPath );
        if ( info.isSymLink()) {
            QFileInfo target( info.symLinkTarget());
            if ( !target.isSymLink())
                windowsPath = target.absoluteFilePath();
        }
        directory.cd( windowsPath );

        if ( !directory.exists( windowsPath )) {
            this->panel->pushNotification( NotificationPanel::Error, "Error", "Path does not exist" );
            return;
        }

        unixPath = PathUtils::toUnixPath( windowsPath );
        this->m_currentPath = unixPath;
        this->ui->pathEdit->setText( unixPath );

        this->ui->listView->model()->setMode( ContainerModel::FileMode );
        this->ui->tableView->model()->setMode( ContainerModel::FileMode );
    }

    this->ui->statusBar->setText( this->tr( "%1 items" ).arg( this->ui->listView->model()->numItems()));

    if ( saveToHistory )
        this->historyManager()->addItem( this->m_currentPath );
}

/**
 * @brief MainWindow::~MainWindow
 */
MainWindow::~MainWindow() {
    // get rid of ui
    delete ui;

    this->disconnect( this->historyManager(), SIGNAL( changed()));
    this->m_historyManager->deleteLater();

    this->viewModeMenu->deleteLater();
    this->menuStyle->deleteLater();
    this->actionViewGrid->deleteLater();
    this->actionViewList->deleteLater();
    this->actionViewDetails->deleteLater();
    this->disconnect( this->actionViewGrid, SIGNAL( triggered( bool )));
    this->disconnect( this->actionViewList, SIGNAL( triggered( bool )));
    this->disconnect( this->actionViewDetails, SIGNAL( triggered( bool )));

    this->panel->deleteLater();
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

    // TODO: fix view mode switching to avoid too frequent mime type detection
  //  ..if ( this->ui->stackedWidget->currentIndex() == 1 )
  // ./     return;

    value *= 16;
    this->ui->listView->model()->setIconSize( value );
    this->ui->listView->switchDisplayMode( this->ui->listView->viewMode());
}

/**
 * @brief MainWindow::on_actionBack_triggered
 */
void MainWindow::on_actionBack_triggered() {
    this->historyManager()->back();
    this->setCurrentPath( this->historyManager()->current(), false );
}

/**
 * @brief MainWindow::on_actionForward_triggered
 */
void MainWindow::on_actionForward_triggered() {
    this->historyManager()->forward();
    this->setCurrentPath( this->historyManager()->current(), false );
}

/**
 * @brief MainWindow::checkHistoryPosition
 */
void MainWindow::checkHistoryPosition() {
    qDebug() << "check" << this->historyManager()->position() << this->historyManager()->count();
    qDebug() << "     " << this->historyManager()->isBackEnabled() << this->historyManager()->isForwardEnabled();

    this->ui->actionBack->setEnabled( this->historyManager()->isBackEnabled());
    this->ui->actionForward->setEnabled( this->historyManager()->isForwardEnabled());
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
    if ( this->ui->stackedWidget->currentIndex() == 1 ) {
        this->ui->listView->selectionModel()->select( this->ui->tableView->selectionModel()->selection(), QItemSelectionModel::Select );
    }

    if ( this->ui->listView->isVisible()) {
        qDebug() << "## SET ICON VIEW";
        this->ui->listView->model()->reset( true );
    }

    this->ui->tableView->setHidden( true );
    this->ui->listView->setVisible( true );

    this->ui->stackedWidget->setCurrentIndex( 0 );
    this->ui->listView->switchDisplayMode( QListView::IconMode );
    this->ui->horizontalSlider->setEnabled( true );

    Variable::setValue( "mainWindow/viewMode", IconMode );
}

/**
 * @brief MainWindow::setListView
 */
void MainWindow::setListView() {
    if ( this->ui->stackedWidget->currentIndex() == 1 ) {
        this->ui->listView->selectionModel()->select( this->ui->tableView->selectionModel()->selection(), QItemSelectionModel::Select );
    }

    if ( this->ui->listView->isVisible()) {
        qDebug() << "## SET LIST VIEW";
        this->ui->listView->model()->reset( true );
    }

    this->ui->tableView->setHidden( true );
    this->ui->listView->setVisible( true );

    this->ui->stackedWidget->setCurrentIndex( 0 );
    this->ui->listView->switchDisplayMode( QListView::ListMode );
    this->ui->horizontalSlider->setEnabled( true );

    Variable::setValue( "mainWindow/viewMode", ListMode );
}

/**
 * @brief MainWindow::setDetailView
 */
void MainWindow::setDetailView() {    
    if ( this->ui->stackedWidget->currentIndex() == 0 ) {
        this->ui->tableView->selectionModel()->select( this->ui->listView->selectionModel()->selection(), QItemSelectionModel::Select | QItemSelectionModel::Rows );
    }

    if ( !this->ui->tableView->isVisible()) {
        qDebug() << "## SET DETAIL VIEW";
        this->ui->tableView->model()->reset( true );
    }

    this->ui->listView->setHidden( true );
    this->ui->tableView->setVisible( true );

    this->ui->stackedWidget->setCurrentIndex( 1 );
    this->ui->horizontalSlider->setEnabled( false );

    Variable::setValue( "mainWindow/viewMode", DetailMode );
}

/**
 * @brief MainWindow::addToHistory
 * @param path
 */
void MainWindow::addToHistory( const QString &path ) {
    Q_UNUSED( path )

    /*this->history.insert( this->historyPosition() + 1, path );
    this->m_historyPosition++;

    qDebug() << "add" << path << this->historyPosition() << this->history.count();

    this->checkHistoryPosition();*/
}

/**
 * @brief MainWindow::on_notificationInfo_clicked
 */
void MainWindow::on_notificationInfo_clicked() {
    this->panel->move( this->geometry().width() - this->panel->geometry().width() - 11,
                       this->geometry().height() - this->panel->geometry().height() - this->ui->statusBar->height() - 4 );

    this->panel->pushNotification( NotificationPanel::Information, "INFO", "aaaa\n" );
}
