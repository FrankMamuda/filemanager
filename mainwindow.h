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
#include <QComboBox>
#include <QProxyStyle>
#include "ui_mainwindow.h"

//
// classes
//
class MainWindow;
class ContainerModel;

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
    int pixelMetric( PixelMetric m, const QStyleOption *option = 0, const QWidget *widget = 0 ) const{
        if ( m == QStyle::PM_SmallIconSize )
            return 32;
        else
            return QProxyStyle::pixelMetric( m, option, widget );
    }
};

/**
 * @brief The MainWindow class
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow( QWidget *parent = 0 );
    ~MainWindow();
    QString currentPath() const { return this->m_currentPath; }
    Ui::MainWindow *ui;
    int historyPosition() const { return this->m_historyPosition; }

    enum IconScale {
        NoScale = 0,
        Tiny,
        Small,
        Medium,
        Large
    };
    Q_ENUMS( IconScale )

    enum ViewModes {
        NoMode = -1,
        IconMode,
        ListMode,
        DetailMode
    };
    Q_ENUMS( ViewModes )

protected:
    virtual void resizeEvent( QResizeEvent * );

public slots:
    void setCurrentPath( const QString &path = QDir::currentPath());

private slots:
    void on_actionUp_triggered();
    void on_pathEdit_returnPressed();
    void on_horizontalSlider_valueChanged(int value);
    void on_actionBack_triggered();
    void on_actionForward_triggered();
    void checkHistoryPosition();
    void on_actionViewMode_triggered();
    void setGridView();
    void setListView();
    void setDetailView();
    void addToHistory( const QString &path );

private:
    QString m_currentPath;
    QStringList history;
    int m_historyPosition;

    // view mode widgets
    MenuStyle *menuStyle;
    QAction *actionViewGrid;
    QAction *actionViewList;
    QAction *actionViewDetails;
    QMenu *viewModeMenu;
};

#endif // MAINWINDOW_H
