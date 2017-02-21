#ifndef CONTAINER_H
#define CONTAINER_H

//
// includes
//
#include <QAbstractItemView>

//
// classes
//
class FileListModel;

/**
 * @brief The Container class
 */
class Container : public QObject {
    Q_OBJECT

public:
    explicit Container() {}
    ~Container();
    FileListModel *model() const { return this->m_model; }

public slots:
    virtual void setModel( FileListModel *model );

private slots:
    void itemClicked( const QModelIndex &index );

private:
    FileListModel *m_model;
};

#endif // CONTAINER_H
