#include "UserOrderTable.h"

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

UserOrderTable::UserOrderTable(QString userName,QWidget *parent)
    : UserName(userName),QWidget{parent}
{
    m_manager = Common::getNetManager();
    initTableWidget();
}

UserOrderTable::~UserOrderTable()
{
    if (m_manager) delete m_manager;
}

void UserOrderTable::initTableWidget()
{
    Search_Btn = new QPushButton(tr("搜索"), this);
    Search_LineEdit = new QLineEdit(this);
    Delete_Btn = new QPushButton(tr("删除"), this);
    Update_Btn = new QPushButton(tr("更新"), this);

    connect(Search_Btn, &QPushButton::clicked, this, &UserOrderTable::search);
    connect(Delete_Btn, &QPushButton::clicked, this, &UserOrderTable::remove);
    connect(Update_Btn, &QPushButton::clicked, this, &UserOrderTable::update);

    m_tableWidget = new QTableWidget(this);
    // 创建商品表格
    m_tableWidget->setColumnCount(4);
    m_tableWidget->setHorizontalHeaderLabels(QStringList() <<"订单ID" <<"商品名称"<<"订购数量"<<"订购时间");


    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    m_tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);



    QVBoxLayout *vLayout = new QVBoxLayout(this);
    QHBoxLayout *hLayout = new QHBoxLayout();


    hLayout->addWidget(Search_Btn);
    hLayout->addWidget(Search_LineEdit);
    hLayout->addStretch();
    hLayout->addWidget(Delete_Btn);
    hLayout->addWidget(Update_Btn);

    vLayout->addLayout(hLayout);
    vLayout->addWidget(m_tableWidget);

    refreshTable();
}

void UserOrderTable::initEditWidget()
{
    UserOrder_Edit = new QWidget();

    QLabel *UserOrder_OrderID_Label = new QLabel(tr("订单ID"));
    QLabel *UserOrder_ProductName_Label = new QLabel(tr("商品名称"));
    QLabel *UserOrder_Count_Label = new QLabel(tr("订购数量"));

    UserOrder_OrderID_Edit = new QLineEdit();
    UserOrder_OrderID_Edit->setFocusPolicy(Qt::NoFocus);
    UserOrder_ProductName_Edit  = new QLineEdit();
    UserOrder_Count_Edit = new QLineEdit();
    update_Save_Btn = new QPushButton(tr("保存"));
    Edit_Cancel_Btn = new QPushButton(tr("取消"));
    connect(update_Save_Btn, &QPushButton::clicked, this, &UserOrderTable::update_save_info);
    connect(Edit_Cancel_Btn, &QPushButton::clicked, this, &UserOrderTable::cancel);

    QGridLayout *EditLayout = new QGridLayout();
    EditLayout->addWidget(UserOrder_OrderID_Label,0,0);
    EditLayout->addWidget(UserOrder_ProductName_Label,1,0);
    EditLayout->addWidget(UserOrder_Count_Label,2,0);
    EditLayout->addWidget(UserOrder_OrderID_Edit,0,1);
    EditLayout->addWidget(UserOrder_ProductName_Edit,1,1);
    EditLayout->addWidget(UserOrder_Count_Edit,2,1);

    EditLayout->addWidget(update_Save_Btn,6,0);
    EditLayout->addWidget(Edit_Cancel_Btn,6,1);
    UserOrder_Edit->setLayout(EditLayout);
}

QStringList UserOrderTable::getCountStatus(QByteArray json)
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

void UserOrderTable::refreshTable()
{
    // 清空文件列表信息
    clearUserOrderList();

    //将之前的UserOrderlist清空
    clearUserOrderItems();

    
    m_tableWidget->setRowCount(0);

    //获取商品信息数目
    m_UserOrderCount = 0;

    QNetworkRequest request;

    
    
    LoginInfoInstance *login = LoginInfoInstance::getInstance();

    // 127.0.0.1:80/UserOrder?cmd=UserOrdercount
    // 获取商品信息数目
    QString url = QString("http://%1:%2/UserOrder?cmd=userOrdercount=%3").arg(login->getIp()).arg(login->getPort()).arg(QString::fromUtf8(UserName.toUtf8()));
    request.setUrl(QUrl(url));
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
            qDebug()<<reply->errorString();
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

        
        m_UserOrderCount = list.at(1).toLong();

        clearUserOrderList();

        if(m_UserOrderCount > 0){
            m_start = 0;
            m_count = 10;
            getUserOrderList();
        }else{
            refreshUserOrderItems();
        }
    });
}

void UserOrderTable::clearUserOrderList()
{
    m_tableWidget->clearContents();
}

void UserOrderTable::clearUserOrderItems()
{
    m_UserOrderList.clear();
}

void UserOrderTable::refreshUserOrderItems()
{
    //如果文件列表不为空，显示商品列表
    if(m_UserOrderList.isEmpty() == false){
        int n = m_UserOrderList.size();
        for(int i = 0;i < n;++i){
            UserOrderTableInfo *tmp = m_UserOrderList.at(i);
            int row = m_tableWidget->rowCount();
            m_tableWidget->insertRow(row);
            m_tableWidget->setItem(row,0,new QTableWidgetItem(QString::number(tmp->OrderID)));
            m_tableWidget->setItem(row,1,new QTableWidgetItem(tmp->ProductName));
            m_tableWidget->setItem(row,2,new QTableWidgetItem(QString::number(tmp->count)));
            m_tableWidget->setItem(row,3,new QTableWidgetItem(tmp->timestamp));
        }
    }
}

void UserOrderTable::getUserOrderList()
{
    // 遍历数目，结束条件处理
    if(m_UserOrderCount <= 0){ // 函数递归的结束条件
        refreshUserOrderItems();// 更新表单
        return;
    }else if(m_count > m_UserOrderCount) // 如果请求文件数量大于商品数目
    {
        m_count = m_UserOrderCount;
    }

    QNetworkRequest request; // 请求对象

    //获取登录信息实例
    LoginInfoInstance *login = LoginInfoInstance::getInstance();    

    QString url = QString("http://%1:%2/UserOrder?cmd=userOrdernormal=%3").arg(login->getIp()).arg(login->getPort()).arg(QString::fromUtf8(UserName.toUtf8()));

    request.setUrl(QUrl(url));

    // 设置请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = setUserOrderListJson(login->getUser(),login->getToken(),m_start,m_count);

    //改变文件起始点位置
    m_start += m_count;
    m_UserOrderCount -= m_count;

    QNetworkReply * reply = m_manager->post(request,data);
    if(reply == NULL){
        cout<<"getUserOrderList reply == NULL";
        return;
    }

    connect(reply,&QNetworkReply::finished,[=](){
       if(reply->error() != QNetworkReply::NoError){
           cout<<"getUserOrderList error: "<<reply->errorString();
           reply->deleteLater(); // 释放资源
           return;
       }

       
       QByteArray array = reply->readAll();

       reply->deleteLater();

       
       if("111" == m_cm.getCode(array)){
           QMessageBox::warning(this,"账户异常","请重新登录！");

           return;
       }

       if("015" != m_cm.getCode(array)){
           getUserOrderJsonInfo(array);

           //继续获取商品信息列表
           getUserOrderList();
       }
    });
}

void UserOrderTable::getSearchList()
{
    
    if(m_SearchCount <= 0) 
    {
        refreshUserOrderItems(); //更新item
        return;
    }
    else if(s_count > m_SearchCount) 
    {
        s_count = m_SearchCount;
    }
    QNetworkRequest request; 
    
    LoginInfoInstance *login = LoginInfoInstance::getInstance();

    QString url;

    url = QString("http://%1:%2/UserOrder?cmd=userOrderresult=%3").arg(login->getIp()).arg(login->getPort()).arg(QString::fromUtf8(UserName.toUtf8()));
    request.setUrl(QUrl( url ));

    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = setUserOrderListJson( login->getUser(), login->getToken(), s_start, s_count);

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
            // 解析商品列表json信息，存放在文件列表中
            getUserOrderJsonInfo(array);

            //继续获取商品信息列表
            getSearchList();
        }
    });
}

void UserOrderTable::getUserOrderJsonInfo(QByteArray data)
{
    QJsonParseError error;

    QJsonDocument doc = QJsonDocument::fromJson(data,&error);
    if(error.error == QJsonParseError::NoError){
        if(doc.isNull() || doc.isEmpty()){
            cout<<" m_UserOrderList doc.isNUll() || doc.isEmpty() ";
            return;
        }
        if(doc.isObject()){
            QJsonObject obj = doc.object();
            QJsonArray array = obj.value("UserOrder").toArray();

            int size = array.size(); 

            for(int i = 0;i < size;++i){
                QJsonObject tmp = array[i].toObject();  
                UserOrderTableInfo *info = new UserOrderTableInfo;
                info->OrderID = tmp.value("OrderID").toInt();
                info->ProductName = tmp.value("ProductName").toString();
                info->count = tmp.value("count").toInt();
                info->timestamp = tmp.value("timestamp").toString();

                //List添加节点
                m_UserOrderList.append(info);
            }
        }
    }else{
        cout<<"getUserOrderJsonInfo error = "<<error.errorString();
    }
}

QByteArray UserOrderTable::setGetCountJson(QString user, QString token)
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

QByteArray UserOrderTable::setUserOrderListJson(QString user, QString token, int start, int count)
{
    QMap<QString,QVariant> tmp;
    tmp.insert("user",user);
    tmp.insert("token",token);
    tmp.insert("start",start);
    tmp.insert("count",count);

    QJsonDocument jsonDocument = QJsonDocument::fromVariant(tmp);
    if( jsonDocument.isNull()){
        cout<<"setUserOrderListJson jsonDocument.isNull()";
        return "";
    }

    return jsonDocument.toJson();
}

QByteArray UserOrderTable::setUploadJson()
{
    QModelIndexList selectedRows = m_tableWidget->selectionModel()->selectedRows();
    QMap<QString,QVariant> tmp;
    tmp.insert("OrderID",UserOrder_OrderID_Edit->text());
    tmp.insert("ProductName",UserOrder_ProductName_Edit->text());
    tmp.insert("count",UserOrder_Count_Edit->text().toInt());

    QJsonDocument jsonDocument = QJsonDocument::fromVariant(tmp);
    if(jsonDocument.isNull()){
        cout<<"setUploadJson jsonDocument.isNull()";
        return "";
    }
    return jsonDocument.toJson();
}

QByteArray UserOrderTable::setSelectJson()
{
    QMap<QString,QVariant> tmp;
    QModelIndexList selectedRows = m_tableWidget->selectionModel()->selectedRows();
    foreach (QModelIndex index, selectedRows) {
        int row = index.row();
        tmp.insert("OrderID",m_tableWidget->item(row, 0)->text());
        tmp.insert("ProductName",m_tableWidget->item(row, 1)->text());
        tmp.insert("count",m_tableWidget->item(row, 2)->text().toInt());
    }
    QJsonDocument jsonDocument = QJsonDocument::fromVariant(tmp);
    if(jsonDocument.isNull()){
        cout<<"setSelectJson jsonDocument.isNull()";
        return "";
    }
    return jsonDocument.toJson();
}

void UserOrderTable::search()
{
    if(Search_LineEdit->text().isEmpty()){
        refreshTable();
        return;
    }
    // 清空文件列表信息
    clearUserOrderList();

    //将之前的UserOrderlist清空
    clearUserOrderItems();

    m_tableWidget->setRowCount(0);

    //获取商品信息数目
    m_SearchCount = 0;
    QNetworkRequest request;

    LoginInfoInstance *login = LoginInfoInstance::getInstance();

    // 127.0.0.1:80/UserOrder?cmd=search
    // 获取商品信息数目
    QString url = QString("http://%1:%2/UserOrder?cmd=userOrdersearch=%3=%4").arg(login->getIp()).arg(login->getPort()).arg(QString::fromUtf8(UserName.toUtf8())).arg(QString::fromUtf8(Search_LineEdit->text().toUtf8().toBase64()));
    request.setUrl(QUrl(url));
    qDebug()<<"UserOrderSearch : "<<url;
    
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

        m_SearchCount = list.at(1).toLong();

        if(m_SearchCount > 0){
            s_start = 0;
            s_count = 10;
            getSearchList();
        }else{
            refreshUserOrderItems();
        }
    });
}

void UserOrderTable::remove()
{
    QByteArray array = setSelectJson();

    QNetworkRequest request;
    LoginInfoInstance *login = LoginInfoInstance::getInstance();
    QString url= QString("http://%1:%2/UserOrder?cmd=userOrderdelete=%3").arg(login->getIp()).arg(login->getPort()).arg(QString::fromUtf8(UserName.toUtf8()));

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

void UserOrderTable::update()
{
    cur_status = update_status;
    initEditWidget();
    QModelIndexList selectedRows = m_tableWidget->selectionModel()->selectedRows();
    foreach (QModelIndex index, selectedRows) {
        int row = index.row();
        UserOrder_OrderID_Edit->setText(m_tableWidget->item(row, 0)->text());
        UserOrder_ProductName_Edit->setText(m_tableWidget->item(row, 1)->text());
        UserOrder_Count_Edit->setText(m_tableWidget->item(row, 2)->text());
    }
    UserOrder_Edit->show();
}

void UserOrderTable::update_save_info()
{
    QByteArray array = setUploadJson();

    QNetworkRequest request;
    LoginInfoInstance *login = LoginInfoInstance::getInstance();
    QString url;
    if(update_status == cur_status){
        url = QString("http://%1:%2/UserOrder?cmd=userOrderupdate=%3").arg(login->getIp()).arg(login->getPort()).arg(QString::fromUtf8(UserName.toUtf8()));
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
            UserOrder_ProductName_Edit->clear();
            UserOrder_Count_Edit->clear();
            UserOrder_Edit->hide();
            refreshTable();
        }else{
            QMessageBox::warning(this,"上传失败","上传失败");
            UserOrder_Edit->hide();
        }
        delete reply;
    });
}

void UserOrderTable::cancel()
{
    UserOrder_ProductName_Edit->clear();
    UserOrder_Count_Edit->clear();
    UserOrder_Edit->hide();
}
