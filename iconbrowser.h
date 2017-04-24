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

#ifndef ICONBROWSER_H
#define ICONBROWSER_H

//
// includes
//
#include <QDialog>
#include <QResizeEvent>
#include <QModelIndex>

//
// classes
//
class IconModel;

/**
 * The Ui namespace
 */
namespace Ui {
class IconBrowser;
}

/**
 * @brief The IconBrowser class
 */
class IconBrowser : public QDialog {
    Q_OBJECT

public:
    explicit IconBrowser( QWidget *parent = 0 );
    ~IconBrowser();
    QPixmap selectedPixmap() const;

private slots:
    void on_iconView_clicked( const QModelIndex &index );

    void on_pushButton_clicked();

protected:
    void resizeEvent( QResizeEvent *e );

private:
    Ui::IconBrowser *ui;
    IconModel *iconModel;
    QModelIndex currentIndex;
};

#endif // ICONBROWSER_H
