#include "ShopWidget.h"
#include "ui_ShopWidget.h"
#include "Purchase.h"
#include "PurchaseItem.h"

#include <QJsonDocument>
#include <QNetworkReply>

#include <common/logininfoinstance.h>

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
    QByteArray array = GetAllPurchaseJson();
    qDebug()<<"All Purchase Json is :"<<array;

    QNetworkRequest request;
    LoginInfoInstance *login = LoginInfoInstance::getInstance();
    QString url = QString("http://%1:%2/showpro?cmd=upload").arg(login->getIp()).arg(login->getPort());

    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json"));
    request.setHeader(QNetworkRequest::ContentLengthHeader,QVariant(array.size()));
    QNetworkReply *reply = m_manager->post(request,array);

    connect(reply,&QNetworkReply::readyRead,[=](){

    });
}


void ShopWidget::on_cancelBtn_clicked()
{
    Purchase::getInstance()->RemoveAllPurchase();
    refreshPurchaseItems();
}

// 设置json包
// 还需要把用户名添加上去------------
QByteArray ShopWidget::GetAllPurchaseJson()
{
    QMap<QString, QVariant> tmp;
    int size = Purchase::getInstance()->GetPurchaseSize();
    for(int i = 0;i < size;++i){
        QString name = Purchase::getInstance()->purchaseAt(i);
        tmp.insert(name,Purchase::getInstance()->GetPurchaseCount(name));
    }
    QJsonDocument jsonDocument = QJsonDocument::fromVariant(tmp);
    if(jsonDocument.isNull()){
        cout<<"setGetCountJson jsonDocument.isNull()";
        return "";
    }
    return jsonDocument.toJson();
}
