#ifndef NOTIFICATIONDRAWER_H
#define NOTIFICATIONDRAWER_H

#include <QWidget>

namespace Ui {
class NotificationDrawer;
}

class NotificationDrawer : public QWidget
{
    Q_OBJECT

public:
    explicit NotificationDrawer(QWidget *parent = 0);
    ~NotificationDrawer();

private:
    Ui::NotificationDrawer *ui;
};

#endif // NOTIFICATIONDRAWER_H
