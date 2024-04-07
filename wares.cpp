#include "wares.h"

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
#include "common/base64.h"


Wares::Wares(QWidget *parent)
    : QWidget{parent}
{

    m_manager = Common::getNetManager();
    //初始化原料信息表页面
    initTableWidget();
}

Wares::~Wares()
{
    if (m_manager) delete m_manager;
}

void Wares::initTableWidget()
{

    Search_Btn = new QPushButton(tr("搜索"), this);
    Search_LineEdit = new QLineEdit(this);
    Add_Btn = new QPushButton(tr("添加"), this);
    Delete_Btn = new QPushButton(tr("删除"), this);
    Update_Btn = new QPushButton(tr("更新"), this);


    connect(Search_Btn, &QPushButton::clicked, this, &Wares::search);
    connect(Add_Btn, &QPushButton::clicked, this, &Wares::add);
    connect(Delete_Btn, &QPushButton::clicked, this, &Wares::remove);
    connect(Update_Btn, &QPushButton::clicked, this, &Wares::update);

    m_tableWidget = new QTableWidget(this);
    // 创建原料表格
    m_tableWidget->setColumnCount(6);
    m_tableWidget->setHorizontalHeaderLabels(QStringList() <<"原料编号"<<"原料名称"<<"计量单位"<<"原料库存"<<"计价单位"<<"单价");

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

void Wares::initEditWidget()
{
    Wares_Edit = new QWidget();

    QLabel *id_Label = new QLabel(tr("原料ID"));
    QLabel *name_Label = new QLabel(tr("原料名称"));
    QLabel *store_Label = new QLabel(tr("计量单位"));
    QLabel *amount_Label = new QLabel(tr("原料库存"));
    QLabel *sell_Label = new QLabel(tr("计价单位"));
    QLabel *price_Label = new QLabel(tr("原料价格"));
    id_Edit  = new QLineEdit();
    name_Edit = new QLineEdit();
    store_Edit = new QLineEdit();
    amount_Edit = new QLineEdit();
    sell_Edit = new QLineEdit();
    price_Edit = new QLineEdit();
    update_Save_Btn = new QPushButton(tr("保存"));
    Edit_Cancel_Btn = new QPushButton(tr("取消"));
    connect(update_Save_Btn, &QPushButton::clicked, this, &Wares::update_save_info);
    connect(Edit_Cancel_Btn, &QPushButton::clicked, this, &Wares::cancel);


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
    Wares_Edit->setLayout(EditLayout);
}


QStringList Wares::getCountStatus(QByteArray json)
{
    QJsonParseError error;
    QStringList list;

    QJsonDocument doc = QJsonDocument::fromJson(json,&error);
    if(error.error == QJsonParseError::NoError)
    {
        if(doc.isNull() || doc.isEmpty()){
            cout<<" doc.isNull() || doc.isEmpty()";
            return list;
        }
        if(doc.isObject()){
            QJsonObject obj = doc.object();
            list.append(obj.value("token").toString());
            list.append(obj.value("num").toString());
        }
    }else{
        cout<<" error = "<<error.errorString();
    }

    return list;
}

void Wares::refreshTable()
{
    clearWaresList();

    clearWaresItems();

    m_tableWidget->setRowCount(0);

    m_WaresCount = 0;

    QNetworkRequest request;

    LoginInfoInstance *login = LoginInfoInstance::getInstance();

    QString url = QString("http://%1:%2/wares?cmd=warescount").arg(login->getIp()).arg(login->getPort());
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
            //emit
            return;
        }

        m_WaresCount = list.at(1).toLong();

        clearWaresList();

        if(m_WaresCount > 0){
            m_start = 0;
            m_count = 10;
            getWaresList();
        }else{
            refreshWaresItems();
        }
    });
}

void Wares::clearWaresList()
{
    m_tableWidget->clearContents();
}

void Wares::clearWaresItems()
{
    m_waresList.clear();
}

void Wares::refreshWaresItems()
{
    if(m_waresList.isEmpty() == false){
        int n = m_waresList.size();
        for(int i = 0;i < n;++i){
            WaresInfo *tmp = m_waresList.at(i);
            int row = m_tableWidget->rowCount();
            m_tableWidget->insertRow(row);
            m_tableWidget->setItem(row,0,new QTableWidgetItem(QString::number(tmp->wares_id)));
            m_tableWidget->setItem(row,1,new QTableWidgetItem(tmp->wares_name));
            m_tableWidget->setItem(row,2,new QTableWidgetItem(tmp->wares_store_unit));
            m_tableWidget->setItem(row,3,new QTableWidgetItem(QString::number(tmp->wares_amount)));
            m_tableWidget->setItem(row,4,new QTableWidgetItem(tmp->wares_sell_unit));
            m_tableWidget->setItem(row,5,new QTableWidgetItem(QString::number(tmp->wares_price)));
        }
    }
}

void Wares::getWaresList()
{
    if(m_WaresCount <= 0){
        refreshWaresItems();
        return;
    }else if(m_count > m_WaresCount)
    {
        m_count = m_WaresCount;
    }

    QNetworkRequest request;

    LoginInfoInstance *login = LoginInfoInstance::getInstance();    

    QString tmp = QString("waresnormal");

    QString url = QString("http://%1:%2/wares?cmd=%3").arg(login->getIp()).arg(login->getPort()).arg(tmp);

    request.setUrl(QUrl(url));

    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = setWaresListJson(login->getUser(),login->getToken(),m_start,m_count);

    m_start += m_count;
    m_WaresCount -= m_count;

    QNetworkReply * reply = m_manager->post(request,data);
    if(reply == NULL){
        return;
    }

    connect(reply,&QNetworkReply::finished,[=](){
       if(reply->error() != QNetworkReply::NoError){
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
           getWaresJsonInfo(array);
           getWaresList();
       }
    });
}

void Wares::getSearchList()
{
    
    if(m_SearchCount <= 0) 
    {
        refreshWaresItems();
        return;
    }
    else if(s_count > m_SearchCount) 
    {
        s_count = m_SearchCount;
    }


    QNetworkRequest request; 

    
    LoginInfoInstance *login = LoginInfoInstance::getInstance();

    QString url;

    url = QString("http://%1:%2/wares?cmd=waresresult").arg(login->getIp()).arg(login->getPort());
    request.setUrl(QUrl( url )); 

    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = setWaresListJson( login->getUser(), login->getToken(), s_start, s_count);

    
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
            getWaresJsonInfo(array);
            getSearchList();
        }
    });
}

void Wares::getWaresJsonInfo(QByteArray data)
{
    QJsonParseError error;

    
    
    QJsonDocument doc = QJsonDocument::fromJson(data,&error);
    if(error.error == QJsonParseError::NoError){
        if(doc.isNull() || doc.isEmpty()){
            cout<<" m_waresList doc.isNUll() || doc.isEmpty() ";
            return;
        }
        if(doc.isObject()){
            
            QJsonObject obj = doc.object();

            QJsonArray array = obj.value("wares").toArray();

            int size = array.size(); 

            for(int i = 0;i < size;++i){
                QJsonObject tmp = array[i].toObject();  
                WaresInfo *info = new WaresInfo;
                info->wares_id = tmp.value("wares_id").toInt();
                info->wares_name = tmp.value("wares_name").toString();
                info->wares_store_unit = tmp.value("wares_store_unit").toString();
                info->wares_amount = tmp.value("wares_amount").toInt();
                info->wares_sell_unit = tmp.value("wares_sell_unit").toString();
                info->wares_price = tmp.value("wares_price").toDouble();

                m_waresList.append(info);

            }
        }
    }else{
        cout<<"getWaresJsonInfo error = "<<error.errorString();
    }
}


QByteArray Wares::setGetCountJson(QString user, QString token)
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


QByteArray Wares::setWaresListJson(QString user, QString token, int start, int count)
{
    QMap<QString,QVariant> tmp;
    tmp.insert("user",user);
    tmp.insert("token",token);
    tmp.insert("start",start);
    tmp.insert("count",count);

    QJsonDocument jsonDocument = QJsonDocument::fromVariant(tmp);
    if( jsonDocument.isNull()){
        cout<<"setWaresListJson jsonDocument.isNull()";
        return "";
    }

    return jsonDocument.toJson();
}

QByteArray Wares::setUploadJson()
{
    QMap<QString,QVariant> tmp;
    tmp.insert("wares_id",id_Edit->text().toInt());
    tmp.insert("wares_name",name_Edit->text());
    tmp.insert("wares_store_unit",store_Edit->text());
    tmp.insert("wares_amount",amount_Edit->text().toInt());
    tmp.insert("wares_sell_unit",sell_Edit->text());
    tmp.insert("wares_price",price_Edit->text().toInt());

    QJsonDocument jsonDocument = QJsonDocument::fromVariant(tmp);
    if(jsonDocument.isNull()){
        cout<<"setUploadJson jsonDocument.isNull()";
        return "";
    }
    return jsonDocument.toJson();
}

void Wares::search()
{
    if(Search_LineEdit->text().isEmpty()){
        refreshTable();
        return;
    }

    clearWaresList();

    clearWaresItems();

    m_tableWidget->setRowCount(0);

    m_SearchCount = 0;

    QNetworkRequest request;

    LoginInfoInstance *login = LoginInfoInstance::getInstance();

    QString url = QString("http://%1:%2/wares?cmd=waressearch=%3").arg(login->getIp()).arg(login->getPort()).arg(QString::fromUtf8(Search_LineEdit->text().toUtf8().toBase64()));
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
            refreshWaresItems();
        }
    });
}

void Wares::add()
{
    cur_status = add_status;
    initEditWidget();
    Wares_Edit->show();
}

QByteArray Wares::setSelectJson(){
    QMap<QString,QVariant> tmp;
    QModelIndexList selectedRows = m_tableWidget->selectionModel()->selectedRows();
    foreach (QModelIndex index, selectedRows) {
        int row = index.row();
        tmp.insert("wares_id",m_tableWidget->item(row, 0)->text().toInt());
        tmp.insert("wares_name",m_tableWidget->item(row, 1)->text());
        tmp.insert("wares_store_unit",m_tableWidget->item(row, 2)->text());
        tmp.insert("wares_amount",m_tableWidget->item(row, 3)->text());
        tmp.insert("wares_sell_unit",m_tableWidget->item(row, 4)->text());
        tmp.insert("wares_price",m_tableWidget->item(row, 5)->text());
    }
    QJsonDocument jsonDocument = QJsonDocument::fromVariant(tmp);
    if(jsonDocument.isNull()){
        cout<<"setSelectJson jsonDocument.isNull()";
        return "";
    }
    return jsonDocument.toJson();
}

void Wares::remove()
{
    QByteArray array = setSelectJson();

    QNetworkRequest request;
    LoginInfoInstance *login = LoginInfoInstance::getInstance();
    QString url= QString("http://%1:%2/wares?cmd=waresdelete").arg(login->getIp()).arg(login->getPort());

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

void Wares::update()
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
    Wares_Edit->show();
}

void Wares::update_save_info()
{
    QByteArray array = setUploadJson();

    QNetworkRequest request;
    LoginInfoInstance *login = LoginInfoInstance::getInstance();
    QString url;
    if(update_status == cur_status){
        url = QString("http://%1:%2/wares?cmd=waresupdate").arg(login->getIp()).arg(login->getPort());
    }else if(add_status == cur_status){
        url = QString("http://%1:%2/wares?cmd=waresadd").arg(login->getIp()).arg(login->getPort());
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
            Wares_Edit->hide();
            refreshTable();
        }else{
            QMessageBox::warning(this,"上传失败","上传失败");
            Wares_Edit->hide();
        }
        delete reply;
    });
}

void Wares::cancel()
{
    id_Edit->clear();
    name_Edit->clear();
    store_Edit->clear();
    amount_Edit->clear();
    sell_Edit->clear();
    price_Edit->clear();
    Wares_Edit->hide();
}
