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
#ifdef Q_OS_WIN32
#include <windows.h>
#include <dwmapi.h>
#endif
#include "filebrowser.h"
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
MainWindow::MainWindow( QWidget *parent ) : QMainWindow( parent ), ui( new Ui::MainWindow ), gesture( NoGesture ), currentGrabArea( NoArea ), fileBrowser( new FileBrowser( this )) {
    QSize size;

    // set up ui
    this->ui->setupUi( this );

    // style docks
    this->setStyleSheet( "QDockWidget::title { background-color: transparent; text-align: center; }" );

    // trigger size change
    Variable::add( "mainWindow/iconSize/sliderPosition", Medium );
    this->ui->horizontalSlider->setValue( Variable::integer( "mainWindow/iconSize/sliderPosition" ));
    this->on_horizontalSlider_valueChanged( Variable::integer( "mainWindow/iconSize/sliderPosition" ));

    // get geometry
    Variable::add( "mainWindow/size", this->size());
    size = Variable::value<QSize>( "mainWindow/size" );
    if ( size != this->size())
        this->resize( size );

    // notification icons
    this->ui->notificationInfo->setIcon( m.pixmapCache->icon( "dialog-information" ));

    // make sidebar dark
    this->ui->dockBookmarks->setStyleSheet( "QDockWidget { background: #353535; }" );
    this->ui->dockBookmarksContents->setStyleSheet( "QWidget { background: #353535; }" );
    this->ui->sideView->setStyleSheet( "QListView { background: #353535; color: #8c8c8c; } QListWidget::item { color: #8c8c8c; }" );
    this->ui->dockBookmarks->setTitleBarWidget( new QWidget( this ));
    this->ui->dockBookmarks->setVisible( Variable::isEnabled( "mainWindow/bookmarkPanelVisible" ));

    // add file browser widget
    this->fileBrowser->parentWindow = this;
    this->ui->centralLayout->addWidget( this->fileBrowser );

    // change window icon
    this->setWindowIcon( m.pixmapCache->icon( "document-open-folder" ));

    // remove frame
    this->removeFrame();

    // update info panel
    this->fileBrowser->updateInfoPanel();

    // connect bookmark toggle button
    this->connect( this->fileBrowser, SIGNAL( bookmarkPanelToggled( bool )), this, SLOT( bookmarkPanelToggled( bool )));
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
        if ( this->gesture == NoGesture && !this->isMaximized()) {
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

                // no drag when maximized
                if ( this->isMaximized())
                    return true;

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
 * @brief MainWindow::bookmarkPanelToggled
 * @param toggled
 */
void MainWindow::bookmarkPanelToggled( bool toggled ) {
    this->ui->dockBookmarks->setVisible( toggled );
}

/**
 * @brief MainWindow::setCurrentPath
 * @param path
 * @param saveToHistory
 */
void MainWindow::setCurrentPath( const QString &path, bool saveToHistory ) {
    this->fileBrowser->setCurrentPath( path, saveToHistory );
}

/**
 * @brief MainWindow::removeFrame
 */
void MainWindow::removeFrame() {
#ifdef Q_OS_WIN32
    MARGINS margins = { -1, -1, -1, -1 };
    int flag = DWMNCRP_ENABLED;
    HWND hwnd;
#endif

    // remove frame, detect leave event
    this->setWindowFlags( Qt::FramelessWindowHint );
    this->setAttribute( Qt::WA_Hover );

    // enable shadow
#ifdef Q_OS_WIN32
    hwnd = reinterpret_cast<HWND>( this->winId());
    DwmSetWindowAttribute( hwnd, 2, &flag, 4 );
    DwmExtendFrameIntoClientArea( hwnd, &margins );
#endif

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

    this->disconnect( this->fileBrowser, SIGNAL( bookmarkPanelToggled( bool )));
    this->fileBrowser->deleteLater();
}

/**
 * @brief MainWindow::resizeEvent
 */
void MainWindow::resizeEvent( QResizeEvent *e ) {
    QMainWindow::resizeEvent( e );
    this->fileBrowser->updateInfoPanel();
}

/**
 * @brief MainWindow::closeEvent
 */
void MainWindow::closeEvent( QCloseEvent *e ) {

    if ( !this->isMaximized())
        Variable::setValue( "mainWindow/size", this->size());

    QMainWindow::closeEvent( e );
}

/**
 * @brief MainWindow::on_horizontalSlider_valueChanged
 * @param value
 */
void MainWindow::on_horizontalSlider_valueChanged( int value ) {
    this->fileBrowser->horizontalSliderMoved( value );
}

/**
 * @brief MainWindow::on_notificationInfo_clicked
 */
void MainWindow::on_notificationInfo_clicked() {
    m.notifications()->raise();
}

