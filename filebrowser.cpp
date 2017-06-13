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
#include <QMimeDatabase>
#include "filebrowser.h"
#include "ui_filebrowser.h"
#include "variable.h"
#include "pixmapcache.h"
#include "containermodel.h"
#include "mainwindow.h"
#include "pathutils.h"
#include "notificationpanel.h"
#include "entry.h"
#include "textutils.h"
#include "worker.h"
#include "history.h"

/**
 * @brief FileBrowser::FileBrowser
 * @param parent
 */
FileBrowser::FileBrowser( QWidget *parent ) : QMainWindow( parent ), ui( new Ui::FileBrowser ), m_historyManager( new History()) {
    // set up ui
    this->ui->setupUi( this );

    // set current path
    this->ui->navigationBar->setFileBrowser( this );
    this->setCurrentPath( QDir::currentPath(), false );

    // make history!
    this->connect( this->historyManager(), SIGNAL( changed()), this, SLOT( checkHistoryPosition()));
    this->checkHistoryPosition();

    // set up selection models
    this->ui->listView->setSelectionModel( this->ui->listView->model()->selectionModel());
    this->ui->tableView->setSelectionModel( this->ui->tableView->model()->selectionModel());

    // set up toolbars
    this->setupToolBar();
    this->setupNavigationBar();
    this->setupFrameBar();

    // setup view mode and actions
    this->setupViewModes();

    // remove dock titlebar
    // TODO: deleteme
    this->ui->dockInfo->setTitleBarWidget( new QWidget());

    // FOR NOW
    this->ui->tabWidget->setTabText( 0, "File manager" );
    this->ui->tabWidget->setTabIcon( 0, m.pixmapCache->icon( "inode-folder", 16 ));
    this->ui->tabWidget->removeTab( 1 );

    // filesystem updates
    this->connect( &pathUtils.watcher, SIGNAL( directoryChanged( QString )), this, SLOT( directoryChanged( QString )));
}

/**
 * @brief FileBrowser::setupToolBar
 */
void FileBrowser::setupToolBar() {
    this->ui->actionBookmarks->setIcon( m.pixmapCache->icon( "folder-bookmark", this->ui->mainToolBar->iconSize().width()));
    this->ui->actionInfo->setIcon( m.pixmapCache->icon( "dialog-information", this->ui->mainToolBar->iconSize().width()));

    // FIXME: a little messy
    Variable::add( "mainWindow/bookmarkPanelVisible", true );
    Variable::add( "mainWindow/infoPanelVisible", true );
    this->ui->actionBookmarks->setChecked( Variable::isEnabled( "mainWindow/bookmarkPanelVisible" ));
    this->ui->actionInfo->setChecked( Variable::isEnabled( "mainWindow/infoPanelVisible" ));
    this->on_actionBookmarks_toggled( Variable::isEnabled( "mainWindow/bookmarkPanelVisible" ));
    this->on_actionInfo_toggled( Variable::isEnabled( "mainWindow/infoPanelVisible" ));
}

/**
 * @brief FileBrowser::setupFrameBar
 */
void FileBrowser::setupFrameBar() {
    // TODO: deleteme
    QWidget* spacer = new QWidget( this );
    spacer->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    this->ui->frameToolbar->addWidget( spacer );
    this->ui->actionMinimize->setIcon( m.pixmapCache->icon( "window-minimize-symbolic" ));
    this->ui->actionMaximize->setIcon( m.pixmapCache->icon( "window-maximize-symbolic" ));
    this->ui->actionClose->setIcon( m.pixmapCache->icon( "window-close-symbolic" ));
    this->ui->frameToolbar->addAction( this->ui->actionMinimize );
    this->ui->frameToolbar->addAction( this->ui->actionMaximize );
    this->ui->frameToolbar->addAction( this->ui->actionClose );
}

/**
 * @brief FileBrowser::setupNavigationBar
 */
void FileBrowser::setupNavigationBar() {
    this->insertToolBarBreak( this->ui->navigationToolbar );
    this->ui->actionBack->setIcon( m.pixmapCache->icon( "go-previous" ));
    this->ui->actionUp->setIcon( m.pixmapCache->icon( "go-up" ));
    this->ui->actionForward->setIcon( m.pixmapCache->icon( "go-next" ));
    this->ui->navigationToolbar->addWidget( this->ui->navigationBar );

    // TODO: deleteme
    //QWidget* spacer = new QWidget();
    //spacer->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    // this->ui->navigationToolbar->addWidget( spacer );
}

/**
 * @brief FileBrowser::setupViewModes
 */
void FileBrowser::setupViewModes() {
    ViewModes viewMode;

    // create and style view mode menu
    this->viewModeMenu = new QMenu();
    this->menuStyle = new MenuStyle();
    this->viewModeMenu->setStyle( this->menuStyle );

    // add actions
    this->actionViewGrid = new QAction( m.pixmapCache->icon( "preferences-desktop-icons" ), this->tr( "Grid view" ));
    this->actionViewList = new QAction( m.pixmapCache->icon( "view-media-playlist" ), this->tr( "List view" ));
    this->actionViewDetails = new QAction( m.pixmapCache->icon( "x-office-spreadsheet" ), this->tr( "Detail view" ));
    this->connect( this->actionViewGrid, SIGNAL( triggered( bool )), this, SLOT( setGridView()));
    this->connect( this->actionViewList, SIGNAL( triggered( bool )), this, SLOT( setListView()));
    this->connect( this->actionViewDetails, SIGNAL( triggered( bool )), this, SLOT( setDetailView()));
    this->viewModeMenu->addAction( this->actionViewGrid );
    this->viewModeMenu->addAction( this->actionViewList );
    this->viewModeMenu->addAction( this->actionViewDetails );

    // set view mode list to view mode action
    this->ui->actionViewMode->setMenu( this->viewModeMenu );
    this->ui->actionViewMode->setIcon( m.pixmapCache->icon( "preferences-system-windows-actions" ));

    // restore view mode
    viewMode = Variable::value<ViewModes>( "mainWindow/viewMode", IconMode );

    if ( viewMode == IconMode )
        this->setGridView();
    else if ( viewMode == ListMode )
        this->setListView();
    else if ( viewMode == DetailMode )
        this->setDetailView();

    // connect info panel for updates
    this->connect( this->ui->listView->selectionModel(), SIGNAL( selectionChanged( QItemSelection, QItemSelection )), this, SLOT( updateInfoPanel()));
    this->connect( this->ui->tableView->selectionModel(), SIGNAL( selectionChanged( QItemSelection, QItemSelection )), this, SLOT( updateInfoPanel()));
}

/**
 * @brief FileBrowser::directoryChanged
 * @param directory
 */
void FileBrowser::directoryChanged( const QString &directory ) {
    if ( !QString::compare( PathUtils::toUnixPath( directory ), pathUtils.currentPath )) {
        qDebug() << "current directory" << directory << "has changed";
        this->populate();
    }
}

/**
 * @brief FileBrowser::populate
 */
void FileBrowser::populate() {
    // populate views
    this->ui->listView->model()->populate();
    this->ui->tableView->model()->populate();
}

/**
 * @brief FileBrowser::~FileBrowser
 */
FileBrowser::~FileBrowser() {
    delete ui;

    // sever signals/slots connections
    this->disconnect( this->ui->listView->selectionModel(), SIGNAL( selectionChanged( QItemSelection, QItemSelection )));
    this->disconnect( this->ui->tableView->selectionModel(), SIGNAL( selectionChanged( QItemSelection, QItemSelection )));
    this->disconnect( this->actionViewGrid, SIGNAL( triggered( bool )));
    this->disconnect( this->actionViewList, SIGNAL( triggered( bool )));
    this->disconnect( this->actionViewDetails, SIGNAL( triggered( bool )));
    this->disconnect( this->historyManager(), SIGNAL( changed()));
    this->disconnect( &pathUtils.watcher, SIGNAL( directoryChanged( QString )));

    // view mode related
    this->viewModeMenu->deleteLater();
    this->menuStyle->deleteLater();
    this->actionViewGrid->deleteLater();
    this->actionViewList->deleteLater();
    this->actionViewDetails->deleteLater();

    // delete objects
    this->m_historyManager->deleteLater();
}

/**
 * @brief FileBrowser::on_actionBookmarks_toggled
 * @param toggled
 */
void FileBrowser::on_actionBookmarks_toggled( bool toggled ) {
    if ( this->parentWindow == nullptr )
        return;

    /*  if ( toggled )
        this->parentWindow->showBookmarkDock();
    else
        this->parentWindow->hideBookmarkDock();

    Variable::setValue( "mainWindow/bookmarkPanelVisible", toggled );*/
}

/**
 * @brief FileBrowser::on_actionInfo_toggled
 * @param toggled
 */
void FileBrowser::on_actionInfo_toggled( bool toggled ) {
    if ( toggled )
        this->ui->dockInfo->show();
    else
        this->ui->dockInfo->close();

    Variable::setValue( "mainWindow/infoPanelVisible", toggled );

}

/**
 * @brief FileBrowser::on_actionClose_triggered
 */
void FileBrowser::on_actionClose_triggered() {
    this->parentWindow->close();
}

/**
 * @brief FileBrowser::on_actionClose_triggered
 */
void FileBrowser::on_actionMaximize_triggered() {
    if ( this->parentWindow->isMaximized())
        this->parentWindow->showNormal();
    else
        this->parentWindow->showMaximized();
}

/**
 * @brief FileBrowser::on_actionMinimize_triggered
 */
void FileBrowser::on_actionMinimize_triggered() {
    this->parentWindow->showMinimized();
}

/**
 * @brief FileBrowser::updateInfoPanel
 */
void FileBrowser::updateInfoPanel() {
    ContainerModel *model;
    Entry *entry;
    QPixmap pixmap;
    QString fileName, typeString, sizeString;

    if ( this->ui->stackedWidget->currentIndex() == 0 )
        model = this->ui->listView->model();
    else
        model = this->ui->tableView->model();

    // FIXME/TODO: update info panel on mime type detection?
    if ( model->selectionList.isEmpty()) {
        QFileInfo info( PathUtils::toWindowsPath( pathUtils.currentPath ));

        QMimeDatabase mdb;
        QDir directory( info.absoluteFilePath());
        QMimeType mimeType;

        mimeType = mdb.mimeTypeForFile( info );
        pixmap = m.pixmapCache->pixmap( mimeType.iconName(), 64 );
        fileName = info.fileName();
        typeString = mimeType.iconName();
        sizeString = this->tr( "%1 items" ).arg( directory.entryList( QDir::NoDotAndDotDot | QDir::AllEntries, QDir::IgnoreCase | QDir::DirsFirst ).count());
    } else {
        if ( model->selectionList.count() == 1 || ( model->container() == ContainerModel::TableContainer && model->selectionList.count() == 4 )) {
            entry = model->selectionList.first();

            if ( entry == nullptr )
                return;

            if ( entry->type() == Entry::Thumbnail || entry->type() == Entry::Executable ) {
                // get jumbo icon
                if ( entry->type() == Entry::Executable ) {
                    bool ok;
                    pixmap = Worker::extractPixmap( entry->path(), ok, true );

                    if ( !ok )
                        pixmap = QPixmap();
                } else
                    pixmap = QPixmap( entry->path());

                if ( pixmap.isNull() || !pixmap.width())
                    pixmap = entry->iconPixmap();

                // fast resize twice the size of info panel
                if ( pixmap.width() > this->ui->dockInfo->width() * 2 )
                    pixmap = pixmap.scaledToWidth( this->ui->dockInfo->width() * 2, Qt::FastTransformation );
            } else
                pixmap = m.pixmapCache->pixmap( entry->iconName(), 64 );

            fileName = entry->alias();
            typeString = entry->mimeType().iconName();
            sizeString = TextUtils::sizeToText( entry->info().size());
        } else {
            pixmap = m.pixmapCache->pixmap( "document-multiple", 64 );
            // TODO: use COLUMN COUNT as global constant
            if ( model->container() == ContainerModel::ListContainer )
                fileName = this->tr( "%1 files" ).arg( model->selectionList.count());
            else
                fileName = this->tr( "%1 files" ).arg( model->selectionList.count() / 4 );

            typeString = this->tr( "Multiple files" );

            quint64 bytes = 0;
            foreach ( entry, model->selectionList )
                bytes += entry->info().size();

            if ( model->container() == ContainerModel::ListContainer )
                sizeString = TextUtils::sizeToText( bytes );
            else
                sizeString = TextUtils::sizeToText( bytes / 4 );
        }
    }

    this->ui->infoPixmap->setPixmap( pixmap );
    this->ui->editFileName->setText( fileName );
    this->ui->valueType->setText( typeString );
    this->ui->valueSize->setText( sizeString );
}

/**
 * @brief FileBrowser::setGridView
 */
void FileBrowser::setGridView() {
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
    //this->ui->horizontalSlider->setEnabled( true );

    Variable::setValue( "mainWindow/viewMode", IconMode );
}

/**
 * @brief FileBrowser::setListView
 */
void FileBrowser::setListView() {
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
    //this->ui->horizontalSlider->setEnabled( true );

    Variable::setValue( "mainWindow/viewMode", ListMode );
}

/**
 * @brief FileBrowser::setDetailView
 */
void FileBrowser::setDetailView() {
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
    //this->ui->horizontalSlider->setEnabled( false );

    Variable::setValue( "mainWindow/viewMode", DetailMode );
}

/**
 * @brief FileBrowser::on_actionViewMode_triggered
 */
void FileBrowser::on_actionViewMode_triggered() {
    this->ui->actionViewMode->menu()->exec( QCursor::pos());
}

/**
 * @brief FileBrowser::setCurrentPath
 * @param path
 * @param saveToHistory
 */
void FileBrowser::setCurrentPath( const QString &path, bool saveToHistory ) {
    QDir directory;
    QString windowsPath, unixPath;

    // clear all selections
    if ( QString::compare( pathUtils.currentPath, PathUtils::toUnixPath( path ))) {
        this->ui->tableView->model()->selectionList.clear();
        this->ui->listView->model()->selectionList.clear();
    }

    // determine directory type
    switch ( SpecialDirectory::pathToType( path )) {
    case SpecialDirectory::Root:
    case SpecialDirectory::Trash:
    case SpecialDirectory::Bookmarks:
        pathUtils.currentPath = path;

        // populate views
        this->populate();

        // disable up button since we're at root already
        this->ui->actionUp->setDisabled( true );
        break;

    case SpecialDirectory::General:
    {
        // enable up button
        this->ui->actionUp->setEnabled( true );

        // convert path to windows path
        windowsPath = PathUtils::toWindowsPath( path );

        // remove previous path from watch
        if ( !PathUtils::toWindowsPath( pathUtils.currentPath ).isEmpty())
            pathUtils.watcher.removePath( PathUtils::toWindowsPath( pathUtils.currentPath ));

        // add the new path to watch
        if ( !path.isEmpty())
            pathUtils.watcher.addPath( path );

        // handle symlinks
        QFileInfo info( windowsPath );
        if ( info.isSymLink()) {
            QFileInfo target( info.symLinkTarget());
            if ( !target.isSymLink())
                windowsPath = target.absoluteFilePath();
        }
        directory.cd( windowsPath );

        // notify on non-existant directories
        if ( !directory.exists( windowsPath )) {
            m.notifications()->push( NotificationPanel::Error, this->tr( "Error" ), this->tr( "Path does not exist" ));
            return;
        }

        // convert path back to unix path
        unixPath = PathUtils::toUnixPath( windowsPath );

        // update navigation bar
        this->ui->navigationBar->setPath( unixPath );

        // no use checking the same path
        if ( !QString::compare( pathUtils.currentPath, unixPath ))
            return;

        // store current path
        pathUtils.currentPath = unixPath;

        // populate views
        this->populate();

        // update info panel
        this->updateInfoPanel();
    }
        break;

    case SpecialDirectory::NoType:
    default:
        return;
    }

    // save path to history
    if ( saveToHistory )
        this->historyManager()->addItem( pathUtils.currentPath );
}

/**
 * @brief FileBrowser::on_actionUp_triggered
 */
void FileBrowser::on_actionUp_triggered() {
    QDir directory( PathUtils::toWindowsPath( pathUtils.currentPath ));

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
 * @brief FileBrowser::on_actionBack_triggered
 */
void FileBrowser::on_actionBack_triggered() {
    this->historyManager()->back();
    this->setCurrentPath( this->historyManager()->current().toString(), false );
}

/**
 * @brief FileBrowser::on_actionForward_triggered
 */
void FileBrowser::on_actionForward_triggered() {
    this->historyManager()->forward();
    this->setCurrentPath( this->historyManager()->current().toString(), false );
}

/**
 * @brief FileBrowser::checkHistoryPosition
 */
void FileBrowser::checkHistoryPosition() {
    this->ui->actionBack->setEnabled( this->historyManager()->isBackEnabled());
    this->ui->actionForward->setEnabled( this->historyManager()->isForwardEnabled());
}
