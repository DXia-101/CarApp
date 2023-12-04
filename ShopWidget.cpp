#include "ShopWidget.h"
#include "ui_ShopWidget.h"
#include "Purchase.h"
#include "PurchaseItem.h"

ShopWidget::ShopWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ShopWidget)
{
    ui->setupUi(this);

    m_manager = Common::getNetManager();
    layout = new QVBoxLayout(ui->scrollArea);
    ui->scrollAreaWidgetContents->setLayout(layout);

    refreshPurchaseItems();
}

ShopWidget::~ShopWidget()
{
    delete ui;
}

void ShopWidget::refreshPurchaseItems()
{
    if(Purchase::getInstance()->purchaseisEmpty() == false){
        int n = Purchase::getInstance()->GetPurchaseSize();
        for(int i = 0;i < n;++i){
            QString tmp = Purchase::getInstance()->purchaseAt(i);
            PurchaseItem* purItem = new PurchaseItem(tmp);
            layout->addWidget(purItem);
            totalHeight += purItem->height();
        }
    }
    ui->scrollAreaWidgetContents->setGeometry(0,0,400,totalHeight);
}

void ShopWidget::SetUserName(const QString &name)
{
    UserName = name;
}

void ShopWidget::on_settlementBtn_clicked()
{
    //处理购物车
}


void ShopWidget::on_cancelBtn_clicked()
{
    Purchase::getInstance()->RemoveAllPurchase();
    refreshPurchaseItems();
}

