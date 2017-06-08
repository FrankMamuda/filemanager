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
#include <QWidget>
#include <QPushButton>
#include <QLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QResizeEvent>
#include <QDir>
#include <QMenu>
#include <QStackedLayout>
#include <QLineEdit>

/**
 * @brief The NavigationBarNamespace namespace
 */
namespace NavigationBarNamespace {
static const QString stylesheet( "\
QPushButton#crumb {\
    color: rgb( 77, 77, 77, 255 );\
    margin: 0px;\
    font-size: 10pt;\
    font-weight: bold;\
    border-width: 0px;\
    border-style: solid;\
    padding-top: 2px;\
    padding-bottom: 2px;\
    padding-left: 6px;\
    padding-right: 6px;\
}\
QPushButton#crumb:hover {\
    background-color: rgb( 64, 64, 64, 196 );\
    color: white;\
}\
\
QPushButton#crumb:pressed {\
    background-color: rgb( 32, 32, 32, 196 );\
    color: white;\
}\
\
QPushButton#pathSeparator {\
    background-image: url(:/icons/path_splitter);\
    background-position: center;\
    background-repeat: none;\
    width: 6px;\
    height: 12px;\
    margin: 0px;\
    border-width: 0px;\
    border-style: solid;\
    padding: 4px;\
}\
\
QPushButton#pathSeparator:hover {\
    background-image: url(:/icons/path_splitter_light);\
    background-color: rgb( 64, 64, 64, 196 );\
}\
\
QPushButton#pathSeparator:pressed {\
    background-image: url(:/icons/path_splitter_light);\
    background-color: rgb( 32, 32, 32, 196 );\
}\
\
QPushButton#scrollButtonLeft {\
    border-width: 0px;\
    border-style: solid;\
    background-color: rgb( 96, 96, 96, 255 );\
    background-image: url(:/icons/scroll_left);\
    background-position: center;\
    background-repeat: none;\
    width: 10px;\
    height: 12px;\
    margin: 0px;\
    padding-top: 4px;\
    padding-bottom: 4px;\
    padding-left: 2px;\
    padding-right: 2px;\
}\
\
QPushButton#scrollButtonLeft:hover {\
    background-color: rgb( 64, 64, 64, 255 );\
}\
\
QPushButton#scrollButtonLeft:pressed {\
    background-color: rgb( 96, 96, 96, 255 );\
}\
\
\
QPushButton#scrollButtonRight {\
    border-width: 0px;\
    border-style: solid;\
    background-color: rgb( 96, 96, 96, 255 );\
    background-image: url(:/icons/scroll_right);\
    background-position: center;\
    background-repeat: none;\
    width: 10px;\
    height: 12px;\
    margin: 0px;\
    padding-top: 4px;\
    padding-bottom: 4px;\
    padding-left: 2px;\
    padding-right: 2px;\
}\
\
QPushButton#scrollButtonRight:hover {\
   background-color: rgb( 64, 64, 64, 255 );\
}\
\
QPushButton#scrollButtonRight:pressed {\
    background-color: rgb( 96, 96, 96, 255 );\
}\
\
QPushButton#scrollButtonRight:disabled {\
    width: 10px;\
    height: 12px;\
    margin: 0px;\
    padding-top: 4px;\
    padding-bottom: 4px;\
    padding-left: 2px;\
    padding-right: 2px;\
    border-width: 0px;\
    border-style: solid;\
    background-image: url();\
    background-color: palette(window);\
}\
\
QPushButton#scrollButtonLeft:disabled {\
    width: 10px;\
    height: 12px;\
    margin: 0px;\
    padding-top: 4px;\
    padding-bottom: 4px;\
    padding-left: 2px;\
    padding-right: 2px;\
    border-width: 0px;\
    border-style: solid;\
    background-image: url();\
    background-color: palette(window);\
}\
\
QPushButton#editButton {\
    background-image: url(:/icons/path_edit);\
    background-position: center;\
    background-repeat: none;\
    width: 20px;\
    height: 20px;\
    margin: 0px;\
    border-width: 0px;\
    border-style: solid;\
    padding: 4px;\
}\
\
QPushButton#editButton:hover {\
    background-image: url(:/icons/path_edit_light);\
    background-color: rgb( 64, 64, 64, 196 );\
}\
\
QPushButton#editButton:pressed {\
    background-image: url(:/icons/path_edit_light);\
    background-color: rgb( 32, 32, 32, 196 );\
}\
\
QPushButton#spacerButton {\
    margin: 0px;\
    border-width: 0px;\
    border-style: solid;\
    padding: 0px;\
    width: 0px;\
}\
\
QLineEdit {\
    font-size: 10pt;\
    font-weight: bold;\
    color: white;\
    border: none;\
    background-color: rgb( 64, 64, 64, 196 );\
}\
\
QPushButton#editClear {\
    background-image: url(:/icons/edit_clear);\
    background-position: center;\
    background-repeat: none;\
    width: 20px;\
    height: 20px;\
    margin: 0px;\
    border-width: 0px;\
    border-style: solid;\
    padding: 4px;\
}\
\
QPushButton#editClear:hover {\
    background-image: url(:/icons/edit_clear_light);\
    background-color: rgb( 64, 64, 64, 196 );\
}\
\
QPushButton#editClear:pressed {\
    background-image: url(:/icons/edit_clear_light);\
    background-color: rgb( 32, 32, 32, 196 );\
}\
\
QPushButton#editBack {\
    background-image: url(:/icons/edit_back);\
    background-position: center;\
    background-repeat: none;\
    width: 20px;\
    height: 20px;\
    margin: 0px;\
    border-width: 0px;\
    border-style: solid;\
    padding: 4px;\
}\
\
QPushButton#editBack:hover {\
    background-image: url(:/icons/edit_back_light);\
    background-color: rgb( 64, 64, 64, 196 );\
}\
\
QPushButton#editBack:pressed {\
    background-image: url(:/icons/edit_back_light);\
    background-color: rgb( 32, 32, 32, 196 );\
}\
\
" );
}

//
// classes
//
class NavigationBar;
class FileBrowser;

/**
 * @brief The Crumb class
 */
class Crumb : public QObject {
    Q_OBJECT

public:
    enum Types {
        NoType = -1,
        Root,
        Separator,
        Path,
    };
    Q_ENUMS( Types )

    Crumb( NavigationBar *bar, QLayout *layout, Types type, const QString &path = QString::null, const QString &text = QString::null );
    ~Crumb();

private slots:
    void buttonPressed();

private:
    QPushButton *button;
    Types type;
    QString path;
    NavigationBar *parentBar;
};

/**
 * @brief The NavigationBar class
 */
class NavigationBar : public QWidget {
    Q_OBJECT

public:
    explicit NavigationBar( QWidget *parent = 0 );
    ~NavigationBar();
    QAction *addMenuAction( const QString &action ) { return this->menu->addAction( action ); }
    int actionCount() const { return this->menu->actions().count(); }
    FileBrowser *fileBrowser() const { return this->m_fileBrowser; }

protected:
    void resizeEvent( QResizeEvent * );

public slots:
    void scrollLeft();
    void scrollRight();
    void setPath( const QString &path );
    void clearActions() { this->menu->clear(); }
    void showMenu() { this->menu->exec( QCursor::pos()); }
    void setFileBrowser( FileBrowser *fb ) { this->m_fileBrowser = fb; }

private slots:
    void clear();
    void checkBounds();
    void folderSelected( QAction *action );
    void spacerClicked();
    void editFinished();
    void back();

private:
    // widget factory
    template<class T>
    static T *makeSpacer( const QString &objectName = QString::null, const QString &styleSheet = QString::null );
    template<class T>
    static T *makeLayout();
    template<class T>
    static T *makeWidget( const QString &objectName = QString::null, const QString &styleSheet = QString::null );

    // easy scrollbar access
    QScrollBar *scrollBar() { return this->scrollArea->horizontalScrollBar(); }

    // all sorts of widgets
    // layout [NavigationBar(baseLayout(stack[0:navigationWidget(navigationLayout[scrollLeftButton][scrollArea[navigationWidget(navigationLayout[crumbs..])]][scrollRightButton])]
    //                            [1:editWidget]))]
    QWidget *navigationWidget;
    QScrollArea *scrollArea;
    QPushButton *navigationEdit;
    QPushButton *navigationSpacer;
    QWidget *navigationBar;
    QHBoxLayout *navigationLayout;
    QHBoxLayout *baseLayout;
    QStackedLayout *stack;
    QPushButton *scrollLeftButton;
    QPushButton *scrollRightButton;
    QMenu *menu;
    QPushButton *buttonClear;
    QPushButton *buttonBack;
    QHBoxLayout *editLayout;
    QWidget *editSpacer;
    QWidget *editWidget;
    QLineEdit *lineEdit;

    // other stuff
    QString currentPath;
    QList<Crumb*> crumbs;
    FileBrowser *m_fileBrowser;
};
