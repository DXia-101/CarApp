#include "ReportForms.h"

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
#include <QCheckBox>
#include "common/logininfoinstance.h"
#include "common/base64.h"

ReportForms::ReportForms(QWidget *parent)
    : QWidget{parent}
{
    m_manager = Common::getNetManager();
    initTableWidget();
}

ReportForms::~ReportForms()
{
    if (m_manager) delete m_manager;
}

void ReportForms::initTableWidget()
{
    Search_Btn = new QPushButton(tr("搜索"), this);
    Search_LineEdit = new QLineEdit(this);
    Update_Btn = new QPushButton(tr("更新"), this);


    connect(Search_Btn, &QPushButton::clicked, this, &ReportForms::search);
    connect(Update_Btn, &QPushButton::clicked, this, &ReportForms::update);

    m_tableWidget = new QTableWidget(this);
    // 创建报表表格
    m_tableWidget->setColumnCount(5);
    m_tableWidget->setHorizontalHeaderLabels(QStringList() <<"订单编号"<<"客户名称"<<"订购日期"<<"发货日期"<<"交付状态");

    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    m_tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);



    QVBoxLayout *vLayout = new QVBoxLayout(this);
    QHBoxLayout *hLayout = new QHBoxLayout();


    hLayout->addWidget(Search_Btn);
    hLayout->addWidget(Search_LineEdit);
    hLayout->addStretch();
    hLayout->addWidget(Update_Btn);

    vLayout->addLayout(hLayout);
    vLayout->addWidget(m_tableWidget);

    refreshTable();
}

void ReportForms::initEditWidget()
{
    ReportForm_Edit = new QWidget();

    QLabel *DeliveryDate_Label = new QLabel(tr("发货日期"));
    QLabel *IsSuccess_Label = new QLabel(tr("交付状态"));
    DeliveryDate_Edit = new QLineEdit();
    IsSuccess_Edit = new QLineEdit();
    update_Save_Btn = new QPushButton(tr("保存"));
    Edit_Cancel_Btn = new QPushButton(tr("取消"));
    connect(update_Save_Btn, &QPushButton::clicked, this, &ReportForms::update_save_info);
    connect(Edit_Cancel_Btn, &QPushButton::clicked, this, &ReportForms::cancel);


    QGridLayout *EditLayout = new QGridLayout();
    EditLayout->addWidget(DeliveryDate_Label,1,0);
    EditLayout->addWidget(IsSuccess_Label,2,0);
    EditLayout->addWidget(DeliveryDate_Edit,1,1);
    EditLayout->addWidget(IsSuccess_Edit,2,1);
    EditLayout->addWidget(update_Save_Btn,3,0);
    EditLayout->addWidget(Edit_Cancel_Btn,3,1);
    ReportForm_Edit->setLayout(EditLayout);
}

QStringList ReportForms::getCountStatus(QByteArray json)
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

void ReportForms::refreshTable()
{
    // 清空文件列表信息
    clearReportFormList();

    //将之前的wareslist清空
    clearReportFormItems();

    
    m_tableWidget->setRowCount(0);

    //获取报表信息数目
    m_ReportFormCount = 0;

    QNetworkRequest request;

    
    
    LoginInfoInstance *login = LoginInfoInstance::getInstance();

    // 127.0.0.1:80/reportforms?cmd=reportformscount
    // 获取报表信息数目
    QString url = QString("http://%1:%2/ReportForm?cmd=ReportFormCount").arg(login->getIp()).arg(login->getPort());
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

        
        m_ReportFormCount = list.at(1).toLong();

        clearReportFormList();

        if(m_ReportFormCount > 0){
            m_start = 0;
            m_count = 10;
            getReportFormList();
        }else{
            refreshReportFormItems();
        }
    });
}

void ReportForms::clearReportFormList()
{
    m_tableWidget->clearContents();
}

void ReportForms::clearReportFormItems()
{
    m_ReportFormList.clear();
}

void ReportForms::refreshReportFormItems()
{
    if(m_ReportFormList.isEmpty() == false){
        int n = m_ReportFormList.size();
        for(int i = 0;i < n;++i){
            ReportFormsInfo *tmp = m_ReportFormList.at(i);
            int row = m_tableWidget->rowCount();
            m_tableWidget->insertRow(row);
            m_tableWidget->setItem(row,0,new QTableWidgetItem(QString::number(tmp->OrderId)));
            m_tableWidget->setItem(row,1,new QTableWidgetItem(tmp->CustomerName));
            m_tableWidget->setItem(row,2,new QTableWidgetItem(tmp->SubscriptionDate));
            m_tableWidget->setItem(row,3,new QTableWidgetItem(tmp->DeliveryDate));
            m_tableWidget->setItem(row,4,new QTableWidgetItem(tmp->IsSuccess));
        }
    }
}

void ReportForms::getReportFormList()
{
    // 遍历数目，结束条件处理
    if(m_ReportFormCount <= 0){ // 函数递归的结束条件
        refreshReportFormItems();// 更新表单
        return;
    }else if(m_count > m_ReportFormCount) // 如果请求文件数量大于报表数目
    {
        m_count = m_ReportFormCount;
    }

    QNetworkRequest request; // 请求对象

    //获取登录信息实例
    LoginInfoInstance *login = LoginInfoInstance::getInstance();    

    QString url = QString("http://%1:%2/ReportForm?cmd=ReportFormNormal").arg(login->getIp()).arg(login->getPort());

    request.setUrl(QUrl(url));

    // 设置请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = setReportFormListJson(login->getUser(),login->getToken(),m_start,m_count);

    //改变文件起始点位置
    m_start += m_count;
    m_ReportFormCount -= m_count;

    
    QNetworkReply * reply = m_manager->post(request,data);
    if(reply == NULL){
        cout<<"getWaresList reply == NULL";
        return;
    }

    
    connect(reply,&QNetworkReply::finished,[=](){
       if(reply->error() != QNetworkReply::NoError){
           cout<<"getWaresList error: "<<reply->errorString();
           reply->deleteLater(); // 释放资源
           return;
       }

       
       QByteArray array = reply->readAll();
       //qDebug()<<"getReportFormList Array:"<<array;

       reply->deleteLater();

       
       if("111" == m_cm.getCode(array)){
           QMessageBox::warning(this,"账户异常","请重新登录！");
           return;
       }

       
       if("015" != m_cm.getCode(array)){
           // 解析报表列表json信息，存放在文件列表中
           getReportFormJsonInfo(array);

           //继续获取报表信息列表
           getReportFormList();
       }
    });
}

void ReportForms::getSearchList()
{
    
    if(m_SearchCount <= 0) 
    {
        cout << "获取用户文件列表条件结束";
        refreshReportFormItems(); //更新item
        return;
    }
    else if(s_count > m_SearchCount) 
    {
        s_count = m_SearchCount;
    }


    QNetworkRequest request; 

    
    LoginInfoInstance *login = LoginInfoInstance::getInstance();

    QString url;

    url = QString("http://%1:%2/ReportForm?cmd=ReportFormResult").arg(login->getIp()).arg(login->getPort());
    request.setUrl(QUrl( url )); 

    
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = setReportFormListJson( login->getUser(), login->getToken(), s_start, s_count);

    
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
            // 解析报表列表json信息，存放在文件列表中
            getReportFormJsonInfo(array);

            //继续获取报表信息列表
            getSearchList();
        }
    });
}

void ReportForms::getReportFormJsonInfo(QByteArray data)
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

            //获取wares对应的数组
            
            QJsonArray array = obj.value("ReportForms").toArray();

            int size = array.size(); 

            for(int i = 0;i < size;++i){
                QJsonObject tmp = array[i].toObject();  
                ReportFormsInfo *info = new ReportFormsInfo;
                info->OrderId = tmp.value("OrderID").toInt();
                info->CustomerName = tmp.value("CustomerName").toString();
                info->SubscriptionDate = tmp.value("SubscriptionDate").toString();
                info->DeliveryDate = tmp.value("DeliveryDate").toString();
                info->IsSuccess = tmp.value("IsSuccess").toString();
                //List添加节点
                m_ReportFormList.append(info);
            }
        }
    }else{
        cout<<"getReportFormJsonInfo error = "<<error.errorString();
    }
}

QByteArray ReportForms::setGetCountJson(QString user, QString token)
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

QByteArray ReportForms::setReportFormListJson(QString user, QString token, int start, int count)
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

QByteArray ReportForms::setUploadJson()
{
    QMap<QString,QVariant> tmp;
    QModelIndexList selectedRows = m_tableWidget->selectionModel()->selectedRows();
    foreach (QModelIndex index, selectedRows) {
        int row = index.row();
        tmp.insert("OrderID",m_tableWidget->item(row, 0)->text().toInt());
        tmp.insert("CustomerName",m_tableWidget->item(row, 1)->text());
    }
    tmp.insert("DeliveryDate",DeliveryDate_Edit->text());
    tmp.insert("IsSuccess",IsSuccess_Edit->text());

    QJsonDocument jsonDocument = QJsonDocument::fromVariant(tmp);
    if(jsonDocument.isNull()){
        cout<<"setUploadJson jsonDocument.isNull()";
        return "";
    }
    return jsonDocument.toJson();
}

QByteArray ReportForms::setSelectJson()
{
    QMap<QString,QVariant> tmp;
    QModelIndexList selectedRows = m_tableWidget->selectionModel()->selectedRows();
    foreach (QModelIndex index, selectedRows) {
        int row = index.row();
        tmp.insert("OrderId",m_tableWidget->item(row, 0)->text().toInt());
        tmp.insert("CustomerName",m_tableWidget->item(row, 1)->text());
        tmp.insert("SubscriptionDate",m_tableWidget->item(row, 2)->text());
        tmp.insert("DeliveryDate",m_tableWidget->item(row, 3)->text());
        tmp.insert("IsSuccess",m_tableWidget->item(row, 4)->text());
    }
    QJsonDocument jsonDocument = QJsonDocument::fromVariant(tmp);
    if(jsonDocument.isNull()){
        cout<<"setSelectJson jsonDocument.isNull()";
        return "";
    }
    return jsonDocument.toJson();
}

void ReportForms::search()
{
    if(Search_LineEdit->text().isEmpty()){
        refreshTable();
        return;
    }
    // 清空文件列表信息
    clearReportFormList();

    //将之前的wareslist清空
    clearReportFormItems();

    
    m_tableWidget->setRowCount(0);

    //获取报表信息数目
    m_SearchCount = 0;

    QNetworkRequest request;

    
    
    LoginInfoInstance *login = LoginInfoInstance::getInstance();

    QString url = QString("http://%1:%2/ReportForm?cmd=ReportFormSearch=%3").arg(login->getIp()).arg(login->getPort()).arg(QString::fromUtf8(Search_LineEdit->text().toUtf8().toBase64()));
    request.setUrl(QUrl(url));

    
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = setGetCountJson(login->getUser(),login->getToken());
    qDebug()<<data;

    
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

        
        m_SearchCount = list.at(1).toLong();
        cout<<"userSearchCount = " << m_SearchCount;


        if(m_SearchCount > 0){
            s_start = 0;
            s_count = 10;
            getSearchList();
        }else{
            refreshReportFormItems();
        }
    });
}

void ReportForms::update()
{
    initEditWidget();
    QModelIndexList selectedRows = m_tableWidget->selectionModel()->selectedRows();
    foreach (QModelIndex index, selectedRows) {
        int row = index.row();
        DeliveryDate_Edit->setText(m_tableWidget->item(row, 2)->text());
        IsSuccess_Edit->setText(m_tableWidget->item(row, 4)->text());
    }
    ReportForm_Edit->show();
}

void ReportForms::update_save_info()
{
    QByteArray array = setUploadJson();

    QNetworkRequest request;
    LoginInfoInstance *login = LoginInfoInstance::getInstance();
    QString url;

    url = QString("http://%1:%2/ReportForm?cmd=ReportFormUpdate").arg(login->getIp()).arg(login->getPort());


    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json"));
    request.setHeader(QNetworkRequest::ContentLengthHeader,QVariant(array.size()));
    QNetworkReply *reply = m_manager->post(request,array);

    connect(reply, &QNetworkReply::readyRead, [=]()
    {
        QByteArray jsonData = reply->readAll();

        qDebug()<<QString(jsonData);
        if("020" == m_cm.getCode(jsonData))
        {
            QMessageBox::information(this,"上传成功","上传成功");
            DeliveryDate_Edit->clear();
            IsSuccess_Edit->clear();
            ReportForm_Edit->hide();
            refreshTable();
        }else{
            QMessageBox::warning(this,"上传失败","上传失败");
            ReportForm_Edit->hide();
        }
        delete reply;
    });
}

void ReportForms::cancel()
{
    DeliveryDate_Edit->clear();
    IsSuccess_Edit->clear();
}
