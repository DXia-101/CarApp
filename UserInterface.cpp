#include "UserInterface.h"
#include "ui_UserInterface.h"

UserInterface::UserInterface(QString Name,QWidget *parent)
    :QWidget(parent)
    , ui(new Ui::UserInterface)
{
    ui->setupUi(this);

}

UserInterface::~UserInterface()
{
    delete ui;
}

void UserInterface::initTableWidget()
{
    connect(ui->Search_Btn, &QPushButton::clicked, this, &UserInterface::search);

    ui->m_tableWidget->setColumnCount(5);
    QStringList headerLabels;
    ui->m_tableWidget->setHorizontalHeaderLabels(QStringList()<<u8"订单编号" <<u8"商品名称"<<u8"购买数量"<<u8"总金额"<<u8"支付状态");
    ui->m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->m_tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    refreshTable();
}

void UserInterface::refreshTable()
{

}
