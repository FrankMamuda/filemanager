#ifndef FILENAMELABEL_H
#define FILENAMELABEL_H

//
// includes
//
#include <QLabel>

class FilenameLabel : public QLabel {
    Q_OBJECT

public:
    FilenameLabel( QWidget *parent ) : QLabel( parent ) {}
    FilenameLabel( const QString &text, QWidget *parent ) : QLabel( text, parent ) {}
    ~FilenameLabel() {}
    QSize sizeHint() const;
    QSize minimumSizeHint() const;

protected:
    void paintEvent( QPaintEvent * );
};

#endif // FILENAMELABEL_H
