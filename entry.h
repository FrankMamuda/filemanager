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

#ifndef ENTRY_H
#define ENTRY_H

//
// includes
//
#include <QMimeType>
#include <QPixmap>
#include <QFileInfo>

//
// classes
//
class ContainerModel;

/**
 * @brief The Entry class
 */
class Entry : public QObject {
    Q_OBJECT

public:
    enum EntryTypes {
        FileFolder = 0,
        HardDisk,
        Root,
        Home,
        Thumbnail,
        Trash
    };

    explicit Entry( EntryTypes type = FileFolder, const QFileInfo &fileInfo = QFileInfo(), ContainerModel *parent = 0 );
    QFileInfo info() const { return this->m_fileInfo; }
    QString iconName() const { return this->m_iconName; }
    QPixmap pixmap( int scale ) const;
    QString alias() const;
    EntryTypes type() const { return this->m_type; }
    QString path() const;
    QString filePath() const;
    QMimeType mimeType() const { return this->m_mimeType; }
    ContainerModel *parent() const { return this->m_parent; }
    bool updateScheduled() const { return this->m_updateScheduled; }
    bool isDirectory() const;

public slots:
    void setMimeType( const QMimeType &mimeType ) { this->m_mimeType = mimeType; this->setIconName( this->mimeType().iconName()); }
    void setType( const EntryTypes type ) { this->m_type = type; }
    void setIconName( const QString &iconName ) { this->m_iconName = iconName; }
    void scheduleUpdate( bool update = true ) { this->m_updateScheduled = update; }
    void reset();

private:
    ContainerModel *m_parent;
    QFileInfo m_fileInfo;
    QMimeType m_mimeType;
    QString m_iconName;
    EntryTypes m_type;
    bool m_updateScheduled;
};

#endif // ENTRY_H
