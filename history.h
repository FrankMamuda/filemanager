
#ifndef HISTORY_H
#define HISTORY_H

//
// includes
//
#include <QObject>
#include <QVariant>

/**
 * @brief The History class
 */
class History : public QObject {
    Q_OBJECT

signals:
    void reset();
    void changed();

public:
    enum Modes {
        Trim,
        Insert
    };
    Q_ENUMS( Modes )

    History( Modes mode = Trim ) : m_position( -1 ), m_mode( Trim ) { this->m_mode = mode; }
    ~History() {}

    int position() const { return this->m_position; }
    QVariantList history() const { return this->m_history; }
    QVariant current() const { return this->itemAt( this->position()); }
    bool isEmpty() const { return this->history().isEmpty(); }
    QVariant itemAt( int index ) const;
    bool isBackEnabled() const;
    bool isForwardEnabled() const;
    int count() const { return this->history().count(); }
    Modes mode() const { return this->m_mode; }

public slots:
    void clear();
    void addItem( const QVariant &item );
    void setPosition( int pos ) { this->m_position = pos; }
    void back() { if ( this->m_position > 0 ) { this->m_position--; emit this->changed(); }}
    void forward() { if ( this->m_position < this->count() - 1 ) { this->m_position++; emit this->changed(); }}

private:
    int m_limit;
    int m_position;
    Modes m_mode;
    QVariantList m_history;
};

#endif // HISTORY_H
