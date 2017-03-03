
#ifndef HISTORY_H
#define HISTORY_H

//
// includes
//
#include <QObject>
#include <QString>

/**
 * @brief The History class
 */
class History : public QObject {
    Q_OBJECT

signals:
    void reset();
    void changed();

public:
    History( QObject *parent = 0 ) : QObject( parent ), m_position( -1 ) {}
    ~History() {}

    int position() const { return this->m_position; }
    QStringList history() const { return this->m_history; }
    QString current() const { return this->itemAt( this->position()); }
    bool isEmpty() const { return this->history().isEmpty(); }
    QString itemAt( int index ) const;
    bool isBackEnabled() const;
    bool isForwardEnabled() const;
    int count() const { return this->history().count(); }

public slots:
    void clear();
    void addItem( const QString &item );
    void setPosition( int pos ) { this->m_position = pos; }
    void back() { if ( this->m_position > 0 ) { this->m_position--; emit this->changed(); }}
    void forward() { if ( this->m_position < this->count() - 1 ) { this->m_position++; emit this->changed(); }}

private:
    int m_limit;
    int m_position;
    QStringList m_history;
};

#endif // HISTORY_H
