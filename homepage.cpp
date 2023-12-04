#include "homepage.h"
#include "ui_homepage.h"

#include <QScrollArea>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "common/logininfoinstance.h"

#include "productitem.h"

HomePage::HomePage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HomePage)
{
    ui->setupUi(this);

    m_manager = Common::getNetManager();

    layout = new QVBoxLayout(ui->scrollArea);
    ui->scrollAreaWidgetContents->setLayout(layout);

    // 初始化listWidget文件列表
    initListWidget();
}

HomePage::~HomePage()
{
    delete ui;
}

// 设置json包
QByteArray HomePage::setGetCountJson(QString user, QString token)
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

QStringList HomePage::getCountStatus(QByteArray json)
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

void HomePage::initListWidget()
{
    QNetworkRequest request;

    m_ShowProCount = 0;
    // 获取登陆信息实例
    // 获取单例
    LoginInfoInstance *login = LoginInfoInstance::getInstance();

    // 127.0.0.1:80/showpro?cmd=search
    // 获取商品信息数目
    QString url = QString("http://%1:%2/showpro?cmd=showprocount").arg(login->getIp()).arg(login->getPort());
    request.setUrl(QUrl(url));

    // qt默认的请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = setGetCountJson(login->getUser(),login->getToken());
    qDebug()<<"Data is :"<<data;

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
        m_ShowProCount = list.at(1).toLong();
        cout<<"userSearchCount = " << m_ShowProCount;

        if(m_ShowProCount > 0){
            // 说明任然有商品
            m_start = 0;    //从0开始
            m_count = 10;   //每次请求10个

            // 获取新的商品列表信息
            getShowProList();
        }else{//没有文件
            refreshshowproItems(); //更新表
        }
    });
}

//初始化服务器端的数据库，如果该用户未建立数据库则建立，若未建立则建立。
void HomePage::initServerSQL()
{
    QNetworkRequest request; //请求对象

}

//设置json包
QByteArray HomePage::setshowproListJson(QString user, QString token, int start, int count)
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

void HomePage::getShowProList()
{
    //遍历数目，结束条件处理
    if(m_ShowProCount <= 0) //结束条件，这个条件很重要，函数递归的结束条件
    {
        cout << "获取展示商品列表条件结束";
        refreshshowproItems(); //更新item
        return; //中断函数
    }
    else if(m_count > m_ShowProCount) //如果请求文件数量大于用户的文件数目
    {
        m_count = m_ShowProCount;
    }


    QNetworkRequest request; //请求对象

    // 获取登陆信息实例
    LoginInfoInstance *login = LoginInfoInstance::getInstance(); //获取单例

    QString url;

    url = QString("http://%1:%2/showpro?cmd=showpronormal").arg(login->getIp()).arg(login->getPort());
    request.setUrl(QUrl( url )); //设置url
    cout << "search url: " << url;

    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = setshowproListJson( login->getUser(), login->getToken(), m_start, m_count);

    //改变文件起点位置
    m_start += m_count;
    m_ShowProCount -= m_count;

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
        //cout<<" showpro search info: "<<array;

        reply->deleteLater();

        //token验证失败
        if("111" == m_cm.getCode(array)){
            QMessageBox::warning(this,"账户异常","请重新登录！");

            return;
        }

        // 不是错误码就处理文件列表json信息
        if("015" != m_cm.getCode(array)){
            // 解析商品列表json信息，存放在文件列表中
            getshowproJsonInfo(array);

            //继续获取商品信息列表
            getShowProList();
        }
    });
}

void HomePage::getshowproJsonInfo(QByteArray data)
{
    QJsonParseError error;

    // 将来源数据json转化为JsonDocument
    // 由QByteArray 对象构成一个QJsonDocument对象，用于读写操作
    QJsonDocument doc = QJsonDocument::fromJson(data,&error);
    if(error.error == QJsonParseError::NoError){
        if(doc.isNull() || doc.isEmpty()){
            cout<<" m_showproList doc.isNUll() || doc.isEmpty() ";
            return;
        }
        if(doc.isObject()){
            //QJsonObject json对象，描述json数据中{}括起来部分
            QJsonObject obj = doc.object();//取得最外层这个大对象

            //获取showpro对应的数组
            //QJsonArray json数组,描述json数据中[]括起来的部分
            QJsonArray array = obj.value("showpro").toArray();

            int size = array.size(); // 数组个数
            cout<<"size = "<<size;

            for(int i = 0;i < size;++i){
                QJsonObject tmp = array[i].toObject();  // 取第i个对象
                ShowProInfo *info = new ShowProInfo;
                info->showpro_id = tmp.value("showpro_id").toInt();
                info->showpro_name = tmp.value("showpro_name").toString();
                info->showpro_url = tmp.value("showpro_url").toString();
                //List添加节点
                m_showproList.append(info);
            }
        }
    }else{
        cout<<"getshowproJsonInfo error = "<<error.errorString();
    }
}

// 商品Item展示
void HomePage::refreshshowproItems()
{
    //如果文件列表不为空，显示商品列表
    if(m_showproList.isEmpty() == false){
        int n = m_showproList.size();
        for(int i = 0;i < n;++i){
            ShowProInfo *tmp = m_showproList.at(i);
            ProductItem* proItem = new ProductItem(tmp->showpro_id,tmp->showpro_url,tmp->showpro_name);
            layout->addWidget(proItem);
            totalHeight += proItem->height();
        }
    }
    ui->scrollAreaWidgetContents->setGeometry(0,0,400,totalHeight);
}
