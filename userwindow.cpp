#include "userwindow.h"
#include "ui_userwindow.h"

#include <QRect>
#include <QScreen>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QtWidgets>

UserWindow::UserWindow(const QString& userName,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UserWindow)
{
    ui->setupUi(this);
    this->UserName = userName;
    initWindow();
}

void UserWindow::initWindow()
{
    m_HomePage = new HomePage();
    shopWidget = new ShopWidget();
    mineWidget = new UserInterface(UserName);

    shopWidget->SetUserName(UserName);

    ui->stackedWidget->addWidget(m_HomePage);
    ui->stackedWidget->addWidget(shopWidget);
    ui->stackedWidget->addWidget(mineWidget);
    ui->stackedWidget->setCurrentWidget(m_HomePage);
}

UserWindow::~UserWindow()
{
    delete ui;
}

void UserWindow::on_homeBtn_clicked()
{
    ui->stackedWidget->setCurrentWidget(m_HomePage);
}


void UserWindow::on_shopBtn_clicked()
{
    ui->stackedWidget->setCurrentWidget(shopWidget);
    shopWidget->refreshPurchaseItems();
}


void UserWindow::on_mineBtn_clicked()
{
    ui->stackedWidget->setCurrentWidget(mineWidget);
}

