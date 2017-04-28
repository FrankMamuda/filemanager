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
// Conceptual class for file operations:
//  - must be threaded
//  - must emit progress reports
//  - must detect file collisions
//

//
// includes
//
#include "fileutils.h"
#include "pathutils.h"
#include <QDebug>

// TODO: run an event loop that performs file operations in background
//       must not allow termination of app, while performing write operations
//       must not allow multiple instances of app, but allow opening multiple
//       windows or tabs
//
//       must allow forcing of similar operations within a single operation group
//

#if 0
/**
 * @brief FileUtils::FileUtils
 */
FileUtils::FileUtils() {

}

/**
 * @brief FileUtils::addFileOperation
 * @param fileName
 * @param path
 * @param op
 */
void FileUtils::addFileOperation( const QString &fileName, const QString &path, FileOperation::Operations op ) {
    int groupId;
    QMutexLocker( &this->m_mutex );

    groupId = this->getFreeGroupHandle();
    this->operationTable.insert( groupId, FileOperation( fileName, path, op, groupId ));
}

/**
 * @brief FileUtils::addFileOperation
 * @param fileList
 * @param path
 * @param op
 */
void FileUtils::addFileOperation( const QStringList &fileList, const QString &path, FileOperation::Operations op) {
    int groupId;
    QMutexLocker( &this->m_mutex );

    groupId = this->getFreeGroupHandle();
    foreach ( QString fileName, fileList )
        this->operationTable.insert( groupId, FileOperation( fileName, path, op, groupId ));
}

/**
 * @brief FileUtils::run
 */
void FileUtils::run() {
    // enter event loop
    while ( !this->isInterruptionRequested()) {
        QMutexLocker( &this->m_mutex );

        if ( !this->operationTable.isEmpty()) {
            // do smth

            // this should also be threaded, although
            // we should limit threads to groups



        } else {
            msleep( 100 );
        }
    }
}

/**
 * @brief FileUtils::copy
 * @param fileName
 * @param path
 */
/*#define SIMULATION
bool FileUtils::copy2( const QString &fileName, const QString &path, bool force ) {
    Q_UNUSED( force )
    QDir dir( PathUtils::toWindowsPath( path ));
    QFile source( PathUtils::toWindowsPath( fileName )), destination( dir.absolutePath() + "/" + fileName );

    // TODO: force dir creation
    if ( !source.exists() || !dir.exists()) {
        qDebug() << "source file or target directory do not exist";
        return false;
    }

    // TODO: handle collision
    if ( destination.exists()) {
        qDebug() << "destination file already exists";
        return false;
    }

    if ( source.open( QFile::ReadOnly )
#ifndef SIMULATION
          || !destination.open( QFile::WriteOnly | QFile::Truncate )
#endif
         ) {
        QByteArray data;

        while ( !source.atEnd()) {
            data = source.read( FileUtilsNamespace::ChunkSize );

#ifndef SIMULATION
            destination.write( data );
#else
            QFileInfo info( source );
            //qDebug() << "simulating copy operation from" << info.absoluteFilePath() << data.size();
#endif
        }

        source.close();
#ifndef SIMULATION
        destination.close();
#endif
        return true;
    }

    qDebug() << "bad copy";
    return false;
}*/

/**
 * @brief FileOperation::FileOperation
 * @param f
 * @param p
 * @param o
 * @param id
 */
FileOperation::FileOperation(const QString &f, const QString &p, FileOperation::Operations o, int id)
    : m_fileName( f ), m_destination( p ), m_operation( o ), m_groupId( id ),
      m_force( false ), m_progress( 0.0f ), m_finished( false ), m_interrupt( false ),
      bytesTotal( 0 ), position( 0 ) {
    QFileInfo info;

    // correct paths if not done already
    this->m_fileName = PathUtils::toWindowsPath( this->fileName());
    this->m_destination = PathUtils::toWindowsPath( this->destination());

    // set paths
    info.setFile( this->fileName());
    this->sourceFile.setFileName( this->fileName());
    this->dir.setPath( this->destination());
    this->targetFile.setFileName( this->dir.absolutePath() + "/" + info.fileName());

    // failsafes
    if ( !this->prepare())
        return;

    // check if target already exists
    if ( !this->targetFile.exists()) {
        this->m_interrupt = true;
        emit this->interrupted( Collision );

        // must perform all checks after collision?
        return;
    }

    // check if target file is writable
    if ( !this->targetFile.open( QFile::WriteOnly | QFile::Truncate )) {
        this->finish();
        emit this->finished( TargetNotWritable );
        return;
    }
}

/**
 * @brief FileOperation::update
 */
void FileOperation::update() {
    QByteArray buffer;

    if ( this->sourceFile.isOpen()) {
        buffer = this->sourceFile.read( FileUtilsNamespace::ChunkSize );

        qDebug() << "simulate write of" << buffer.size() << "bytes";
        this->position += buffer.size();
    }

    // update progress
    this->m_progress = static_cast<float>( this->position ) / static_cast<float>( this->bytesTotal );

    // signal updates
    if ( this->sourceFile.atEnd()) {
        this->finish();
        emit this->finished( Success );
    } else {
        emit this->progressed( this->progress());
    }
}

/**
 * @brief FileOperation::prepare
 */
bool FileOperation::prepare() {
    // check if source file exists
    if ( !this->sourceFile.exists()) {
        this->finish();
        emit this->finished( NoSource );
        return false;
    }

    // check if source file is readable
    if ( !this->sourceFile.open( QFile::ReadOnly )) {
        this->finish();
        emit this->finished( SourceNotReadable );
        return false;
    }

    // check if destination dir exists
    if ( !this->dir.exists()) {
        this->finish();
        emit this->finished( NoDestination );
        return false;
    }

    return true;
}

/**
 * @brief FileOperation::finish
 */
void FileOperation::finish() {
    this->m_finished = true;
    this->m_interrupt = true;

    if ( this->sourceFile.isOpen())
        this->sourceFile.close();

    if ( this->targetFile.isOpen())
        this->targetFile.close();
}
#endif
