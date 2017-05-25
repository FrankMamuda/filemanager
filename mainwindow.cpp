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
#include "pathutils.h"
#include "entry.h"
#include "containermodel.h"
#include <QMenu>
#include "variable.h"
#include "main.h"
#include "bookmark.h"
#include "notificationpanel.h"
#include "history.h"
#include <QMimeDatabase>
#include "textutils.h"
#include "pixmapcache.h"
#include "worker.h"
#include <windows.h>
#include <dwmapi.h>

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
    + thumbnail cache and cleanup
      (cache preview size limit)
      (cache preview store by md5 hash?)
    text previews just read the first few bytes
    compressed file browsing
    on first run match:
        HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.EXT\OpenWithList
        and math
        HKEY_CLASSES_ROOT\Applications\APPNAME.exe\shell\open\command
        to
        with mime type database and create a list of default programs
    implement QFileSystemWatcher for file updates
    tooltips
    + fix mimetype deletection performance regression
    + fix crash on app exit while detecting mimetypes
    - notifications in categories - warning, error, info in status bar - not implementing this
    + extract windows icon from executables
    + cache to disk:
        mimetype
        thumbnail (+extracted icon)
        (introduce a reasonable file size limit)
            larger than that goes through non-content matching
  */

/**
 * @brief MainWindow::MainWindow
 * @param parent
 */
MainWindow::MainWindow( QWidget *parent ) : QMainWindow( parent ), ui( new Ui::MainWindow ), m_currentPath( QDir::currentPath()), gesture( NoGesture ), currentGrabArea( NoArea ) {
    ViewModes viewMode;
    QSize size;

    // set up ui
    this->ui->setupUi( this );

    // set up selection models
    this->ui->listView->setSelectionModel( this->ui->listView->model()->selectionModel());
    this->ui->tableView->setSelectionModel( this->ui->tableView->model()->selectionModel());

    // style docks
    this->setStyleSheet( "QDockWidget::title { background-color: transparent; text-align: center; }" );
    //this->ui->dockPath->setTitleBarWidget( new QWidget());
    this->ui->dockStatus->setTitleBarWidget( new QWidget());

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

    // get geometry
    Variable::add( "mainWindow/size", this->size());
    size = Variable::value<QSize>( "mainWindow/size" );
    if ( size != this->size())
        this->resize( size );

    // set icons here (some bug resets icons, when defined in form)
    this->ui->actionBack->setIcon( pixmapCache.fromTheme( /*QIcon::fromTheme(*/ "go-previous" ));
    this->ui->actionUp->setIcon( pixmapCache.fromTheme( /*QIcon::fromTheme(*/ "go-up" ));
    this->ui->actionForward->setIcon( pixmapCache.fromTheme( /*QIcon::fromTheme(*/ "go-next" ));

    // setup view mode list
    this->viewModeMenu = new QMenu();
    this->menuStyle = new MenuStyle;
    this->viewModeMenu->setStyle( this->menuStyle );
    this->actionViewGrid = new QAction( pixmapCache.fromTheme( /*QIcon::fromTheme(*/ "preferences-desktop-icons" ), this->tr( "Grid view" ));
    this->actionViewList = new QAction( pixmapCache.fromTheme( /*QIcon::fromTheme(*/ "view-media-playlist" ), this->tr( "List view" ));
    this->actionViewDetails = new QAction( pixmapCache.fromTheme( /*QIcon::fromTheme(*/ "x-office-spreadsheet" ), this->tr( "Detail view" ));
    this->viewModeMenu->addAction( this->actionViewGrid );
    this->viewModeMenu->addAction( this->actionViewList );
    this->viewModeMenu->addAction( this->actionViewDetails );
    this->ui->actionViewMode->setMenu( this->viewModeMenu );
    this->ui->actionViewMode->setIcon( pixmapCache.fromTheme( /*QIcon::fromTheme(*/ "preferences-system-windows-actions" ));
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
    this->ui->notificationInfo->setIcon( pixmapCache.fromTheme( /*QIcon::fromTheme(*/ "dialog-information" ));

    // handle actions
    this->ui->actionBookmarks->setIcon( pixmapCache.fromTheme( /*QIcon::fromTheme(*/ "folder-bookmark" ));
    this->ui->actionInfo->setIcon( pixmapCache.fromTheme( /*QIcon::fromTheme(*/ "dialog-information" ));
    Variable::add( "mainWindow/bookmarkPanelVisible", true );
    Variable::add( "mainWindow/infoPanelVisible", true );

    // FIXME: a little messy
    this->ui->actionBookmarks->setChecked( Variable::isEnabled( "mainWindow/bookmarkPanelVisible" ));
    this->ui->actionInfo->setChecked( Variable::isEnabled( "mainWindow/infoPanelVisible" ));
    this->on_actionBookmarks_toggled( Variable::isEnabled( "mainWindow/bookmarkPanelVisible" ));
    this->on_actionInfo_toggled( Variable::isEnabled( "mainWindow/infoPanelVisible" ));

    // connect info panel for updates
    this->connect( this->ui->listView->selectionModel(), SIGNAL( selectionChanged( QItemSelection, QItemSelection )), this, SLOT( updateInfoPanel()));
    this->connect( this->ui->tableView->selectionModel(), SIGNAL( selectionChanged( QItemSelection, QItemSelection )), this, SLOT( updateInfoPanel()));



    // ==================================================
    // NOTE: lots of UI hacks for desired view
    // FIXME: delete!
    QMainWindow *window = new QMainWindow();
    window->addToolBar( this->ui->mainToolBar );
    window->addToolBarBreak();
    window->addToolBar( this->ui->navigationToolbar );
    window->setContentsMargins( 0, 0, 0, 0 );
    //  window->addDockWidget( Qt::RightDockWidgetArea, this->ui->dockInfo, Qt::Vertical );

    this->ui->navigationToolbar->addWidget( this->ui->pathEdit );
    this->ui->mainToolBar->setStyleSheet( "QToolBar { background: transparent; border: none; }" );
    this->ui->navigationToolbar->setStyleSheet( "QToolBar { background: transparent; border: none; }" );
    this->ui->dockPath->setTitleBarWidget( window );
    this->ui->dockPath->setMinimumHeight( 64 );

    // make sidebar dark
    this->ui->dockBookmarks->setStyleSheet( "QDockWidget { background: #353535; }" );
    this->ui->dockBookmarksContents->setStyleSheet( "QWidget { background: #353535; }" );
    this->ui->sideView->setStyleSheet( "QListView { background: #353535; color: #8c8c8c; } QListWidget::item { color: #8c8c8c; }" );

    this->ui->dockBookmarks->setTitleBarWidget( new QWidget());
    this->ui->dockInfo->setTitleBarWidget( new QWidget());


    //this->ui->sideView->setStyleSheet( "QListView { color: #8c8c8c; background: transparent; } " );
    //this->ui->sideView->setAutoFillBackground( false );
    this->connect( this->ui->actionClose, SIGNAL( triggered( bool )), qApp, SLOT( quit()));

    this->ui->dockBookmarks->installEventFilter( this );
    this->ui->dockBookmarksContents->installEventFilter( this );
    this->ui->sideView->installEventFilter( this );

    this->setWindowIcon( pixmapCache.fromTheme( "document-open-folder" ));

    // ==================================================

    // remove frame
    this->removeFrame();

    // update info panel
    this->updateInfoPanel();
}

/**
 * @brief MainWindow::eventFilter
 * @param object
 * @param event
 * @return
 */
bool MainWindow::eventFilter( QObject *object, QEvent *event ) {
    QMouseEvent *mouseEvent;
    int y;

    // filter mouse events
    if (( event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease ||
          event->type() == QEvent::MouseMove || event->type() == QEvent::HoverMove ||
          event->type() == QEvent::Leave )
           && this->cursor().shape() != Qt::SplitHCursor // HACK: allow resizing of dock widgets
            ) {

        // get mouse event
        mouseEvent = static_cast<QMouseEvent*>( event );

        // test mouse origin if needed
        if ( this->gesture == NoGesture ) {
            this->currentGrabArea = NoArea;
            for ( y = 0; y < Ui::MouseGrabAreas; y++ ) {
                if ( this->grabAreas[y].contains( mouseEvent->pos())) {
                    this->currentGrabArea = static_cast<Areas>( y );
                    break;
                }
            }
        }

        // change cursor shape if needed
        if ( this->gesture == NoGesture ) {
            Qt::CursorShape shape;

            if ( this->currentGrabArea == Top || this->currentGrabArea == Bottom )
                shape = Qt::SizeVerCursor;
            else if ( this->currentGrabArea == Left || this->currentGrabArea == Right )
                shape = Qt::SizeHorCursor;
            else if ( this->currentGrabArea == TopLeft || this->currentGrabArea == BottomRight )
                shape = Qt::SizeFDiagCursor;
            else if ( this->currentGrabArea == TopRight || this->currentGrabArea == BottomLeft )
                shape = Qt::SizeBDiagCursor;
            else if ( this->currentGrabArea == NoArea )
                shape = Qt::ArrowCursor;

            if ( this->cursor().shape() != shape )
                this->setCursor( QCursor( shape ));
        }

        // handle events
        switch ( event->type()) {
        case QEvent::MouseButtonPress:
            if ( mouseEvent->button() == Qt::LeftButton ) {
                // store mouse position
                this->mousePos = mouseEvent->pos();

                // determine gesture
                if ( this->currentGrabArea == NoArea )
                    this->gesture = Drag;
                else
                    this->gesture = Resize;

                return true;
            }
            break;

        case QEvent::MouseButtonRelease:
            if ( mouseEvent->button() == Qt::LeftButton ) {
                this->gesture = NoGesture;
                return true;
            }
            break;

        case QEvent::MouseMove:
            if ( this->gesture == Drag ) {
                this->move( mouseEvent->globalPos().x() - this->mousePos.x(), mouseEvent->globalPos().y() - this->mousePos.y());
                return true;
            } else if ( this->gesture == Resize ) {
                QRect updatedGeometry;

                updatedGeometry = this->geometry();

                switch ( this->currentGrabArea ) {
                case TopLeft:
                    updatedGeometry.setTopLeft( mouseEvent->globalPos());
                    break;

                case Top:
                    updatedGeometry.setTop( mouseEvent->globalPos().y());
                    break;

                case TopRight:
                    updatedGeometry.setTopRight( mouseEvent->globalPos());
                    break;

                case Right:
                    updatedGeometry.setRight( mouseEvent->globalPos().x());
                    break;

                case BottomRight:
                    updatedGeometry.setBottomRight( mouseEvent->globalPos());
                    break;

                case Bottom:
                    updatedGeometry.setBottom( mouseEvent->globalPos().y());
                    break;

                case BottomLeft:
                    updatedGeometry.setBottomLeft( mouseEvent->globalPos());
                    break;

                case Left:
                    updatedGeometry.setLeft( mouseEvent->globalPos().x());
                    break;

                default:
                    break;
                }

                // respect minimum width
                if ( updatedGeometry.width() < this->minimumWidth())
                    updatedGeometry.setLeft( this->geometry().x());

                // respect minimum height
                if ( updatedGeometry.height() < this->minimumHeight())
                    updatedGeometry.setTop( this->geometry().y());

                // set the new geometry
                this->setGeometry( updatedGeometry );

                // rebuild areas
                this->makeGrabAreas();

                // return success
                return true;
            }
            break;

        default:
            break;
        }
    }

    // other events are handled normally
    return QMainWindow::eventFilter( object, event );
}

/**
 * @brief MainWindow::removeFrame
 */
void MainWindow::removeFrame() {
    MARGINS margins = { -1, -1, -1, -1 };
    int flag = DWMNCRP_ENABLED;
    HWND hwnd;

    // remove frame, detect leave event
    this->setWindowFlags( Qt::FramelessWindowHint );
    this->setAttribute( Qt::WA_Hover );

    // enable shadow
    hwnd = reinterpret_cast<HWND>( this->winId());
    DwmSetWindowAttribute( hwnd, 2, &flag, 4 );
    DwmExtendFrameIntoClientArea( hwnd, &margins );

    // enable mouse tracking
    this->setMouseTracking( true );

    // filter events
    this->installEventFilter( this );

    // make mouse grab areas
    this->makeGrabAreas();
}

/**
 * @brief MainWindow::makeAreas
 */
void MainWindow::makeGrabAreas() {
    this->grabAreas[TopLeft] = QRect( 0, 0, Ui::BorderWidth, Ui::BorderWidth );
    this->grabAreas[Top] = QRect( Ui::BorderWidth, 0, this->width() - Ui::BorderWidth * 2, Ui::BorderWidth );
    this->grabAreas[TopRight] = QRect( this->width() - Ui::BorderWidth, 0, Ui::BorderWidth, Ui::BorderWidth );
    this->grabAreas[Right] = QRect( this->width() - Ui::BorderWidth, Ui::BorderWidth, Ui::BorderWidth, this->height() - Ui::BorderWidth * 2 );
    this->grabAreas[BottomRight] = QRect( this->width() - Ui::BorderWidth, this->height() - Ui::BorderWidth, Ui::BorderWidth, Ui::BorderWidth );
    this->grabAreas[Bottom] = QRect( Ui::BorderWidth, this->height() - Ui::BorderWidth, this->width() - Ui::BorderWidth * 2, Ui::BorderWidth );
    this->grabAreas[BottomLeft] = QRect( 0, this->height() - Ui::BorderWidth, Ui::BorderWidth, Ui::BorderWidth );
    this->grabAreas[Left] = QRect( 0, Ui::BorderWidth, Ui::BorderWidth, this->height() - Ui::BorderWidth * 2 );
}

/**
 * @brief MainWindow::~MainWindow
 */
MainWindow::~MainWindow() {
    // get rid of ui
    delete ui;

    // sever connections
    this->disconnect( this->historyManager(), SIGNAL( changed()));
    this->disconnect( this->ui->listView->selectionModel(), SIGNAL( selectionChanged( QItemSelection, QItemSelection )));
    this->disconnect( this->ui->tableView->selectionModel(), SIGNAL( selectionChanged( QItemSelection, QItemSelection )));
    this->disconnect( this->actionViewGrid, SIGNAL( triggered( bool )));
    this->disconnect( this->actionViewList, SIGNAL( triggered( bool )));
    this->disconnect( this->actionViewDetails, SIGNAL( triggered( bool )));

    // delete objects
    this->m_historyManager->deleteLater();
    this->viewModeMenu->deleteLater();
    this->menuStyle->deleteLater();
    this->actionViewGrid->deleteLater();
    this->actionViewList->deleteLater();
    this->actionViewDetails->deleteLater();
}

/**
 * @brief MainWindow::resizeEvent
 */
void MainWindow::resizeEvent( QResizeEvent *e ) {
    QMainWindow::resizeEvent( e );
    this->ui->dockPath->setMaximumHeight( this->ui->dockPath->geometry().height());
    this->ui->dockStatus->setMaximumHeight( this->ui->dockStatus->geometry().height());
    this->updateInfoPanel();
}

/**
 * @brief MainWindow::updateInfoPanel
 */
void MainWindow::updateInfoPanel() {
    ContainerModel *model;
    Entry *entry;
    QPixmap pixmap;
    QString fileName, typeString, sizeString;

    if ( this->ui->stackedWidget->currentIndex() == 0 )
        model = this->ui->listView->model();
    else
        model = this->ui->tableView->model();

    if ( model->mode() != ContainerModel::FileMode )
        return;

    // FIXME/TODO: update info panel on mime type detection?
    if ( model->selectionList.isEmpty()) {
        QFileInfo info( PathUtils::toWindowsPath( this->currentPath()));

        QMimeDatabase mdb;
        QDir directory( info.absoluteFilePath());
        QMimeType mimeType;

        mimeType = mdb.mimeTypeForFile( info );
        pixmap = pixmapCache.pixmap( mimeType.iconName(), 64 );
        fileName = info.fileName();
        typeString = mimeType.iconName();
        sizeString = this->tr( "%1 items" ).arg( directory.entryList( QDir::NoDotAndDotDot | QDir::AllEntries, QDir::IgnoreCase | QDir::DirsFirst ).count());
    } else {
        if ( model->selectionList.count() == 1 || ( model->container() == ContainerModel::TableContainer && model->selectionList.count() == 4 )) {
            entry = model->selectionList.first();

            if ( entry == NULL )
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
                pixmap = pixmapCache.pixmap( entry->iconName(), 64 );

            fileName = entry->alias();
            typeString = entry->mimeType().iconName();
            sizeString = TextUtils::sizeToText( entry->info().size());
        } else {
            pixmap = pixmapCache.pixmap( "document-multiple", 64 );
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
 * @brief MainWindow::closeEvent
 */
void MainWindow::closeEvent( QCloseEvent *e ) {
    Variable::setValue( "mainWindow/size", this->size());
    QMainWindow::closeEvent( e );
}

// TODO: make static
/**
 * @brief MainWindow::setCurrentPath
 * @param path
 * @param saveToHistory
 */
void MainWindow::setCurrentPath( const QString &path, bool saveToHistory ) {
    QDir directory;
    QString windowsPath, unixPath;

    this->ui->tableView->model()->selectionList.clear();
    this->ui->listView->model()->selectionList.clear();


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
            m.notifications()->push( NotificationPanel::Error, this->tr( "Error" ), this->tr( "Path does not exist" ));
            return;
        }

        unixPath = PathUtils::toUnixPath( windowsPath );
        this->m_currentPath = unixPath;
        this->ui->pathEdit->setText( unixPath );

        this->ui->listView->model()->setMode( ContainerModel::FileMode );
        this->ui->tableView->model()->setMode( ContainerModel::FileMode );
    }

    this->ui->statusBar->setText( this->tr( "%1 items" ).arg( this->ui->listView->model()->numItems()));

    this->updateInfoPanel();

    if ( saveToHistory )
        this->historyManager()->addItem( this->m_currentPath );
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
    this->setCurrentPath( this->historyManager()->current().toString(), false );
}

/**
 * @brief MainWindow::on_actionForward_triggered
 */
void MainWindow::on_actionForward_triggered() {
    this->historyManager()->forward();
    this->setCurrentPath( this->historyManager()->current().toString(), false );
}

/**
 * @brief MainWindow::checkHistoryPosition
 */
void MainWindow::checkHistoryPosition() {
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
 * @brief MainWindow::on_notificationInfo_clicked
 */
void MainWindow::on_notificationInfo_clicked() {
    m.notifications()->raise();
}

/**
 * @brief MainWindow::on_actionBookmarks_toggled
 * @param arg1
 */
void MainWindow::on_actionBookmarks_toggled( bool toggled ) {
    if ( toggled )
        this->ui->dockBookmarks->show();
    else
        this->ui->dockBookmarks->close();

    Variable::setValue( "mainWindow/bookmarkPanelVisible", toggled );
}

/**
 * @brief MainWindow::on_actionInfo_toggled
 * @param arg1
 */
void MainWindow::on_actionInfo_toggled( bool toggled ) {
    if ( toggled )
        this->ui->dockInfo->show();
    else
        this->ui->dockInfo->close();

    Variable::setValue( "mainWindow/infoPanelVisible", toggled );
}
