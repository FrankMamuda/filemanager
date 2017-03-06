
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
    Q_PROPERTY( int position READ position WRITE setPosition )
    Q_PROPERTY( bool empty READ isEmpty )
    Q_PROPERTY( bool backEnabled READ isBackEnabled )
    Q_PROPERTY( bool forwardEnabled READ isForwardEnabled )
    Q_PROPERTY( int count READ count )
    Q_PROPERTY( Modes mode READ mode )

public:
    enum Modes {
        Trim,
        Insert
    };
    Q_ENUMS( Modes )

    // constructor/destructor
    History( Modes mode = Trim ) : m_position( -1 ), m_mode( Trim ) { this->m_mode = mode; }
    ~History() {}

    // properties
    int position() const { return this->m_position; }
    bool isEmpty() const { return this->history().isEmpty(); }
    bool isBackEnabled() const;
    bool isForwardEnabled() const;
    int count() const { return this->history().count(); }
    Modes mode() const { return this->m_mode; }

    // other functions
    QVariantList history() const { return this->m_history; }
    QVariant current() const { return this->itemAt( this->position()); }
    QVariant itemAt( int index ) const;

signals:
    void reset();
    void changed();

public slots:
    // properties
    void setPosition( int pos ) { this->m_position = pos; }

    // other functions
    void clear();
    void addItem( const QVariant &item );
    void back() { if ( this->m_position > 0 ) { this->m_position--; emit this->changed(); }}
    void forward() { if ( this->m_position < this->count() - 1 ) { this->m_position++; emit this->changed(); }}

private:
    // properties
    int m_position;
    Modes m_mode;

    // other members
    QVariantList m_history;
};

Q_DECLARE_METATYPE( History::Modes )

#endif // HISTORY_H
