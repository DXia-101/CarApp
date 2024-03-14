#include "ShopWidget.h"
#include "ui_ShopWidget.h"
#include "Purchase.h"
#include "PurchaseItem.h"

#include <QJsonDocument>
#include <QMessageBox>
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
    QLayoutItem* item;
    while ((item = layout->takeAt(0)) != nullptr) {
        QWidget* widget = item->widget();
        if (widget) {
            widget->deleteLater();
        }
        delete item;
    }
    totalHeight = 0;
    if(Purchase::getInstance()->purchaseisEmpty() == false){
        int n = Purchase::getInstance()->GetPurchaseSize();
        for(int i = 0;i < n;++i){
            QString tmp = Purchase::getInstance()->purchaseAt(i);
            PurchaseItem* purItem = new PurchaseItem(tmp);
            layout->addWidget(purItem);
            totalHeight += purItem->height();
        }
    }
    layout->addStretch();
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
        QByteArray jsonData = reply->readAll();
        /*
            注册 - server端返回的json格式数据：
            成功:{"code":"033"}
            失败:{"code":"034"}
        */
        qDebug()<<QString(jsonData);
        if("033"== m_cm.getCode(jsonData)){
            QMessageBox::information(this,"购买成功","购买成功");
            Purchase::getInstance()->RemoveAllPurchase();
            refreshPurchaseItems();
        }else{
            QMessageBox::warning(this,"购买失败","购买失败");
        }
        delete reply;
    });
}


void ShopWidget::on_cancelBtn_clicked()
{
    Purchase::getInstance()->RemoveAllPurchase();
    refreshPurchaseItems();
}


// 还需要把用户名添加上去------------
QByteArray ShopWidget::GetAllPurchaseJson()
{
    QMap<QString, QVariant> tmp;
    tmp.insert(UserName,9999);
    int size = Purchase::getInstance()->GetPurchaseSize();
    for(int i = 0;i < size;++i){
        QString name = Purchase::getInstance()->purchaseAt(i);
        tmp.insert(name,Purchase::getInstance()->GetPurchaseCount(name));
        //tmp.insert("time",QDateTime::currentDateTime());
    }
    QJsonDocument jsonDocument = QJsonDocument::fromVariant(tmp);
    if(jsonDocument.isNull()){
        cout<<"setGetCountJson jsonDocument.isNull()";
        return "";
    }
    return jsonDocument.toJson();
}
