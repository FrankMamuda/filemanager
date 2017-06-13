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
#include <QFile>
#include <QDir>
//#include <QMultiHash>
//#include <QMutexLocker>
//#include <QThread>

#if 0
// must use filesystem watcher even when using native dialogs
// since we cannot reliably generate folder unique hashes
// one option would be list its contents (names+sizes) and hash
// that string. although that is useless for empty folders
// might also add creation date, etc.

struct FileDescriptor {
public:
    enum Mode {
        NoMode = -1,
        Copy,
        Move
    };

    QString absolutePath;
    quint8 hash;
    bool isDir;
};

struct FileOperation {
public:
    enum Mode {
        NoMode = -1,
        Copy,
        Move
    };

    Mode mode;
    QList<FileDescriptor>files;
};
#endif

/**
 * @brief The FileUtils class
 */
class FileUtils : public QObject {
    Q_OBJECT

public:
    FileUtils();

private:

};

#if 0
/**
 * @brief The FileUtilsNamespace namespace
 */
namespace FileUtilsNamespace {
const quint64 ChunkSize = 4096;
}

/**
 * @brief The FileOperation class
 */
class FileOperation {
    Q_PROPERTY( QString fileName READ fileName )
    Q_PROPERTY( QString targetFile READ targetFile )
    Q_PROPERTY( Operations operation READ operation )
    Q_PROPERTY( int groupId READ groupId )
    Q_PROPERTY( bool forced READ isForced )
    Q_PROPERTY( float progress READ progress )
    Q_PROPERTY( bool finished READ isFinished )
    Q_PROPERTY( bool interrupted READ isInterrupted )

public:
    enum Operations {
        NoOperation = -1,
        Copy,
        Move
    };
    Q_ENUMS( Operations )

    enum Results {
        NoResult = -1,
        Success,
        Failure
    };
    Q_ENUMS( Results )

    enum Errors {
        NoErrors = -1,
        NoSource,
        NoDestination,
        Collision,
        SourceNotReadable,
        TargetNotWritable
    };
    Q_ENUMS( Errors )

    FileOperation( const QString &f, const QString &p, Operations o, int id );

public:
    QString fileName() const { return this->m_fileName; }
    QString destination() const { return this->m_destination; }
    Operations operation() const { return this->m_operation; }
    int groupId() const { return this->m_groupId; }
    bool isForced() const { return this->m_force; }
    float progress() const { return this->m_progress; }
    bool isFinished() const { return this->m_finished; }
    bool isInterrupted() const { return this->m_interrupt; }

public slots:
    void update();

signals:
    void finished( Results result );
    void progressed( float progress );
    void interrupted( Errors error );

private:
    QString m_fileName;
    QString m_destination;
    Operations m_operation;
    int m_groupId;
    bool m_force;
    float m_progress;
    bool m_finished;
    bool m_interrupt;

    void finish();
    bool prepare();

    // private progress variables
    qint8 bytesTotal;
    qint8 position;
    QFile sourceFile;
    QFile targetFile;
    QDir dir;
};

class OperationGroup {
    QMultiHash<int, FileOperation> operations;
    QMultiHash<FileOperation::Errors, FileOperation>errors;
};

/**
 * @brief The FileUtils class
 */
class FileUtils : public QThread {
    Q_OBJECT

public:
    FileUtils();

public:
    void addFileOperation( const QString &fileName, const QString &path, FileOperation::Operations op = FileOperation::Copy );
    void addFileOperation( const QStringList &fileList, const QString &path, FileOperation::Operations op = FileOperation::Copy );

private:
    int getFreeGroupHandle() { return 0; }
    QMultiHash<int, FileOperation> operationTable;
    void run();
    mutable QMutex m_mutex;
};
#endif
