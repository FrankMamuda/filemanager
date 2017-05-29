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

#pragma once

//
// includes
//
#include <QMainWindow>
#include <QDir>
#include <QCloseEvent>
#include <QComboBox>
#include <QProxyStyle>
#include "ui_mainwindow.h"
#include <QPaintEvent>

//
// classes
//
class MainWindow;
class ContainerModel;
class NotificationPanel;
class History;
class FileBrowser;

//
// namespace: Ui
//
namespace Ui {
class MainWindow;
static const int BorderWidth = 8;
static const int MouseGrabAreas = 8;
static const QString lightIconTheme = "breeze";
static const QString darkIconTheme = "breeze-dark";
}

/**
 * @brief The MainWindow class
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    // constructor/destructor
    explicit MainWindow( QWidget *parent = 0 );
    ~MainWindow();

    /**
     * @brief The IconScale enum
     */
    enum IconScale {
        NoScale = 0,
        Tiny,
        Small,
        Medium,
        Large
    };
    Q_ENUMS( IconScale )

    // grab areas for frameless resizing
    enum Areas {
        NoArea = -1,
        TopLeft,
        Top,
        TopRight,
        Right,
        BottomRight,
        Bottom,
        BottomLeft,
        Left
    };
    Q_ENUMS( Areas )

    // frameless gestures
    enum Gestures {
        NoGesture = -1,
        Drag,
        Resize
    };
    Q_ENUMS( Gestures )

protected:
    void resizeEvent( QResizeEvent * );
    void closeEvent( QCloseEvent * );
    bool eventFilter( QObject *object, QEvent *event );

public slots:
    void showBookmarkDock();
    void hideBookmarkDock();
    void setCurrentPath( const QString &path = QDir::currentPath(), bool saveToHistory = true );

private slots:
    // ui slots
    void on_notificationInfo_clicked();
    void on_horizontalSlider_valueChanged( int value );

    // custom slots
    void removeFrame();
    void makeGrabAreas();

private:
    Ui::MainWindow *ui;

    // variables for frameless resizing
    QPoint mousePos;
    Gestures gesture;
    Areas currentGrabArea;
    QRect grabAreas[Ui::MouseGrabAreas];

    // file browser
    FileBrowser *fileBrowser;
};

Q_DECLARE_METATYPE( MainWindow::IconScale )
