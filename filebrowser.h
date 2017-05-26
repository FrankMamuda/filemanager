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
#include <QProxyStyle>
#include <QMenu>
#include <QDir>

/**
 * @brief The Ui namespace
 */
namespace Ui {
class FileBrowser;
}

//
// classes
//
class MainWindow;
class History;

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
 * @brief The FileBrowser class
 */
class FileBrowser : public QMainWindow {
    Q_OBJECT

public:
    // view modes - icon grid, list, detail table
    enum ViewModes {
        NoMode = -1,
        IconMode,
        ListMode,
        DetailMode
    };
    Q_ENUMS( ViewModes )

    explicit FileBrowser( QWidget *parent = 0 );
    ~FileBrowser();

    // parent window
    MainWindow *parentWindow;

    History *historyManager() const { return this->m_historyManager; }

public slots:
    void updateInfoPanel();
    void setCurrentPath( const QString &path = QDir::currentPath(), bool saveToHistory = true );

private slots:
    void on_actionBookmarks_toggled( bool toggled );
    void on_actionInfo_toggled( bool toggled );
    void on_actionClose_triggered();
    void on_actionMaximize_triggered();
    void on_actionMinimize_triggered();
    void on_actionViewMode_triggered();
    void on_actionUp_triggered();
    void on_actionBack_triggered();
    void on_actionForward_triggered();
    void on_pathEdit_returnPressed();

    // custom slots
    void setGridView();
    void setListView();
    void setDetailView();
    void checkHistoryPosition();
    void setupToolBar();
    void setupFrameBar();
    void setupNavigationBar();
    void setupViewModes();

private:
    Ui::FileBrowser *ui;

    // view mode widgets
    MenuStyle *menuStyle;
    QAction *actionViewGrid;
    QAction *actionViewList;
    QAction *actionViewDetails;
    QMenu *viewModeMenu;

    // path related
    History *m_historyManager;
};

Q_DECLARE_METATYPE( FileBrowser::ViewModes )
