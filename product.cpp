#include "product.h"

#include <QTableWidget>
#include <QPushButton>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGridLayout>
#include "common/logininfoinstance.h"


Product::Product(QWidget *parent)
    : QWidget{parent}
{

    m_manager = Common::getNetManager();
    //初始化商品信息表页面
    initTableWidget();
}

Product::~Product()
{
    if (m_manager) delete m_manager;
}

void Product::initTableWidget()
{

    Search_Btn = new QPushButton(tr("搜索"), this);
    Search_LineEdit = new QLineEdit(this);
    Add_Btn = new QPushButton(tr("添加"), this);
    Delete_Btn = new QPushButton(tr("删除"), this);
    Update_Btn = new QPushButton(tr("更新"), this);


    connect(Search_Btn, &QPushButton::clicked, this, &Product::search);
    connect(Add_Btn, &QPushButton::clicked, this, &Product::add);
    connect(Delete_Btn, &QPushButton::clicked, this, &Product::remove);
    connect(Update_Btn, &QPushButton::clicked, this, &Product::update);

    m_tableWidget = new QTableWidget(this);
    // 创建商品表格
    m_tableWidget->setColumnCount(6);
    m_tableWidget->setHorizontalHeaderLabels(QStringList() << u8"商品编号" <<u8"商品名称"<<u8"计量单位"<<u8"商品库存"<<u8"计价单位"<<u8"单价";);

    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    m_tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);



    QVBoxLayout *vLayout = new QVBoxLayout(this);
    QHBoxLayout *hLayout = new QHBoxLayout();


    hLayout->addWidget(Search_Btn);
    hLayout->addWidget(Search_LineEdit);
    hLayout->addStretch();
    hLayout->addWidget(Add_Btn);
    hLayout->addWidget(Delete_Btn);
    hLayout->addWidget(Update_Btn);

    vLayout->addLayout(hLayout);
    vLayout->addWidget(m_tableWidget);

    refreshTable();
}

void Product::initEditWidget()
{
    product_Edit = new QWidget();

    QLabel *id_Label = new QLabel(tr("商品ID"));
    QLabel *name_Label = new QLabel(tr("商品名称"));
    QLabel *store_Label = new QLabel(tr("计量单位"));
    QLabel *amount_Label = new QLabel(tr("商品库存"));
    QLabel *sell_Label = new QLabel(tr("计价单位"));
    QLabel *price_Label = new QLabel(tr("商品价格"));
    id_Edit  = new QLineEdit();
    name_Edit = new QLineEdit();
    store_Edit = new QLineEdit();
    amount_Edit = new QLineEdit();
    sell_Edit = new QLineEdit();
    price_Edit = new QLineEdit();
    update_Save_Btn = new QPushButton(tr("保存"));
    Edit_Cancel_Btn = new QPushButton(tr("取消"));
    connect(update_Save_Btn, &QPushButton::clicked, this, &Product::update_save_info);
    connect(Edit_Cancel_Btn, &QPushButton::clicked, this, &Product::cancel);

    QGridLayout *EditLayout = new QGridLayout();
    EditLayout->addWidget(id_Label,0,0);
    EditLayout->addWidget(name_Label,1,0);
    EditLayout->addWidget(store_Label,2,0);
    EditLayout->addWidget(amount_Label,3,0);
    EditLayout->addWidget(sell_Label,4,0);
    EditLayout->addWidget(price_Label,5,0);
    EditLayout->addWidget(id_Edit,0,1);
    EditLayout->addWidget(name_Edit,1,1);
    EditLayout->addWidget(store_Edit,2,1);
    EditLayout->addWidget(amount_Edit,3,1);
    EditLayout->addWidget(sell_Edit,4,1);
    EditLayout->addWidget(price_Edit,5,1);
    EditLayout->addWidget(update_Save_Btn,6,0);
    EditLayout->addWidget(Edit_Cancel_Btn,6,1);
    product_Edit->setLayout(EditLayout);
}


QStringList Product::getCountStatus(QByteArray json)
{
    QJsonParseError error;
    QStringList list;

    //将来源数据json转化为JsonDocument
    //由QByteArray对象构造一个QJsonDocument对象,用于读写操作
    QJsonDocument doc = QJsonDocument::fromJson(json,&error);
    if(error.error == QJsonParseError::NoError)//没有出错
    {
        if(doc.isNull() || doc.isEmpty()){
            cout<<" doc.isNull() || doc.isEmpty()";
            return list;
        }
        if(doc.isObject()){
            QJsonObject obj = doc.object();
            list.append(obj.value("token").toString());//登录token
            list.append(obj.value("num").toString());//文件个数
        }
    }else{
        cout<<" error = "<<error.errorString();
    }
    return list;
}

// 显示用户的文件列表
void Product::refreshTable()
{
    // 清空文件列表信息
    clearproductList();

    //将之前的productlist清空
    clearproductItems();

    
    m_tableWidget->setRowCount(0);

    //获取商品信息数目
    m_productCount = 0;

    QNetworkRequest request;

    
    
    LoginInfoInstance *login = LoginInfoInstance::getInstance();

    // 127.0.0.1:80/product?cmd=productcount
    // 获取商品信息数目
    QString url = QString("http://%1:%2/product?cmd=productcount").arg(login->getIp()).arg(login->getPort());
    request.setUrl(QUrl(url));

    
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    QByteArray data = setGetCountJson(login->getUser(),login->getToken());

    
    QNetworkReply* reply = m_manager->post(request,data);
    if(reply == NULL){
        qDebug()<<"reply == NULL";
        return;
    }
    connect(reply,&QNetworkReply::finished,[=](){
        if(reply->error() != QNetworkReply::NoError)
        {
            cout<<reply->errorString();
            reply->deleteLater();
            return;
        }
        
        QByteArray array = reply->readAll();

        reply->deleteLater();

        
        QStringList list = getCountStatus(array);

        
        if(list.at(0) == "111"){
            QMessageBox::warning(this,"账户异常","请重新登录!");
            return;
        }

        
        m_productCount = list.at(1).toLong();

        clearproductList();

        if(m_productCount > 0){
            m_start = 0;
            m_count = 10;

            getproductList();
        }else{
            refreshproductItems();
        }
    });
}

void Product::clearproductList()
{
    m_tableWidget->clear();
}

void Product::clearproductItems()
{
    m_productList.clear();
}

void Product::refreshproductItems()
{
    if(m_productList.isEmpty() == false){
        int n = m_productList.size();
        for(int i = 0;i < n;++i){
            productInfo *tmp = m_productList.at(i);
            int row = m_tableWidget->rowCount();
            m_tableWidget->insertRow(row);
            m_tableWidget->setItem(row,0,new QTableWidgetItem(QString::number(tmp->product_id)));
            m_tableWidget->setItem(row,1,new QTableWidgetItem(tmp->product_name));
            m_tableWidget->setItem(row,2,new QTableWidgetItem(tmp->product_store_unit));
            m_tableWidget->setItem(row,3,new QTableWidgetItem(QString::number(tmp->product_amount)));
            m_tableWidget->setItem(row,4,new QTableWidgetItem(tmp->product_sell_unit));
            m_tableWidget->setItem(row,5,new QTableWidgetItem(QString::number(tmp->product_price)));
        }
    }
}

void Product::getproductList()
{
    if(m_productCount <= 0){
        refreshproductItems();
        return;
    }else if(m_count > m_productCount)
    {
        m_count = m_productCount;
    }

    QNetworkRequest request;

    LoginInfoInstance *login = LoginInfoInstance::getInstance();    

    QString tmp = QString("productnormal");

    QString url = QString("http://%1:%2/product?cmd=%3").arg(login->getIp()).arg(login->getPort()).arg(tmp);

    request.setUrl(QUrl(url));

    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = setproductListJson(login->getUser(),login->getToken(),m_start,m_count);

    m_start += m_count;
    m_productCount -= m_count;

    
    QNetworkReply * reply = m_manager->post(request,data);
    if(reply == NULL){
        cout<<"getproductList reply == NULL";
        return;
    }

    
    connect(reply,&QNetworkReply::finished,[=](){
       if(reply->error() != QNetworkReply::NoError){
           cout<<"getproductList error: "<<reply->errorString();
           reply->deleteLater();
           return;
       }

       QByteArray array = reply->readAll();

       reply->deleteLater();

       if("111" == m_cm.getCode(array)){
           QMessageBox::warning(this,"账户异常","请重新登录！");

           return;
       }

       
       if("015" != m_cm.getCode(array)){
           getproductJsonInfo(array);

           getproductList();
       }
    });
}

void Product::getSearchList()
{
    
    if(m_SearchCount <= 0) 
    {
        refreshproductItems();
        return;
    }
    else if(s_count > m_SearchCount) 
    {
        s_count = m_SearchCount;
    }


    QNetworkRequest request; 

    
    LoginInfoInstance *login = LoginInfoInstance::getInstance();

    QString url;

    url = QString("http://%1:%2/product?cmd=productresult").arg(login->getIp()).arg(login->getPort());
    request.setUrl(QUrl( url )); 

    
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = setproductListJson( login->getUser(), login->getToken(), s_start, s_count);

    
    s_start += s_count;
    m_SearchCount -= s_count;

    
    QNetworkReply * reply = m_manager->post( request, data );
    if(reply == NULL)
    {
        cout << "reply == NULL";
        return;
    }

    
    connect(reply, &QNetworkReply::finished, [=]()
    {
        if (reply->error() != QNetworkReply::NoError) 
        {
            cout << reply->errorString();
            reply->deleteLater(); 
            return;
        }

        
        QByteArray array = reply->readAll();

        reply->deleteLater();

        
        if("111" == m_cm.getCode(array)){
            QMessageBox::warning(this,"账户异常","请重新登录！");

            return;
        }

        
        if("015" != m_cm.getCode(array)){
            getproductJsonInfo(array);
            getSearchList();
        }
    });
}

void Product::getproductJsonInfo(QByteArray data)
{
    QJsonParseError error;

    
    
    QJsonDocument doc = QJsonDocument::fromJson(data,&error);
    if(error.error == QJsonParseError::NoError){
        if(doc.isNull() || doc.isEmpty()){
            cout<<" m_productList doc.isNUll() || doc.isEmpty() ";
            return;
        }
        if(doc.isObject()){
            
            QJsonObject obj = doc.object();
            QJsonArray array = obj.value("product").toArray();

            int size = array.size(); 

            for(int i = 0;i < size;++i){
                QJsonObject tmp = array[i].toObject();  
                productInfo *info = new productInfo;
                info->product_id = tmp.value("product_id").toInt();
                info->product_name = tmp.value("product_name").toString();
                info->product_store_unit = tmp.value("product_store_unit").toString();
                info->product_amount = tmp.value("product_amount").toInt();
                info->product_sell_unit = tmp.value("product_sell_unit").toString();
                info->product_price = tmp.value("product_price").toDouble();
                m_productList.append(info);

            }
        }
    }else{
        cout<<"getproductJsonInfo error = "<<error.errorString();
    }
}


QByteArray Product::setGetCountJson(QString user, QString token)
{
    QMap<QString, QVariant> tmp;
    tmp.insert("user", user);
    tmp.insert("token", token);

    QJsonDocument jsonDocument = QJsonDocument::fromVariant(tmp);
    if ( jsonDocument.isNull() )
    {
        cout << "setGetCountJson jsonDocument.isNull() ";
        return "";
    }

    
    return jsonDocument.toJson();
}


QByteArray Product::setproductListJson(QString user, QString token, int start, int count)
{
    QMap<QString,QVariant> tmp;
    tmp.insert("user",user);
    tmp.insert("token",token);
    tmp.insert("start",start);
    tmp.insert("count",count);

    QJsonDocument jsonDocument = QJsonDocument::fromVariant(tmp);
    if( jsonDocument.isNull()){
        cout<<"setproductListJson jsonDocument.isNull()";
        return "";
    }

    return jsonDocument.toJson();
}

QByteArray Product::setUploadJson()
{
    QMap<QString,QVariant> tmp;
    tmp.insert("product_id",id_Edit->text().toInt());
    tmp.insert("product_name",name_Edit->text());
    tmp.insert("product_store_unit",store_Edit->text());
    tmp.insert("product_amount",amount_Edit->text().toInt());
    tmp.insert("product_sell_unit",sell_Edit->text());
    tmp.insert("product_price",price_Edit->text().toInt());

    QJsonDocument jsonDocument = QJsonDocument::fromVariant(tmp);
    if(jsonDocument.isNull()){
        cout<<"setUploadJson jsonDocument.isNull()";
        return "";
    }
    return jsonDocument.toJson();
}

void Product::search()
{
    clearproductList();
    clearproductItems();

    
    m_tableWidget->setRowCount(0);

    m_SearchCount = 0;

    QNetworkRequest request;

    LoginInfoInstance *login = LoginInfoInstance::getInstance();

    QString url = QString("http://%1:%2/product?cmd=productsearch=%3").arg(login->getIp()).arg(login->getPort()).arg(QString::fromUtf8(Search_LineEdit->text().toUtf8().toBase64()));
    request.setUrl(QUrl(url));

    
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = setGetCountJson(login->getUser(),login->getToken());

    
    QNetworkReply* reply = m_manager->post(request,data);
    if(reply == NULL){
        qDebug()<<"reply == NULL";
        return;
    }

    connect(reply,&QNetworkReply::finished,[=](){
        if(reply->error() != QNetworkReply::NoError)
        {
            cout<<reply->errorString();
            reply->deleteLater();
            return;
        }
        
        QByteArray array = reply->readAll();

        reply->deleteLater();

        
        QStringList list = getCountStatus(array);

        
        if(list.at(0) == "111"){
            QMessageBox::warning(this,"账户异常","请重新登录!");
            return;
        }

        
        m_SearchCount = list.at(1).toLong();
        if(m_SearchCount > 0){
            s_start = 0;
            s_count = 10;

            getSearchList();
        }else{
            refreshproductItems();
        }
    });
}

void Product::add()
{
    cur_status = add_status;
    initEditWidget();
    product_Edit->show();
}

QByteArray Product::setSelectJson(){
    QMap<QString,QVariant> tmp;
    QModelIndexList selectedRows = m_tableWidget->selectionModel()->selectedRows();
    foreach (QModelIndex index, selectedRows) {
        int row = index.row();
        tmp.insert("product_id",m_tableWidget->item(row, 0)->text().toInt());
        tmp.insert("product_name",m_tableWidget->item(row, 1)->text());
        tmp.insert("product_store_unit",m_tableWidget->item(row, 2)->text());
        tmp.insert("product_amount",m_tableWidget->item(row, 3)->text());
        tmp.insert("product_sell_unit",m_tableWidget->item(row, 4)->text());
        tmp.insert("product_price",m_tableWidget->item(row, 5)->text());
    }
    QJsonDocument jsonDocument = QJsonDocument::fromVariant(tmp);
    if(jsonDocument.isNull()){
        cout<<"setSelectJson jsonDocument.isNull()";
        return "";
    }
    return jsonDocument.toJson();
}

void Product::remove()
{
    QByteArray array = setSelectJson();

    QNetworkRequest request;
    LoginInfoInstance *login = LoginInfoInstance::getInstance();
    QString url= QString("http://%1:%2/product?cmd=productdelete").arg(login->getIp()).arg(login->getPort());

    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json"));
    request.setHeader(QNetworkRequest::ContentLengthHeader,QVariant(array.size()));
    QNetworkReply *reply = m_manager->post(request,array);

    connect(reply, &QNetworkReply::readyRead, [=]()
    {
        QByteArray jsonData = reply->readAll();
        if("023" == m_cm.getCode(jsonData))
        {
            QMessageBox::information(this,"删除成功","删除成功");
            refreshTable();
        }else{
            QMessageBox::warning(this,"删除失败","删除失败");
        }
        delete reply;
    });
}

void Product::update()
{
    cur_status = update_status;
    initEditWidget();
    QModelIndexList selectedRows = m_tableWidget->selectionModel()->selectedRows();
    foreach (QModelIndex index, selectedRows) {
        int row = index.row();
        id_Edit->setText(m_tableWidget->item(row, 0)->text());
        name_Edit->setText(m_tableWidget->item(row, 1)->text());
        store_Edit->setText(m_tableWidget->item(row, 2)->text());
        amount_Edit->setText(m_tableWidget->item(row, 3)->text());
        sell_Edit->setText(m_tableWidget->item(row, 4)->text());
        price_Edit->setText(m_tableWidget->item(row, 5)->text());
    }
    product_Edit->show();
}

void Product::update_save_info()
{
    QByteArray array = setUploadJson();

    QNetworkRequest request;
    LoginInfoInstance *login = LoginInfoInstance::getInstance();
    QString url;
    if(update_status == cur_status){
        url = QString("http://%1:%2/product?cmd=productupdate").arg(login->getIp()).arg(login->getPort());
    }else if(add_status == cur_status){
        url = QString("http://%1:%2/product?cmd=productadd").arg(login->getIp()).arg(login->getPort());
    }

    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json"));
    request.setHeader(QNetworkRequest::ContentLengthHeader,QVariant(array.size()));
    QNetworkReply *reply = m_manager->post(request,array);

    connect(reply, &QNetworkReply::readyRead, [=]()
    {
        QByteArray jsonData = reply->readAll();
        if("020" == m_cm.getCode(jsonData))
        {
            QMessageBox::information(this,"上传成功","上传成功");
            id_Edit->clear();
            name_Edit->clear();
            store_Edit->clear();
            amount_Edit->clear();
            sell_Edit->clear();
            price_Edit->clear();
            product_Edit->hide();
            refreshTable();
        }else{
            QMessageBox::warning(this,"上传失败","上传失败");
            product_Edit->hide();
        }
        delete reply;
    });
}

void Product::cancel()
{
    id_Edit->clear();
    name_Edit->clear();
    store_Edit->clear();
    amount_Edit->clear();
    sell_Edit->clear();
    price_Edit->clear();
    product_Edit->hide();
}
