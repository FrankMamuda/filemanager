#include "notificationdrawer.h"
#include "ui_notificationdrawer.h"

NotificationDrawer::NotificationDrawer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NotificationDrawer)
{
    ui->setupUi(this);
}

NotificationDrawer::~NotificationDrawer()
{
    delete ui;
}
