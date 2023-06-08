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
#include "common/logininfoinstance.h"

Wares::Wares(QWidget *parent)
    : QWidget{parent}
{
    qDebug()<<"进入商品类构造函数";
    //http管理类对象
    m_manager = Common::getNetManager();
    //初始化商品信息表页面
    initTableWidget();
}

Wares::~Wares()
{
    if (m_manager) delete m_manager;
}

void Wares::initTableWidget()
{
    qDebug()<<"进入初始化商品信息表页面";
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
    // 创建商品表格
    m_tableWidget->setColumnCount(6);
    m_tableWidget->setHorizontalHeaderLabels({"原料编号","原料名","计量单位","原料库存","计价单位","单价"});

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

    // 将水平布局和商品表添加到垂直布局中
    vLayout->addLayout(hLayout);
    vLayout->addWidget(m_tableWidget);

    //显示商品信息
    refreshTable();

#if 0
    // 获取单例
    LoginInfoInstance *login = LoginInfoInstance::getInstance();

    QByteArray data = setGetCountJson(login->getUser(),login->getToken());

    qDebug()<<data;
#endif
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
    //获取商品信息数目
    m_WaresCount = 0;

    QNetworkRequest request;

    // 获取登陆信息实例
    // 获取单例
    LoginInfoInstance *login = LoginInfoInstance::getInstance();

    // 127.0.0.1:80/wares?cmd=warescount
    // 获取商品信息数目
    QString url = QString("http://%1:%2/wares?cmd=warescount").arg(login->getIp()).arg(login->getPort());
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

    // 获取请求的数据完成时，就会发送信号SIGNAL(finished())
//    connect(reply, &QNetworkReply::readyRead, [=] {
//        if (reply->error() == QNetworkReply::NoError) {
//            qDebug() << reply->readAll();
//        }else{
//            qDebug()<<reply->errorString();
//        }
//        reply->deleteLater();
//    });
    connect(reply,&QNetworkReply::finished,[=](){
        if(reply->error() != QNetworkReply::NoError)//出错
        {
            cout<<reply->errorString();
            reply->deleteLater();//释放资源
            return;
        }
        //服务器返回数据
        QByteArray array = reply->readAll();
        cout<<" server return file: "<<array;

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
        cout<<"userWaresCount = " << m_WaresCount;

        // 清空文件列表信息
        clearWaresList();

        if(m_WaresCount > 0){
            // 说明任然有商品
            m_start = 0;    //从0开始
            m_count = 10;   //每次请求10个

            // 获取新的商品列表信息
            getWaresList();
        }else{//没有商品
            refreshWaresItems(); //更新Items
        }
    });
}

// 清空商品列表
void Wares::clearWaresList()
{
    m_tableWidget->clear();
}

// 清空所有商品Item
void Wares::clearWaresItems()
{
    m_waresList.clear();
}

// 商品Item展示
void Wares::refreshWaresItems()
{
    // 清空所有商品Item
    clearWaresList();

    //如果文件列表不为空，显示商品列表
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
        cout<< "获取用户文件列表的条件结束";
        refreshWaresItems();// 更新表单
        return;
    }else if(m_count > m_WaresCount) // 如果请求文件数量大于商品数目
    {
        m_count = m_WaresCount;
    }

    QNetworkRequest request; // 请求对象

    //获取登录信息实例
    LoginInfoInstance *login = LoginInfoInstance::getInstance();    // 获取单例

    QString tmp = QString("waresnormal");

    QString url = QString("http://%1:%2/wares?cmd=%3").arg(login->getIp()).arg(login->getPort()).arg(tmp);

    request.setUrl(QUrl(url));

    cout<<"Wares url: "<<url;

    // 设置请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    /*
    {
        "user": "yoyo"
        "token": "xxxx"
        "start": 0
        "count": 10
    }
    */

    QByteArray data = setWaresListJson(login->getUser(),login->getToken(),m_start,m_count);

    //改变文件起始点位置
    m_start += m_count;
    m_WaresCount -= m_count;

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
       cout<<" wares info: "<<array;

       reply->deleteLater();

       //token验证失败
       if("111" == m_cm.getCode(array)){
           QMessageBox::warning(this,"账户异常","请重新登录！");

           return;
       }

       // 不是错误码就处理文件列表json信息
       if("015" != m_cm.getCode(array)){
           // 解析商品列表json信息，存放在文件列表中
           getWaresJsonInfo(array);

           //继续获取商品信息列表
           getWaresList();
       }
    });
}

void Wares::getSearchList()
{
    //遍历数目，结束条件处理
    if(m_WaresCount <= 0) //结束条件，这个条件很重要，函数递归的结束条件
    {
        cout << "获取用户文件列表条件结束";
        refreshWaresItems(); //更新item
        return; //中断函数
    }
    else if(m_count > m_WaresCount) //如果请求文件数量大于用户的文件数目
    {
        m_count = m_WaresCount;
    }


    QNetworkRequest request; //请求对象

    // 获取登陆信息实例
    LoginInfoInstance *login = LoginInfoInstance::getInstance(); //获取单例

    QString url;

    url = QString("http://%1:%2/myfiles?cmd=waresresult").arg(login->getIp()).arg(login->getPort());
    request.setUrl(QUrl( url )); //设置url
    cout << "search url: " << url;

    //qt默认的请求头
    //request.setRawHeader("Content-Type","text/html");
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    /*
    {
        "user": "yoyo"
        "token": "xxxx"
        "start": 0
        "count": 10
    }
    */
    QByteArray data = setWaresListJson( login->getUser(), login->getToken(), m_start, m_count);

    //改变文件起点位置
    m_start += m_count;
    m_WaresCount -= m_count;

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
        cout<<" wares search info: "<<array;

        reply->deleteLater();

        //token验证失败
        if("111" == m_cm.getCode(array)){
            QMessageBox::warning(this,"账户异常","请重新登录！");

            return;
        }

        // 不是错误码就处理文件列表json信息
        if("015" != m_cm.getCode(array)){
            // 解析商品列表json信息，存放在文件列表中
            getWaresJsonInfo(array);

            //继续获取商品信息列表
            getSearchList();
        }
    });
}

// 解析商品列表json信息，存放在文件列表中
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
            cout<<"size = "<<size;

            for(int i = 0;i < size;++i){
                QJsonObject tmp = array[i].toObject();  // 取第i个对象
                // 商品信息
                /*struct WaresInfo{
                    int wares_id;
                    QString wares_name;
                    QString wares_store_unit;
                    quint16 wares_amount;
                    QString wares_sell_unit;
                    double wares_price;
                };
                {
                "wares_id":"1",
                "wares_name":"黑色无纺布",
                "wares_store_unit":"米/元",
                "wares_amount":2000,
                "wares_sell_unit":"米/元",
                "wares_price":2000
                }
                */
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

//搜索商品信息
void Wares::search()
{
    //获取商品信息数目
    m_WaresCount = 0;

    QNetworkRequest request;

    // 获取登陆信息实例
    // 获取单例
    LoginInfoInstance *login = LoginInfoInstance::getInstance();

    // 127.0.0.1:80/wares?cmd=search
    // 获取商品信息数目
    QString url = QString("http://%1:%2/wares?cmd=waressearch=%3").arg(login->getIp()).arg(login->getPort()).arg(Search_LineEdit->text());
    request.setUrl(QUrl(url));

    cout<<"Search url: "<<url;

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
        cout<<" server return file: "<<array;

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
        cout<<"userWaresCount = " << m_WaresCount;

        // 清空文件列表信息
        clearWaresList();

        //将之前的wareslist清空
        clearWaresItems();

        if(m_WaresCount > 0){
            // 说明任然有商品
            m_start = 0;    //从0开始
            m_count = 10;   //每次请求10个

            // 获取新的商品列表信息
            getSearchList();
        }else{//没有文件
            refreshWaresItems(); //更新表
        }
    });
}

//添加商品信息
void Wares::add()
{

}

//删除商品信息
void Wares::remove()
{

}

//更新商品信息
void Wares::update()
{

}
