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
    //http管理类对象
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
    // 创建按钮和搜索框
    Search_Btn = new QPushButton(tr("搜索"), this);
    Search_LineEdit = new QLineEdit(this);
    Add_Btn = new QPushButton(tr("添加"), this);
    Delete_Btn = new QPushButton(tr("删除"), this);
    Update_Btn = new QPushButton(tr("更新"), this);

    // 连接按钮的点击信号到槽函数
    connect(Search_Btn, &QPushButton::clicked, this, &Wares::search);
    connect(Add_Btn, &QPushButton::clicked, this, &Wares::add);
    connect(Delete_Btn, &QPushButton::clicked, this, &Wares::remove);
    connect(Update_Btn, &QPushButton::clicked, this, &Wares::update);

    m_tableWidget = new QTableWidget(this);
    // 创建原料表格
    m_tableWidget->setColumnCount(6);
    m_tableWidget->setHorizontalHeaderLabels(QStringList() <<"原料编号"<<"原料名称"<<"计量单位"<<"原料库存"<<"计价单位"<<"单价");
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
    hLayout->addWidget(Add_Btn);
    hLayout->addWidget(Delete_Btn);
    hLayout->addWidget(Update_Btn);

    // 将水平布局和原料表添加到垂直布局中
    vLayout->addLayout(hLayout);
    vLayout->addWidget(m_tableWidget);

    //显示原料信息
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

// 得到服务器json文件
QStringList Wares::getCountStatus(QByteArray json)
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

// 显示用户的文件列表
void Wares::refreshTable()
{
    // 清空文件列表信息
    clearWaresList();

    //将之前的wareslist清空
    clearWaresItems();

    //将表格行数清零
    m_tableWidget->setRowCount(0);

    //获取原料信息数目
    m_WaresCount = 0;

    QNetworkRequest request;

    // 获取登陆信息实例
    // 获取单例
    LoginInfoInstance *login = LoginInfoInstance::getInstance();

    // 127.0.0.1:80/wares?cmd=warescount
    // 获取原料信息数目
    QString url = QString("http://%1:%2/wares?cmd=warescount").arg(login->getIp()).arg(login->getPort());
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
        m_WaresCount = list.at(1).toLong();

        // 清空文件列表信息
        clearWaresList();

        if(m_WaresCount > 0){
            // 说明任然有原料
            m_start = 0;    //从0开始
            m_count = 10;   //每次请求10个

            // 获取新的原料列表信息
            getWaresList();
        }else{//没有原料
            refreshWaresItems(); //更新Items
        }
    });
}

// 清空原料列表
void Wares::clearWaresList()
{
    m_tableWidget->clear();
}

// 清空所有原料Item
void Wares::clearWaresItems()
{
    m_waresList.clear();
}

// 原料Item展示
void Wares::refreshWaresItems()
{
    //如果文件列表不为空，显示原料列表
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
    // 遍历数目，结束条件处理
    if(m_WaresCount <= 0){ // 函数递归的结束条件
        refreshWaresItems();// 更新表单
        return;
    }else if(m_count > m_WaresCount) // 如果请求文件数量大于原料数目
    {
        m_count = m_WaresCount;
    }

    QNetworkRequest request; // 请求对象

    //获取登录信息实例
    LoginInfoInstance *login = LoginInfoInstance::getInstance();    // 获取单例

    QString tmp = QString("waresnormal");

    QString url = QString("http://%1:%2/wares?cmd=%3").arg(login->getIp()).arg(login->getPort()).arg(tmp);

    request.setUrl(QUrl(url));

    // 设置请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = setWaresListJson(login->getUser(),login->getToken(),m_start,m_count);

    //改变文件起始点位置
    m_start += m_count;
    m_WaresCount -= m_count;

    //发送post请求
    QNetworkReply * reply = m_manager->post(request,data);
    if(reply == NULL){
        return;
    }

    //获取请求的数据完成时，就会发送信号SIGNAL(finished())
    connect(reply,&QNetworkReply::finished,[=](){
       if(reply->error() != QNetworkReply::NoError){
           reply->deleteLater(); // 释放资源
           return;
       }

       // 服务器返回用户的数据
       QByteArray array = reply->readAll();
       //cout<<" wares info: "<<array;

       reply->deleteLater();

       //token验证失败
       if("111" == m_cm.getCode(array)){
           QMessageBox::warning(this,"账户异常","请重新登录！");

           return;
       }

       // 不是错误码就处理文件列表json信息
       if("015" != m_cm.getCode(array)){
           // 解析原料列表json信息，存放在文件列表中
           getWaresJsonInfo(array);

           //继续获取原料信息列表
           getWaresList();
       }
    });
}

void Wares::getSearchList()
{
    //遍历数目，结束条件处理
    if(m_SearchCount <= 0) //结束条件，这个条件很重要，函数递归的结束条件
    {
        refreshWaresItems(); //更新item
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

    url = QString("http://%1:%2/wares?cmd=waresresult").arg(login->getIp()).arg(login->getPort());
    request.setUrl(QUrl( url )); //设置url

    //qt默认的请求头
    //request.setRawHeader("Content-Type","text/html");
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = setWaresListJson( login->getUser(), login->getToken(), s_start, s_count);

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
            // 解析原料列表json信息，存放在文件列表中
            getWaresJsonInfo(array);

            //继续获取原料信息列表
            getSearchList();
        }
    });
}

// 解析原料列表json信息，存放在文件列表中
void Wares::getWaresJsonInfo(QByteArray data)
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
            QJsonArray array = obj.value("wares").toArray();

            int size = array.size(); // 数组个数

            for(int i = 0;i < size;++i){
                QJsonObject tmp = array[i].toObject();  // 取第i个对象
                WaresInfo *info = new WaresInfo;
                info->wares_id = tmp.value("wares_id").toInt();
                info->wares_name = tmp.value("wares_name").toString();
                info->wares_store_unit = tmp.value("wares_store_unit").toString();
                info->wares_amount = tmp.value("wares_amount").toInt();
                info->wares_sell_unit = tmp.value("wares_sell_unit").toString();
                info->wares_price = tmp.value("wares_price").toDouble();

                //List添加节点
                m_waresList.append(info);

            }
        }
    }else{
        cout<<"getWaresJsonInfo error = "<<error.errorString();
    }
}

// 设置json包
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

//设置json包
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

//设置上传json包
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

//搜索原料信息
void Wares::search()
{
    // 清空文件列表信息
    clearWaresList();

    //将之前的wareslist清空
    clearWaresItems();

    //将表格行数清零
    m_tableWidget->setRowCount(0);

    //获取原料信息数目
    m_SearchCount = 0;

    QNetworkRequest request;

    // 获取登陆信息实例
    // 获取单例
    LoginInfoInstance *login = LoginInfoInstance::getInstance();

    // 127.0.0.1:80/wares?cmd=search
    // 获取原料信息数目
    QString url = QString("http://%1:%2/wares?cmd=waressearch=%3").arg(login->getIp()).arg(login->getPort()).arg(QString::fromUtf8(Search_LineEdit->text().toUtf8().toBase64()));
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
            // 说明任然有原料
            s_start = 0;    //从0开始
            s_count = 10;   //每次请求10个

            // 获取新的原料列表信息
            getSearchList();
        }else{//没有文件
            refreshWaresItems(); //更新表
        }
    });
}

//添加原料信息
void Wares::add()
{
    cur_status = add_status;
    initEditWidget();
    Wares_Edit->show();
}

//将选中的表数据的行只作为json包
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

//删除原料信息
void Wares::remove()
{
    //将要上传的信息打包为json格式.
    QByteArray array = setSelectJson();

    QNetworkRequest request;
    LoginInfoInstance *login = LoginInfoInstance::getInstance();
    QString url= QString("http://%1:%2/wares?cmd=waresdelete").arg(login->getIp()).arg(login->getPort());

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

//更新原料信息
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
    //将要上传的信息打包为json格式.
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
