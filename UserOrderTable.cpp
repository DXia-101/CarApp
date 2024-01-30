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
    m_tableWidget->setHorizontalHeaderLabels({"订单ID","商品名称","订购数量","订购时间"});
    //禁止单元格编辑
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    //设置表格选择整行
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    //设置允许多个选择
    m_tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);


    // 创建垂直布局和水平布局
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    QHBoxLayout *hLayout = new QHBoxLayout();

    // 将按钮和搜索框添加到水平布局中
    hLayout->addWidget(Search_Btn);
    hLayout->addWidget(Search_LineEdit);
    hLayout->addStretch();
    hLayout->addWidget(Delete_Btn);
    hLayout->addWidget(Update_Btn);

    // 将水平布局和商品表添加到垂直布局中
    vLayout->addLayout(hLayout);
    vLayout->addWidget(m_tableWidget);

    //显示商品信息
    refreshTable();
}

void UserOrderTable::initEditWidget()
{
    UserOrder_Edit = new QWidget();

    QLabel *UserOrder_ProductName_Label = new QLabel(tr("商品名称"));
    QLabel *UserOrder_Count_Label = new QLabel(tr("订购数量"));

    UserOrder_ProductName_Edit  = new QLineEdit();
    UserOrder_Count_Edit = new QLineEdit();
    update_Save_Btn = new QPushButton(tr("保存"));
    Edit_Cancel_Btn = new QPushButton(tr("取消"));
    connect(update_Save_Btn, &QPushButton::clicked, this, &UserOrderTable::update_save_info);
    connect(Edit_Cancel_Btn, &QPushButton::clicked, this, &UserOrderTable::cancel);

    QGridLayout *EditLayout = new QGridLayout();
    EditLayout->addWidget(UserOrder_ProductName_Label,0,0);
    EditLayout->addWidget(UserOrder_Count_Label,1,0);
    EditLayout->addWidget(UserOrder_ProductName_Edit,0,1);
    EditLayout->addWidget(UserOrder_Count_Edit,1,1);

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
            QJsonObject obj = doc.object();//取得最外层这个大对象
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

    //将表格行数清零
    m_tableWidget->setRowCount(0);

    //获取商品信息数目
    m_UserOrderCount = 0;

    QNetworkRequest request;

    // 获取登陆信息实例
    // 获取单例
    LoginInfoInstance *login = LoginInfoInstance::getInstance();

    // 127.0.0.1:80/UserOrder?cmd=UserOrdercount
    // 获取商品信息数目
    QString url = QString("http://%1:%2/UserOrder?cmd=userOrdercount=%3").arg(login->getIp()).arg(login->getPort()).arg(QString::fromUtf8(UserName.toUtf8()));
    request.setUrl(QUrl(url));
    request.setUrl(QUrl(url));

    // qt默认的请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    QByteArray data = setGetCountJson(login->getUser(),login->getToken());

    //发送post请求
    QNetworkReply* reply = m_manager->post(request,data);
    if(reply == NULL){
        qDebug()<<"reply == NULL";
        return;
    }
    connect(reply,&QNetworkReply::finished,[=](){
        if(reply->error() != QNetworkReply::NoError)//出错
        {
            qDebug()<<reply->errorString();
            reply->deleteLater();//释放资源
            return;
        }
        //服务器返回数据
        QByteArray array = reply->readAll();

        reply->deleteLater();

        // 得到服务器json文件
        QStringList list = getCountStatus(array);

        // token验证失败
        if(list.at(0) == "111"){
            QMessageBox::warning(this,"账户异常","请重新登录!");
            return;
        }

        //转换为long
        m_UserOrderCount = list.at(1).toLong();

        // 清空文件列表信息
        clearUserOrderList();

        if(m_UserOrderCount > 0){
            // 说明任然有商品
            m_start = 0;    //从0开始
            m_count = 10;   //每次请求10个

            // 获取新的商品列表信息
            getUserOrderList();
        }else{//没有商品
            refreshUserOrderItems(); //更新Items
        }
    });
}

void UserOrderTable::clearUserOrderList()
{
    m_tableWidget->clear();
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
            m_tableWidget->setItem(row,0,new QTableWidgetItem(QString::number(tmp->UserOrderTable_OrderID)));
            m_tableWidget->setItem(row,1,new QTableWidgetItem(tmp->UserOrderTable_Productname));
            m_tableWidget->setItem(row,2,new QTableWidgetItem(QString::number(tmp->UserOrderTable_count)));
            m_tableWidget->setItem(row,3,new QTableWidgetItem(tmp->UserOrderTable_time));
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
    LoginInfoInstance *login = LoginInfoInstance::getInstance();    // 获取单例

    QString url = QString("http://%1:%2/UserOrder?cmd=userOrdernormal=%3").arg(login->getIp()).arg(login->getPort()).arg(QString::fromUtf8(UserName.toUtf8()));

    request.setUrl(QUrl(url));

    // 设置请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = setUserOrderListJson(login->getUser(),login->getToken(),m_start,m_count);

    //改变文件起始点位置
    m_start += m_count;
    m_UserOrderCount -= m_count;

    //发送post请求
    QNetworkReply * reply = m_manager->post(request,data);
    if(reply == NULL){
        cout<<"getUserOrderList reply == NULL";
        return;
    }

    //获取请求的数据完成时，就会发送信号SIGNAL(finished())
    connect(reply,&QNetworkReply::finished,[=](){
       if(reply->error() != QNetworkReply::NoError){
           cout<<"getUserOrderList error: "<<reply->errorString();
           reply->deleteLater(); // 释放资源
           return;
       }

       // 服务器返回用户的数据
       QByteArray array = reply->readAll();

       reply->deleteLater();

       //token验证失败
       if("111" == m_cm.getCode(array)){
           QMessageBox::warning(this,"账户异常","请重新登录！");

           return;
       }

       // 不是错误码就处理文件列表json信息
       if("015" != m_cm.getCode(array)){
           getUserOrderJsonInfo(array);

           //继续获取商品信息列表
           getUserOrderList();
       }
    });
}

void UserOrderTable::getSearchList()
{
    //遍历数目，结束条件处理
    if(m_SearchCount <= 0) //结束条件，这个条件很重要，函数递归的结束条件
    {
        refreshUserOrderItems(); //更新item
        return; //中断函数
    }
    else if(s_count > m_SearchCount) //如果请求文件数量大于用户的文件数目
    {
        s_count = m_SearchCount;
    }
    QNetworkRequest request; //请求对象
    // 获取登陆信息实例
    LoginInfoInstance *login = LoginInfoInstance::getInstance(); //获取单例

    QString url;

    url = QString("http://%1:%2/UserOrder?cmd=userOrderresult=%3").arg(login->getIp()).arg(login->getPort()).arg(QString::fromUtf8(UserName.toUtf8()));
    request.setUrl(QUrl( url ));

    //qt默认的请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = setUserOrderListJson( login->getUser(), login->getToken(), s_start, s_count);

    //改变文件起点位置
    s_start += s_count;
    m_SearchCount -= s_count;

    //发送post请求
    QNetworkReply * reply = m_manager->post( request, data );
    if(reply == NULL)
    {
        cout << "reply == NULL";
        return;
    }

    //获取请求的数据完成时，就会发送信号SIGNAL(finished())
    connect(reply, &QNetworkReply::finished, [=]()
    {
        if (reply->error() != QNetworkReply::NoError) //有错误
        {
            cout << reply->errorString();
            reply->deleteLater(); //释放资源
            return;
        }

        // 服务器返回用户的数据
        QByteArray array = reply->readAll();

        reply->deleteLater();

        //token验证失败
        if("111" == m_cm.getCode(array)){
            QMessageBox::warning(this,"账户异常","请重新登录！");
            return;
        }

        // 不是错误码就处理文件列表json信息
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

    // 将来源数据json转化为JsonDocument
    // 由QByteArray 对象构成一个QJsonDocument对象，用于读写操作
    QJsonDocument doc = QJsonDocument::fromJson(data,&error);
    if(error.error == QJsonParseError::NoError){
        if(doc.isNull() || doc.isEmpty()){
            cout<<" m_UserOrderList doc.isNUll() || doc.isEmpty() ";
            return;
        }
        if(doc.isObject()){
            //QJsonObject json对象，描述json数据中{}括起来部分
            QJsonObject obj = doc.object();//取得最外层这个大对象

            //获取UserOrder对应的数组
            //QJsonArray json数组,描述json数据中[]括起来的部分
            QJsonArray array = obj.value("UserOrder").toArray();

            int size = array.size(); // 数组个数

            for(int i = 0;i < size;++i){
                QJsonObject tmp = array[i].toObject();  // 取第i个对象
                UserOrderTableInfo *info = new UserOrderTableInfo;
                info->UserOrderTable_OrderID = tmp.value("UserOrderTable_OrderID").toInt();
                info->UserOrderTable_Productname = tmp.value("UserOrderTable_Productname").toString();
                info->UserOrderTable_count = tmp.value("UserOrderTable_count").toInt();
                info->UserOrderTable_time = tmp.value("UserOrderTable_time").toString();


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

    /*json数据如下
    {
        user:xxxx
        token: xxxx
    }
    */
    QJsonDocument jsonDocument = QJsonDocument::fromVariant(tmp);
    if ( jsonDocument.isNull() )
    {
        cout << "setGetCountJson jsonDocument.isNull() ";
        return "";
    }

    //cout << jsonDocument.toJson().data();
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
    QMap<QString,QVariant> tmp;
    tmp.insert("UserOrderTable_Productname",UserOrder_ProductName_Edit->text());
    tmp.insert("UserOrderTable_count",UserOrder_Count_Edit->text().toInt());


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
        tmp.insert("UserOrderTable_Productname",m_tableWidget->item(row, 0)->text());
        tmp.insert("UserOrderTable_count",m_tableWidget->item(row, 1)->text().toInt());
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
    // 清空文件列表信息
    clearUserOrderList();

    //将之前的UserOrderlist清空
    clearUserOrderItems();

    //将表格行数清零
    m_tableWidget->setRowCount(0);

    //获取商品信息数目
    m_SearchCount = 0;
    QNetworkRequest request;
    // 获取登陆信息实例
    // 获取单例
    LoginInfoInstance *login = LoginInfoInstance::getInstance();

    // 127.0.0.1:80/UserOrder?cmd=search
    // 获取商品信息数目
    QString url = QString("http://%1:%2/UserOrder?cmd=userOrdersearch=%3&%4").arg(login->getIp()).arg(login->getPort()).arg(QString::fromUtf8(UserName.toUtf8())),arg(QString::fromUtf8(Search_LineEdit->text().toUtf8().toBase64()));
    request.setUrl(QUrl(url));

    // qt默认的请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = setGetCountJson(login->getUser(),login->getToken());

    //发送post请求
    QNetworkReply* reply = m_manager->post(request,data);
    if(reply == NULL){
        qDebug()<<"reply == NULL";
        return;
    }

    connect(reply,&QNetworkReply::finished,[=](){
        if(reply->error() != QNetworkReply::NoError)//出错
        {
            cout<<reply->errorString();
            reply->deleteLater();//释放资源
            return;
        }
        //服务器返回数据
        QByteArray array = reply->readAll();

        reply->deleteLater();//释放

        // 得到服务器json文件
        QStringList list = getCountStatus(array);

        // token验证失败
        if(list.at(0) == "111"){
            QMessageBox::warning(this,"账户异常","请重新登录!");
            //emit
            return;
        }

        //转换为long
        m_SearchCount = list.at(1).toLong();

        if(m_SearchCount > 0){
            // 说明任然有商品
            s_start = 0;    //从0开始
            s_count = 10;   //每次请求10个

            // 获取新的商品列表信息
            getSearchList();
        }else{//没有文件
            refreshUserOrderItems(); //更新表
        }
    });
}

void UserOrderTable::remove()
{
    //将要上传的信息打包为json格式.
    QByteArray array = setSelectJson();

    QNetworkRequest request;
    LoginInfoInstance *login = LoginInfoInstance::getInstance();
    QString url= QString("http://%1:%2/UserOrder?cmd=userOrderdelete=%3").arg(login->getIp()).arg(login->getPort()).arg(QString::fromUtf8(UserName.toUtf8()));

    request.setUrl(QUrl(url));
    //设置请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json"));
    request.setHeader(QNetworkRequest::ContentLengthHeader,QVariant(array.size()));
    //发送数据
    QNetworkReply *reply = m_manager->post(request,array);

    // 判断请求是否被成功处理
    connect(reply, &QNetworkReply::readyRead, [=]()
    {
        // 读sever回写的数据
        QByteArray jsonData = reply->readAll();
        /*
            注册 - server端返回的json格式数据：
            成功:{"code":"023"}
            失败:{"code":"024"}
        */
        // 判断状态码
        if("023" == m_cm.getCode(jsonData))
        {
            //上传成功
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
        UserOrder_ProductName_Edit->setText(m_tableWidget->item(row, 0)->text());
        UserOrder_Count_Edit->setText(m_tableWidget->item(row, 1)->text());
    }
    UserOrder_Edit->show();
}

void UserOrderTable::update_save_info()
{
    //将要上传的信息打包为json格式.
    QByteArray array = setUploadJson();

    QNetworkRequest request;
    LoginInfoInstance *login = LoginInfoInstance::getInstance();
    QString url;
    if(update_status == cur_status){
        url = QString("http://%1:%2/UserOrder?cmd=userOrderupdate=%3").arg(login->getIp()).arg(login->getPort()).arg(QString::fromUtf8(UserName.toUtf8()));
    }

    request.setUrl(QUrl(url));
    //设置请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json"));
    request.setHeader(QNetworkRequest::ContentLengthHeader,QVariant(array.size()));
    //发送数据
    QNetworkReply *reply = m_manager->post(request,array);

    // 判断请求是否被成功处理
    connect(reply, &QNetworkReply::readyRead, [=]()
    {
        // 读sever回写的数据
        QByteArray jsonData = reply->readAll();
        /*
            注册 - server端返回的json格式数据：
            成功:{"code":"020"}
            失败:{"code":"021"}
        */
        // 判断状态码
        if("020" == m_cm.getCode(jsonData))
        {
            //上传成功
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
