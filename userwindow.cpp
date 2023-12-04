#include "userwindow.h"
#include "ui_userwindow.h"

#include <QRect>
#include <QScreen>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QtWidgets>

UserWindow::UserWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UserWindow)
{
    ui->setupUi(this);

    initWindow();
}

void UserWindow::initWindow()
{
//    // 获取主屏幕
//    QScreen* screen = QGuiApplication::primaryScreen();

//    // 获取屏幕分辨率
//    QRect screenGeometry = screen->geometry();
//    int screenWidth = screenGeometry.width();
//    int screenHeight = screenGeometry.height();

//    // 设置窗口大小
//    setFixedSize(screenWidth, screenHeight);

    m_HomePage = new HomePage();
    shopWidget = new ShopWidget();
    mineWidget = new QWidget();

    shopWidget->SetUserName(UserName);

    ui->stackedWidget->addWidget(m_HomePage);
    ui->stackedWidget->addWidget(shopWidget);
    ui->stackedWidget->addWidget(mineWidget);
    ui->stackedWidget->setCurrentWidget(m_HomePage);
}

void UserWindow::InitUserName(const QString &name)
{
    UserName = name;
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
}


void UserWindow::on_mineBtn_clicked()
{
    ui->stackedWidget->setCurrentWidget(mineWidget);
}

