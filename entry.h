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
    Q_PROPERTY( QString iconName READ iconName WRITE setIconName )
    Q_PROPERTY( QFileInfo info READ info )
    Q_PROPERTY( QString alias READ alias )
    Q_PROPERTY( EntryTypes type READ type WRITE setType )
    Q_PROPERTY( QMimeType mimeType READ mimeType WRITE setMimeType )
    Q_PROPERTY( QString path READ path )
    Q_PROPERTY( bool directory READ isDirectory )
    Q_PROPERTY( bool cut WRITE setCut READ isCut )
    Q_PROPERTY( bool updated READ isUpdated WRITE setUpdated )

public:
    enum EntryTypes {
        FileFolder = 0,
        HardDisk,
        Root,
        Home,
        Thumbnail,
        Trash,
        Bookmark,
        Executable
    };
    Q_ENUMS( EntryTypes )

    // constructor
    explicit Entry( EntryTypes type = FileFolder, const QFileInfo &fileInfo = QFileInfo(), ContainerModel *parent = 0 );

    // properties
    QFileInfo info() const { return this->m_fileInfo; }
    QString iconName() const { return this->m_iconName; }
    QString alias() const;
    EntryTypes type() const { return this->m_type; }
    QString path() const;
    QMimeType mimeType() const { return this->m_mimeType; }
    bool isDirectory() const;
    bool isCut() const { return this->m_cut; }
    QPixmap iconPixmap() const { return this->m_pixmap; }
    bool isUpdated() const{ return m_updated;}

    // other functions
    QPixmap pixmap( int scale ) const;
    ContainerModel *parent() const { return this->m_parent; }
    static QString getDriveIconName( const QFileInfo &info );

public slots:
    // properties
    void setMimeType( const QMimeType &mimeType ) { this->m_mimeType = mimeType; this->setIconName( this->mimeType().iconName()); }
    void setType( const EntryTypes type ) { this->m_type = type; }
    void setIconName( const QString &iconName ) { this->m_iconName = iconName; }
    void setCut( bool cut = true ) { this->m_cut = cut; }
    void setIconPixmap( const QPixmap &pixmap ) { this->m_pixmap = pixmap; }
    void setUpdated( bool updated ) { this->m_updated = updated; }

    // other slots
    void reset();

private:
    ContainerModel *m_parent;

    // properties
    QFileInfo m_fileInfo;
    QMimeType m_mimeType;
    QString m_iconName;
    EntryTypes m_type;
    bool m_cut;
    QPixmap m_pixmap;
    bool m_updated;
};

Q_DECLARE_METATYPE( Entry::EntryTypes )

#endif // ENTRY_H
