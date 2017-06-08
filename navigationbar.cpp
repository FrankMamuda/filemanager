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
#include <QScrollBar>
#include <QDebug>
#include "navigationbar.h"
#include "pathutils.h"
#include "filebrowser.h"

/**
 * @brief PathBar::PathBar
 * @param parent
 */
NavigationBar::NavigationBar( QWidget *parent ) : QWidget( parent ), navigationWidget( new QWidget()), scrollArea( new QScrollArea()), navigationBar( new QWidget()), navigationLayout( nullptr ), menu( new QMenu()), editWidget( new QWidget()), lineEdit( new QLineEdit()) {
    // enable mouse tracking
    this->setMouseTracking( true );

    // set up scroll area
    this->scrollArea->setWidgetResizable( true );
    this->scrollArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    this->scrollArea->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    this->scrollArea->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
    this->scrollArea->setFrameShape( QFrame::NoFrame );
    this->scrollArea->setContentsMargins( 0, 0, 0, 0 );

    // set up scroll buttons
    this->baseLayout = NavigationBar::makeLayout<QHBoxLayout>();
    this->scrollLeftButton = this->makeWidget<QPushButton>( "scrollButtonLeft", NavigationBarNamespace::stylesheet );
    this->baseLayout->addWidget( this->scrollLeftButton );
    this->baseLayout->addWidget( this->scrollArea );
    this->scrollRightButton = this->makeWidget<QPushButton>( "scrollButtonRight", NavigationBarNamespace::stylesheet );
    this->baseLayout->addWidget( this->scrollRightButton );
    this->navigationWidget->setLayout( this->baseLayout );

    // set up edit widget
    this->editLayout = NavigationBar::makeLayout<QHBoxLayout>();
    this->lineEdit->setStyleSheet( NavigationBarNamespace::stylesheet );
    this->editLayout->addWidget( this->lineEdit );
    this->buttonClear = NavigationBar::makeWidget<QPushButton>( "editClear", NavigationBarNamespace::stylesheet );
    this->editLayout->addWidget( this->buttonClear );
    this->buttonBack = NavigationBar::makeWidget<QPushButton>( "editBack", NavigationBarNamespace::stylesheet );
    this->editLayout->addWidget( this->buttonBack );
    this->editSpacer = NavigationBar::makeSpacer<QWidget>();
    this->editLayout->addWidget( this->editSpacer );
    this->editWidget->setLayout( this->editLayout );

    // set up stack
    this->stack = NavigationBar::makeLayout<QStackedLayout>();
    this->stack->addWidget( this->navigationWidget );
    this->stack->addWidget( this->editWidget );
    this->stack->setCurrentIndex( 0 );

    // set central layout
    this->setLayout( this->stack );

    // set up spacer
    this->navigationEdit = NavigationBar::makeWidget<QPushButton>( "editButton", NavigationBarNamespace::stylesheet );
    this->navigationSpacer = NavigationBar::makeSpacer<QPushButton>( "spacerButton", NavigationBarNamespace::stylesheet );

    // connect signals/slots
    this->connect( this->scrollLeftButton, SIGNAL( clicked( bool )), this, SLOT( scrollLeft()));
    this->connect( this->scrollRightButton, SIGNAL( clicked( bool )), this, SLOT( scrollRight()));
    this->connect( this->menu, SIGNAL( triggered( QAction* )), this, SLOT( folderSelected( QAction* )));
    this->connect( this->navigationEdit, SIGNAL( clicked( bool )), this, SLOT( spacerClicked()));
    this->connect( this->navigationSpacer, SIGNAL( clicked( bool )), this, SLOT( spacerClicked()));
    this->connect( this->lineEdit, SIGNAL( returnPressed()), this, SLOT( editFinished()));
    this->connect( this->buttonClear, SIGNAL( clicked( bool )), this->lineEdit, SLOT( clear()));
    this->connect( this->buttonBack, SIGNAL( clicked( bool )), this, SLOT( back()));
    this->connect( this->lineEdit, SIGNAL( returnPressed()), this, SLOT( editFinished()));

    // limit height for now
    this->setMaximumHeight( 28 );
}

/**
 * @brief NavigationBar::~PathBar
 */
NavigationBar::~NavigationBar() {
    this->clear();

    // disconnect singals/slots
    this->disconnect( this->scrollLeftButton, SIGNAL( clicked( bool )));
    this->disconnect( this->scrollRightButton, SIGNAL( clicked( bool )));
    this->disconnect( this->menu, SIGNAL( triggered( QAction* )));
    this->disconnect( this->navigationEdit, SIGNAL( clicked( bool )));
    this->disconnect( this->navigationSpacer, SIGNAL( clicked( bool )));
    this->disconnect( this->buttonClear, SIGNAL( clicked( bool )));
    this->disconnect( this->buttonBack, SIGNAL( clicked( bool )));
    this->disconnect( this->lineEdit, SIGNAL( returnPressed()));

    // destory widgets
    delete this->menu;
    delete this->navigationEdit;
    delete this->navigationSpacer;
    delete this->navigationBar;
    delete this->scrollLeftButton;
    delete this->scrollRightButton;
    //delete this->navigationWidget;
    delete this->scrollArea;
    delete this->lineEdit;
    delete this->buttonBack;
    delete this->editSpacer;
    delete this->editLayout;
    delete this->editWidget;
    delete this->stack;
    delete this->baseLayout;
}

/**
 * @brief NavigationBar::setPath
 * @param path
 */
void NavigationBar::setPath( const QString &path ) {
    int y;
    QStringList list;
    QString crumbPath( "/" );

    // set navigation mode
    this->stack->setCurrentIndex( 0 );

    // clean up
    this->clear();

    // set up layout
    this->navigationLayout = NavigationBar::makeLayout<QHBoxLayout>();

    // split path
    list = path.split( "/", QString::SkipEmptyParts );

    // add root button
    this->crumbs << new Crumb( this, this->navigationLayout, Crumb::Root );

    // append path crumb and separator buttons
    for ( y = 0; y < list.count(); y++ ) {
        crumbPath += list.at( y );

        if ( !crumbPath.endsWith( "/" ))
            crumbPath += "/";

        this->crumbs << new Crumb( this, this->navigationLayout, Crumb::Path, crumbPath, list.at( y ));
        this->crumbs << new Crumb( this, this->navigationLayout, Crumb::Separator, crumbPath );
    }

    // add layout to navigationBar widget
    this->navigationLayout->addWidget( this->navigationEdit );
    this->navigationLayout->addWidget( this->navigationSpacer );
    this->navigationBar->setLayout( this->navigationLayout );

    // display navigationBar widget in scrollarea
    this->scrollArea->setWidget( this->navigationBar );

    // force visibility test
    this->scrollBar()->setRange( 0, this->navigationBar->width());
    this->scrollBar()->setValue( this->scrollBar()->maximum());
    this->checkBounds();

    // store path
    this->currentPath = path;
}

/**
 * @brief NavigationBar::resizeEvent
 * @param event
 */
void NavigationBar::resizeEvent( QResizeEvent *event ) {
    QWidget::resizeEvent( event );

    // scroll to maximum
    this->scrollBar()->setValue( this->scrollBar()->maximum());
    this->checkBounds();
}

/**
 * @brief NavigationBar::scrollLeft
 */
void NavigationBar::scrollLeft() {
    this->scrollBar()->setValue( qMax( 0, this->scrollBar()->value() - this->scrollBar()->pageStep()));
    this->checkBounds();
}

/**
 * @brief NavigationBar::scrollRight
 */
void NavigationBar::scrollRight() {
    this->scrollBar()->setValue( qMin( this->scrollBar()->maximum(), this->scrollBar()->value() + this->scrollBar()->pageStep()));
    this->checkBounds();
}

/**
 * @brief NavigationBar::clear
 */
void NavigationBar::clear() {
    if ( this->navigationLayout != nullptr )
        delete this->navigationLayout;

    if ( !this->crumbs.isEmpty()) {
        qDeleteAll( this->crumbs.begin(), this->crumbs.end());
        this->crumbs.clear();
    }
}

/**
 * @brief NavigationBar::checkBounds
 */
void NavigationBar::checkBounds() {
    //
    // this still works bad
    //
    //qDebug() << "pos" << this->scrollBar()->value() << "max" << this->scrollBar()->maximum() << this->scrollBar()->width();
    //qDebug() << this->scrollArea->viewport()->visibleRegion().boundingRect().width();
    //qDebug() << this->scrollArea->viewport()->width();

    // make sure scroll buttons are hidden when not needed
    if ( this->scrollBar()->value() >= this->scrollBar()->maximum())
        this->scrollRightButton->setEnabled( false );
    else
        this->scrollRightButton->setEnabled( true );

    if ( this->scrollBar()->value() == 0 )
        this->scrollLeftButton->setEnabled( false );
    else
        this->scrollLeftButton->setEnabled( true );
}

/**
 * @brief NavigationBar::folderSelected
 * @param action
 */
void NavigationBar::folderSelected( QAction *action ) {
    this->fileBrowser()->setCurrentPath( action->data().toString());
}

/**
 * @brief NavigationBar::spacerClicked
 */
void NavigationBar::spacerClicked() {
    QFontMetrics fm( this->lineEdit->font());
    this->stack->setCurrentIndex( 1 );
    this->lineEdit->setText( this->currentPath );
    this->lineEdit->setFixedWidth( fm.width( this->lineEdit->text()) + 2 );
}

/**
 * @brief NavigationBar::editFinished
 */
void NavigationBar::editFinished() {
    this->fileBrowser()->setCurrentPath( this->lineEdit->text());
}

/**
 * @brief NavigationBar::back
 */
void NavigationBar::back() {
    this->stack->setCurrentIndex( 0 );
}

/**
 * @brief NavigationBar::makeSpacer
 * @return
 */
template<class T>
T *NavigationBar::makeSpacer( const QString &objectName, const QString &styleSheet ) {
    T *widget;
    widget = NavigationBar::makeWidget<T>( objectName, styleSheet );
    widget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );
    widget->setMinimumWidth( 0 );

    return widget;
}

/**
 * @brief NavigationBar::makeLayout
 * @return
 */
template<class T>
T *NavigationBar::makeLayout() {
    T *layout;
    layout = new T();
    layout->setSpacing( 0 );
    layout->setContentsMargins( 0, 0, 0, 0 );
    return layout;
}

/**
 * @brief NavigationBar::widget
 * @return
 */
template<class T>
T *NavigationBar::makeWidget( const QString &objectName, const QString &styleSheet ) {
    T *widget;
    widget = new T();

    if ( !objectName.isEmpty())
        widget->setObjectName( objectName );

    if ( !styleSheet.isEmpty())
        widget->setStyleSheet( styleSheet );

    return widget;
}

/**
 * @brief Crumb::Crumb
 * @param layout
 * @param type
 * @param dir
 * @param text
 */
Crumb::Crumb( NavigationBar *bar, QLayout *layout, Crumb::Types type, const QString &path, const QString &text ) : parentBar( bar ) {
    if ( layout == nullptr )
        return;

    // set up button
    switch ( type ) {
    case Root:
        this->button = new QPushButton( "/" );
        this->button->setObjectName( "crumb" );
        break;

    case Separator:
        this->button = new QPushButton();
        this->button->setObjectName( "pathSeparator" );
        break;

    case Path:
        this->button = new QPushButton( text );
        this->button->setObjectName( "crumb" );
        break;

    default:
        return;
    }

    // set up button
    this->type = type;
    this->path = path;
    this->button->setStyleSheet( NavigationBarNamespace::stylesheet );
    layout->addWidget( this->button );
    this->connect( this->button, SIGNAL( clicked( bool )), this, SLOT( buttonPressed()));
}

/**
 * @brief Crumb::~Crumb
 */
Crumb::~Crumb() {
    this->disconnect( this->button, SIGNAL( clicked( bool )));
    delete this->button;
}

/**
 * @brief Crumb::buttonPressed
 */
void Crumb::buttonPressed() {
    if ( this->type == Separator ) {
        QDir dir( PathUtils::toWindowsPath( this->path ));
        this->parentBar->clearActions();

        // TODO: detect icons
        foreach ( QString folder, dir.entryList( QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name )) {
            QAction *action = this->parentBar->addMenuAction( folder );
            action->setData( PathUtils::toUnixPath( dir.absolutePath() + "/" + folder + "/" ));
        }

        if ( !this->parentBar->actionCount())
            return;

        this->parentBar->showMenu();
    } else if ( this->type == Path ) {
        this->parentBar->fileBrowser()->setCurrentPath( path );
    }
}
