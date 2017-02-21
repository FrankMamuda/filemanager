#if 0
//
// include
//
#include "container.h"
#include "filelistmodel.h"
#include "mainwindow.h"
#include "fileentry.h"
#include <QDesktopServices>

/**
 * @brief Container::setModel
 * @param model
 */
void Container::setModel( FileListModel *model ) {
    this->m_model = model;
}

/**
 * @brief Container::~Container
 */
Container::~Container() {
    if ( this->model() != NULL )
        this->m_model->deleteLater();
}

/**
 * @brief Container::itemClicked
 * @param index
 */
void Container::itemClicked( const QModelIndex &index ) {
/*    FileSystemEntry *entry;
    MainWindow *gui;

    entry = this->model()->indexToEntry( index );
    gui = qobject_cast<MainWindow*>( this->parentWidget()->parentWidget());
    if ( entry == NULL || gui == NULL )
        return;

    if ( entry->isDirectory()) {
        gui->setCurrentPath( entry->path());
    } else {
        if ( this->model()->mode() == FileListModel::FileFolder )
            QDesktopServices::openUrl( QUrl::fromLocalFile( entry->filePath()));
    }*/
}
#endif
