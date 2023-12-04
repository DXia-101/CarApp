#include "productitem.h"
#include "ui_productitem.h"
#include "Purchase.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>

ProductItem::ProductItem(int id, const QUrl &imageUrl, const QString &name, QWidget *parent):
  QWidget(parent),
  ui(new Ui::ProductItem)
{
    ui->setupUi(this);

    Commodity_id = id;
    Commodity_url = imageUrl;
    Commodity_name = name;
    setImageAndName(imageUrl, name);
}

ProductItem::~ProductItem()
{
    delete ui;
}

int ProductItem::GetCommodityID()
{
    return Commodity_id;
}

QString ProductItem::GetCommodityName()
{
    return Commodity_name;
}

void ProductItem::setImageAndName(const QUrl &imageUrl, const QString &name)
{
    // 创建网络访问管理器
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    // 发送GET请求获取图片数据
    QNetworkReply* reply = manager->get(QNetworkRequest(imageUrl));

    // 当请求完成时，读取图片数据并显示图片
    connect(reply,&QNetworkReply::finished,[=]{
        if (reply->error() == QNetworkReply::NoError) {
            // 读取图片数据
            QByteArray imageData = reply->readAll();

            // 加载图片并显示
            QPixmap image;
            image.loadFromData(imageData);
            ui->image_label->setPixmap(image);
            ui->name_label->setText(name);
        } else {
            // 请求失败，显示错误信息
            qDebug() << "Error: " << reply->errorString();
        }
        reply->deleteLater();
    });
}

void ProductItem::on_confirm_btn_clicked()
{
    Purchase::getInstance()->InsertPurchase(Commodity_name,ui->countSpinBox->value());
}

