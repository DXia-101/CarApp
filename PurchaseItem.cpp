#include "PurchaseItem.h"
#include "ui_PurchaseItem.h"
#include "Purchase.h"

PurchaseItem::PurchaseItem(const QString &name,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PurchaseItem)
{
    ui->setupUi(this);

    this->Commodity_name = name;
    ui->Commodity_name_Label->setText(Commodity_name);
    ui->Commodity_count_SpBox->setValue(Purchase::getInstance()->GetPurchaseCount(Commodity_name));
}

PurchaseItem::~PurchaseItem()
{
    delete ui;
}

void PurchaseItem::on_cancel_btn_clicked()
{
    Purchase::getInstance()->RemovePurchase(Commodity_name);
    this->close();
}

