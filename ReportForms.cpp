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

    // 连接按钮的点击信号到槽函数
    connect(Search_Btn, &QPushButton::clicked, this, &ReportForms::search);
    connect(Update_Btn, &QPushButton::clicked, this, &ReportForms::update);

    m_tableWidget = new QTableWidget(this);
    // 创建报表表格
    m_tableWidget->setColumnCount(5);
    m_tableWidget->setHorizontalHeaderLabels({"订单编号","客户名称","订购日期","发货日期","交付状态"});
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
    hLayout->addWidget(Update_Btn);

    // 将水平布局和报表表添加到垂直布局中
    vLayout->addLayout(hLayout);
    vLayout->addWidget(m_tableWidget);

    //显示报表信息
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
            QJsonObject obj = doc.object();//取得最外层这个大对象
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

    //将表格行数清零
    m_tableWidget->setRowCount(0);

    //获取报表信息数目
    m_ReportFormCount = 0;

    QNetworkRequest request;

    // 获取登陆信息实例
    // 获取单例
    LoginInfoInstance *login = LoginInfoInstance::getInstance();

    // 127.0.0.1:80/reportforms?cmd=reportformscount
    // 获取报表信息数目
    QString url = QString("http://%1:%2/ReportForm?cmd=ReportFormCount").arg(login->getIp()).arg(login->getPort());
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
        m_ReportFormCount = list.at(1).toLong();

        clearReportFormList();

        if(m_ReportFormCount > 0){
            // 说明任然有报表
            m_start = 0;    //从0开始
            m_count = 10;   //每次请求10个

            // 获取新的报表列表信息
            getReportFormList();
        }else{//没有报表
            refreshReportFormItems(); //更新Items
        }
    });
}

void ReportForms::clearReportFormList()
{
    m_tableWidget->clear();
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
    LoginInfoInstance *login = LoginInfoInstance::getInstance();    // 获取单例

    QString url = QString("http://%1:%2/ReportForm?cmd=ReportFormNormal").arg(login->getIp()).arg(login->getPort());

    request.setUrl(QUrl(url));

    // 设置请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = setReportFormListJson(login->getUser(),login->getToken(),m_start,m_count);

    //改变文件起始点位置
    m_start += m_count;
    m_ReportFormCount -= m_count;

    //发送post请求
    QNetworkReply * reply = m_manager->post(request,data);
    if(reply == NULL){
        cout<<"getWaresList reply == NULL";
        return;
    }

    //获取请求的数据完成时，就会发送信号SIGNAL(finished())
    connect(reply,&QNetworkReply::finished,[=](){
       if(reply->error() != QNetworkReply::NoError){
           cout<<"getWaresList error: "<<reply->errorString();
           reply->deleteLater(); // 释放资源
           return;
       }

       // 服务器返回用户的数据
       QByteArray array = reply->readAll();
       //qDebug()<<"getReportFormList Array:"<<array;

       reply->deleteLater();

       //token验证失败
       if("111" == m_cm.getCode(array)){
           QMessageBox::warning(this,"账户异常","请重新登录！");
           return;
       }

       // 不是错误码就处理文件列表json信息
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
    //遍历数目，结束条件处理
    if(m_SearchCount <= 0) //结束条件，这个条件很重要，函数递归的结束条件
    {
        cout << "获取用户文件列表条件结束";
        refreshReportFormItems(); //更新item
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

    url = QString("http://%1:%2/ReportForm?cmd=ReportFormResult").arg(login->getIp()).arg(login->getPort());
    request.setUrl(QUrl( url )); //设置url

    //qt默认的请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = setReportFormListJson( login->getUser(), login->getToken(), s_start, s_count);

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

    // 将来源数据json转化为JsonDocument
    // 由QByteArray 对象构成一个QJsonDocument对象，用于读写操作
    QJsonDocument doc = QJsonDocument::fromJson(data,&error);
    if(error.error == QJsonParseError::NoError){
        if(doc.isNull() || doc.isEmpty()){
            cout<<" m_waresList doc.isNUll() || doc.isEmpty() ";
            return;
        }
        if(doc.isObject()){
            //QJsonObject json对象，描述json数据中{}括起来部分
            QJsonObject obj = doc.object();//取得最外层这个大对象

            //获取wares对应的数组
            //QJsonArray json数组,描述json数据中[]括起来的部分
            QJsonArray array = obj.value("ReportForms").toArray();

            int size = array.size(); // 数组个数

            for(int i = 0;i < size;++i){
                QJsonObject tmp = array[i].toObject();  // 取第i个对象
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
    // 清空文件列表信息
    clearReportFormList();

    //将之前的wareslist清空
    clearReportFormItems();

    //将表格行数清零
    m_tableWidget->setRowCount(0);

    //获取报表信息数目
    m_SearchCount = 0;

    QNetworkRequest request;

    // 获取登陆信息实例
    // 获取单例
    LoginInfoInstance *login = LoginInfoInstance::getInstance();

    // 127.0.0.1:80/wares?cmd=search
    // 获取报表信息数目
    QString url = QString("http://%1:%2/ReportForm?cmd=ReportFormSearch=%3").arg(login->getIp()).arg(login->getPort()).arg(QString::fromUtf8(Search_LineEdit->text().toUtf8().toBase64()));
    request.setUrl(QUrl(url));

    // qt默认的请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = setGetCountJson(login->getUser(),login->getToken());
    qDebug()<<data;

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
        cout<<"userSearchCount = " << m_SearchCount;


        if(m_SearchCount > 0){
            // 说明任然有报表
            s_start = 0;    //从0开始
            s_count = 10;   //每次请求10个

            // 获取新的报表列表信息
            getSearchList();
        }else{//没有文件
            refreshReportFormItems(); //更新表
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
    //将要上传的信息打包为json格式.
    QByteArray array = setUploadJson();
//    qDebug()<<"上传的array包: "<<array;

    QNetworkRequest request;
    LoginInfoInstance *login = LoginInfoInstance::getInstance();
    QString url;

    url = QString("http://%1:%2/ReportForm?cmd=ReportFormUpdate").arg(login->getIp()).arg(login->getPort());


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
        qDebug()<<QString(jsonData);
        // 判断状态码
        if("020" == m_cm.getCode(jsonData))
        {
            //上传成功
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
