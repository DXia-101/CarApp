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
            QJsonObject obj = doc.object();
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
    
    
    LoginInfoInstance *login = LoginInfoInstance::getInstance();

    // 127.0.0.1:80/showpro?cmd=search
    // 获取商品信息数目
    QString url = QString("http://%1:%2/showpro?cmd=showprocount").arg(login->getIp()).arg(login->getPort());
    request.setUrl(QUrl(url));

    
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = setGetCountJson(login->getUser(),login->getToken());
    qDebug()<<"Data is :"<<data;

    
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
        cout<<" server return file: "<<array;

        reply->deleteLater();

        
        QStringList list = getCountStatus(array);

        
        if(list.at(0) == "111"){
            QMessageBox::warning(this,"账户异常","请重新登录!");
            //emit
            return;
        }

        
        m_ShowProCount = list.at(1).toLong();
        cout<<"userSearchCount = " << m_ShowProCount;

        if(m_ShowProCount > 0){
            m_start = 0;
            m_count = 10;
            getShowProList();
        }else{
            refreshshowproItems();
        }
    });
}

void HomePage::initServerSQL()
{
    QNetworkRequest request; 

}


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
    
    if(m_ShowProCount <= 0) 
    {
        cout << "获取展示商品列表条件结束";
        refreshshowproItems(); //更新item
        return;
    }
    else if(m_count > m_ShowProCount) 
    {
        m_count = m_ShowProCount;
    }


    QNetworkRequest request; 

    
    LoginInfoInstance *login = LoginInfoInstance::getInstance();

    QString url;

    url = QString("http://%1:%2/showpro?cmd=showpronormal").arg(login->getIp()).arg(login->getPort());
    request.setUrl(QUrl( url )); 
    cout << "search url: " << url;

    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = setshowproListJson( login->getUser(), login->getToken(), m_start, m_count);

    
    m_start += m_count;
    m_ShowProCount -= m_count;

    
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
            getshowproJsonInfo(array);

            getShowProList();
        }
    });
}

void HomePage::getshowproJsonInfo(QByteArray data)
{
    QJsonParseError error;

    
    
    QJsonDocument doc = QJsonDocument::fromJson(data,&error);
    if(error.error == QJsonParseError::NoError){
        if(doc.isNull() || doc.isEmpty()){
            cout<<" m_showproList doc.isNUll() || doc.isEmpty() ";
            return;
        }
        if(doc.isObject()){
            
            QJsonObject obj = doc.object();
            QJsonArray array = obj.value("showpro").toArray();

            int size = array.size(); 
            cout<<"size = "<<size;

            for(int i = 0;i < size;++i){
                QJsonObject tmp = array[i].toObject();  
                ShowProInfo *info = new ShowProInfo;
                info->showpro_id = tmp.value("showpro_id").toInt();
                info->showpro_name = tmp.value("showpro_name").toString();
                info->showpro_url = tmp.value("showpro_url").toString();
                m_showproList.append(info);
            }
        }
    }else{
        cout<<"getshowproJsonInfo error = "<<error.errorString();
    }
}

void HomePage::refreshshowproItems()
{
    if(m_showproList.isEmpty() == false){
        int n = m_showproList.size();
        for(int i = 0;i < n;++i){
            ShowProInfo *tmp = m_showproList.at(i);
            ProductItem* proItem = new ProductItem(tmp->showpro_id,tmp->showpro_url,tmp->showpro_name);
            layout->addWidget(proItem);
            totalHeight += proItem->height();
        }
    }
    layout->addStretch();
    ui->scrollAreaWidgetContents->setGeometry(0,0,400,totalHeight);
}
