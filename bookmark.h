#ifndef BOOKMARK_H
#define BOOKMARK_H

#include <QObject>

class BookMark : public QObject
{
    Q_OBJECT
public:
    explicit BookMark(QObject *parent = 0);

signals:

public slots:
};

#endif // BOOKMARK_H