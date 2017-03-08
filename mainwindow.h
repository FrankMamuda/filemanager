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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//
// includes
//
#include <QMainWindow>
#include <QDir>
#include <QCloseEvent>
#include <QComboBox>
#include <QProxyStyle>
#include "ui_mainwindow.h"

//
// classes
//
class MainWindow;
class ContainerModel;
class NotificationPanel;
class History;

//
// namespace: Ui
//
namespace Ui {
class MainWindow;
}

/**
 * @brief The MenuStyle class
 */
class MenuStyle : public QProxyStyle {
public:
    int pixelMetric( PixelMetric pm, const QStyleOption *option = 0, const QWidget *widget = 0 ) const{
        if ( pm == QStyle::PM_SmallIconSize )
            return 32;
        else
            return QProxyStyle::pixelMetric( pm, option, widget );
    }
};

/**
 * @brief The MainWindow class
 */
class MainWindow : public QMainWindow {
    Q_OBJECT
    Q_PROPERTY( QString currentPath READ currentPath WRITE setCurrentPath )

public:
    // constructor/destructor
    explicit MainWindow( QWidget *parent = 0 );
    ~MainWindow();

    // path related
    QString currentPath() const { return this->m_currentPath; }
    History *historyManager() const { return this->m_historyManager; }

    enum IconScale {
        NoScale = 0,
        Tiny,
        Small,
        Medium,
        Large
    };
    Q_ENUMS( IconScale )

    // view modes - icon grid, list, detail table
    enum ViewModes {
        NoMode = -1,
        IconMode,
        ListMode,
        DetailMode
    };
    Q_ENUMS( ViewModes )

protected:
    void resizeEvent( QResizeEvent * );
    void closeEvent( QCloseEvent * );

public slots:
    void setCurrentPath( const QString &path = QDir::currentPath(), bool saveToHistory = true );

private slots:
    // ui slots
    void on_actionUp_triggered();
    void on_actionBack_triggered();
    void on_actionForward_triggered();
    void on_actionViewMode_triggered();
    void on_notificationInfo_clicked();
    void on_pathEdit_returnPressed();
    void on_horizontalSlider_valueChanged( int value );
    void on_actionBookmarks_toggled( bool toggled );
    void on_actionInfo_toggled( bool toggled );

    // custom slots
    void setGridView();
    void setListView();
    void setDetailView();
    void checkHistoryPosition();
    void updateInfoPanel();

private:
    Ui::MainWindow *ui;

    // path related
    QString m_currentPath;
    History *m_historyManager;

    // view mode widgets
    MenuStyle *menuStyle;
    QAction *actionViewGrid;
    QAction *actionViewList;
    QAction *actionViewDetails;
    QMenu *viewModeMenu;
};

Q_DECLARE_METATYPE( MainWindow::IconScale )
Q_DECLARE_METATYPE( MainWindow::ViewModes )

#endif // MAINWINDOW_H
