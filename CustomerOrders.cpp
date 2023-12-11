#include "CustomerOrders.h"
#include "ui_CustomerOrders.h"
#include "common/logininfoinstance.h"

CustomerOrders::CustomerOrders(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CustomerOrders)
{
    ui->setupUi(this);
    m_manager = Common::getNetManager();
}

CustomerOrders::~CustomerOrders()
{
    delete ui;
}

void CustomerOrders::InitCustomerTree()
{

}

void CustomerOrders::LoadCustomerOrderTable(const QString &UserName)
{

}

void CustomerOrders::on_CompleteBtn_clicked()
{

}

