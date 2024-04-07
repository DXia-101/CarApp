#include "UserManager.h"
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

UserManager::UserManager(QWidget *parent)
    : QWidget{parent}
{

    m_manager = Common::getNetManager();
    //初始化商品信息表页面
    initTableWidget();
}

UserManager::~UserManager()
{
    if (m_manager) delete m_manager;
}

void UserManager::initTableWidget()
{

    Search_Btn = new QPushButton(tr("搜索"), this);
    Search_LineEdit = new QLineEdit(this);
    Add_Btn = new QPushButton(tr("添加"), this);
    Delete_Btn = new QPushButton(tr("删除"), this);
    Update_Btn = new QPushButton(tr("更新"), this);


    connect(Search_Btn, &QPushButton::clicked, this, &UserManager::search);
    connect(Add_Btn, &QPushButton::clicked, this, &UserManager::add);
    connect(Delete_Btn, &QPushButton::clicked, this, &UserManager::remove);
    connect(Update_Btn, &QPushButton::clicked, this, &UserManager::update);

    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(4);
    QStringList headerLabels;
    m_tableWidget->setHorizontalHeaderLabels(QStringList()<<u8"用户ID" <<u8"用户名称"<<u8"用户昵称"<<u8"用户密码"<<u8"联系方式"<<u8"创建时间"<<u8"电子邮箱"<<u8"用户权限");

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

void UserManager::initEditWidget()
{
    UserManager_Edit = new QWidget();

    QLabel *UserId_Label = new QLabel(tr("用户ID"));
    QLabel *UserName_Label = new QLabel(tr("用户名称"));
    QLabel *UserNickName_Label = new QLabel(tr("用户昵称"));
    QLabel *Password_Label = new QLabel(tr("用户密码"));
    QLabel *Phone_Label = new QLabel(tr("联系方式"));
    QLabel *CreateTime_Label = new QLabel(tr("创建时间"));
    QLabel *Email_Label = new QLabel(tr("电子邮箱"));
    QLabel *Power_Label = new QLabel(tr("用户权限"));
    UserId_Edit = new QLineEdit();
    UserName_Edit = new QLineEdit();
    UserNickName_Edit = new QLineEdit();
    password_Edit = new QLineEdit();
    Phone_Edit = new QLineEdit();
    CreateTime_Edit = new QLineEdit();
    Email_Edit = new QLineEdit();
    Power_Edit = new QLineEdit();
    update_Save_Btn = new QPushButton(tr("保存"));
    Edit_Cancel_Btn = new QPushButton(tr("取消"));
    connect(update_Save_Btn, &QPushButton::clicked, this, &UserManager::update_save_info);
    connect(Edit_Cancel_Btn, &QPushButton::clicked, this, &UserManager::cancel);

    QGridLayout *EditLayout = new QGridLayout();
    EditLayout->addWidget(UserId_Label,0,0);
    EditLayout->addWidget(UserName_Label,1,0);
    EditLayout->addWidget(UserNickName_Label,2,0);
    EditLayout->addWidget(Password_Label,3,0);
    EditLayout->addWidget(Phone_Label,4,0);
    EditLayout->addWidget(CreateTime_Label ,5,0);
    EditLayout->addWidget(Email_Label,6,0);
    EditLayout->addWidget(Power_Label,7,0);
    EditLayout->addWidget(UserId_Edit,0,1);
    UserId_Edit->setFocusPolicy(Qt::NoFocus);
    EditLayout->addWidget(UserName_Edit,1,1);
    EditLayout->addWidget(UserNickName_Edit,2,1);
    EditLayout->addWidget(password_Edit,3,1);
    EditLayout->addWidget(Phone_Edit,4,1);
    EditLayout->addWidget(CreateTime_Edit,5,1);
    EditLayout->addWidget(Email_Edit,6,1);
    EditLayout->addWidget(Power_Edit,7,1);
    EditLayout->addWidget(update_Save_Btn,8,0);
    EditLayout->addWidget(Edit_Cancel_Btn,8,1);
    UserManager_Edit->setLayout(EditLayout);
}


QStringList UserManager::getCountStatus(QByteArray json)
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

void UserManager::refreshTable()
{
    clearUserManagerList();
    clearUserManagerItems();
    m_tableWidget->setRowCount(0);
    m_UserManagerCount = 0;

    QNetworkRequest request;

    LoginInfoInstance *login = LoginInfoInstance::getInstance();
    QString url = QString("http://%1:%2/UserManager?cmd=UserManagerCount").arg(login->getIp()).arg(login->getPort());
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
        m_UserManagerCount = list.at(1).toLong();
        clearUserManagerList();
        if(m_UserManagerCount > 0){
            m_start = 0;
            m_count = 10;
            getUserManagerList();
        }else{
            refreshUserManagerItems();
        }
    });
}

void UserManager::clearUserManagerList()
{
    m_tableWidget->clear();
}

void UserManager::clearUserManagerItems()
{
    m_UserManagerList.clear();
}

void UserManager::refreshUserManagerItems()
{
    if(m_UserManagerList.isEmpty() == false){
        int n = m_UserManagerList.size();
        for(int i = 0;i < n;++i){
            UserManagerInfo *tmp = m_UserManagerList.at(i);
            int row = m_tableWidget->rowCount();
            m_tableWidget->insertRow(row);
            m_tableWidget->setItem(row,0,new QTableWidgetItem(QString::number(tmp->UserId)));
            m_tableWidget->setItem(row,1,new QTableWidgetItem(tmp->UserName));
            m_tableWidget->setItem(row,2,new QTableWidgetItem(tmp->UserNickName));
            m_tableWidget->setItem(row,3,new QTableWidgetItem(tmp->password));
            m_tableWidget->setItem(row,4,new QTableWidgetItem(tmp->Phone));
            m_tableWidget->setItem(row,5,new QTableWidgetItem(tmp->CreateTime));
            m_tableWidget->setItem(row,6,new QTableWidgetItem(tmp->Email));
            m_tableWidget->setItem(row,7,new QTableWidgetItem(QString::number(tmp->Power)));
        }
    }
}

void UserManager::getUserManagerList()
{
    if(m_UserManagerCount <= 0){
        refreshUserManagerItems();
        return;
    }else if(m_count > m_UserManagerCount)
    {
        m_count = m_UserManagerCount;
    }

    QNetworkRequest request;

    LoginInfoInstance *login = LoginInfoInstance::getInstance();

    QString url = QString("http://%1:%2/UserManager?cmd=UserManagerNormal").arg(login->getIp()).arg(login->getPort());

    request.setUrl(QUrl(url));

    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = setUserManagerListJson(login->getUser(),login->getToken(),m_start,m_count);

    m_start += m_count;
    m_UserManagerCount -= m_count;

    QNetworkReply * reply = m_manager->post(request,data);
    if(reply == NULL){
        cout<<"getUserManagerList reply == NULL";
        return;
    }
    connect(reply,&QNetworkReply::finished,[=](){
       if(reply->error() != QNetworkReply::NoError){
           cout<<"getUserManagerList error: "<<reply->errorString();
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
           getUserManagerJsonInfo(array);
           getUserManagerList();
       }
    });
}

void UserManager::getSearchList()
{
    if(m_SearchCount <= 0)
    {
        refreshUserManagerItems();
        return;
    }
    else if(s_count > m_SearchCount)
    {
        s_count = m_SearchCount;
    }

    QNetworkRequest request;
    LoginInfoInstance *login = LoginInfoInstance::getInstance();
    QString url;

    url = QString("http://%1:%2/UserManager?cmd=UserManagerResult").arg(login->getIp()).arg(login->getPort());
    request.setUrl(QUrl( url ));

    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = setUserManagerListJson( login->getUser(), login->getToken(), s_start, s_count);

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
            getUserManagerJsonInfo(array);
            getSearchList();
        }
    });
}

void UserManager::getUserManagerJsonInfo(QByteArray data)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data,&error);
    if(error.error == QJsonParseError::NoError){
        if(doc.isNull() || doc.isEmpty()){
            cout<<" m_UserManagerList doc.isNUll() || doc.isEmpty() ";
            return;
        }
        if(doc.isObject()){

            QJsonObject obj = doc.object();

            QJsonArray array = obj.value("UserManager").toArray();

            int size = array.size();

            for(int i = 0;i < size;++i){
                QJsonObject tmp = array[i].toObject();
                UserManagerInfo *info = new UserManagerInfo;
                info->UserId = tmp.value("UserId").toInt();
                info->UserName = tmp.value("UserName").toString();
                info->UserNickName = tmp.value("UserNickName").toString();
                info->password = tmp.value("password").toString();
                info->Phone = tmp.value("Phone").toString();
                info->CreateTime = tmp.value("CreateTime").toString();
                info->Email = tmp.value("Email").toString();
                info->Power = tmp.value("Power").toInt();
                m_UserManagerList.append(info);

            }
        }
    }else{
        cout<<"getUserManagerJsonInfo error = "<<error.errorString();
    }
}

QByteArray UserManager::setGetCountJson(QString user, QString token)
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


QByteArray UserManager::setUserManagerListJson(QString user, QString token, int start, int count)
{
    QMap<QString,QVariant> tmp;
    tmp.insert("user",user);
    tmp.insert("token",token);
    tmp.insert("start",start);
    tmp.insert("count",count);

    QJsonDocument jsonDocument = QJsonDocument::fromVariant(tmp);
    if( jsonDocument.isNull()){
        cout<<"setUserManagerListJson jsonDocument.isNull()";
        return "";
    }

    return jsonDocument.toJson();
}

QByteArray UserManager::setUploadJson()
{
    QMap<QString,QVariant> tmp;
    tmp.insert("UserId",UserId_Edit->text().toInt());
    tmp.insert("UserName",UserName_Edit->text());
    tmp.insert("UserNickName",UserNickName_Edit->text());
    tmp.insert("password",password_Edit->text());
    tmp.insert("Phone",Phone_Edit->text());
    tmp.insert("CreateTime",CreateTime_Edit->text());
    tmp.insert("Power",Email_Edit->text());
    tmp.insert("UserManager_time",Power_Edit->text().toInt());

    QJsonDocument jsonDocument = QJsonDocument::fromVariant(tmp);
    if(jsonDocument.isNull()){
        cout<<"setUploadJson jsonDocument.isNull()";
        return "";
    }
    return jsonDocument.toJson();
}

void UserManager::search()
{
    clearUserManagerList();
    clearUserManagerItems();
    m_tableWidget->setRowCount(0);
    m_SearchCount = 0;

    QNetworkRequest request;
    LoginInfoInstance *login = LoginInfoInstance::getInstance();

    QString url = QString("http://%1:%2/UserManager?cmd=UserManagerSearch=%3").arg(login->getIp()).arg(login->getPort()).arg(QString::fromUtf8(Search_LineEdit->text().toUtf8().toBase64()));
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
            refreshUserManagerItems(); //更新表
        }
    });
}

void UserManager::add()
{
    cur_status = add_status;
    initEditWidget();
    UserManager_Edit->show();
}

QByteArray UserManager::setSelectJson(){
    QMap<QString,QVariant> tmp;
    QModelIndexList selectedRows = m_tableWidget->selectionModel()->selectedRows();
    foreach (QModelIndex index, selectedRows) {
        int row = index.row();
        tmp.insert("UserId",m_tableWidget->item(row, 0)->text());
        tmp.insert("UserName",m_tableWidget->item(row, 1)->text());
        tmp.insert("UserNickName",m_tableWidget->item(row, 2)->text());
        tmp.insert("password",m_tableWidget->item(row, 3)->text());
        tmp.insert("Phone",m_tableWidget->item(row, 4)->text());
        tmp.insert("CreateTime",m_tableWidget->item(row, 5)->text());
        tmp.insert("Power",m_tableWidget->item(row, 6)->text());
        tmp.insert("UserManager_time",m_tableWidget->item(row, 7)->text());
    }
    QJsonDocument jsonDocument = QJsonDocument::fromVariant(tmp);
    if(jsonDocument.isNull()){
        cout<<"setSelectJson jsonDocument.isNull()";
        return "";
    }
    return jsonDocument.toJson();
}

void UserManager::remove()
{
    QByteArray array = setSelectJson();

    QNetworkRequest request;
    LoginInfoInstance *login = LoginInfoInstance::getInstance();
    QString url= QString("http://%1:%2/UserManager?cmd=UserManagerDelete").arg(login->getIp()).arg(login->getPort());

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

void UserManager::update()
{
    cur_status = update_status;
    initEditWidget();
    QModelIndexList selectedRows = m_tableWidget->selectionModel()->selectedRows();
    foreach (QModelIndex index, selectedRows) {
        int row = index.row();
        UserId_Edit->setText(m_tableWidget->item(row, 0)->text());
        UserName_Edit->setText(m_tableWidget->item(row, 1)->text());
        UserNickName_Edit->setText(m_tableWidget->item(row, 2)->text());
        password_Edit->setText(m_tableWidget->item(row, 3)->text());
        Phone_Edit->setText(m_tableWidget->item(row, 4)->text());
        CreateTime_Edit->setText(m_tableWidget->item(row, 5)->text());
        Email_Edit->setText(m_tableWidget->item(row, 6)->text());
        Power_Edit->setText(m_tableWidget->item(row, 7)->text());
    }
    UserManager_Edit->show();
}

void UserManager::update_save_info()
{
    QByteArray array = setUploadJson();

    QNetworkRequest request;
    LoginInfoInstance *login = LoginInfoInstance::getInstance();
    QString url;
    if(update_status == cur_status){
        url = QString("http://%1:%2/UserManager?cmd=UserManagerUpdate").arg(login->getIp()).arg(login->getPort());
    }else if(add_status == cur_status){
        url = QString("http://%1:%2/UserManager?cmd=UserManagerAdd").arg(login->getIp()).arg(login->getPort());
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
            UserId_Edit->clear();
            UserName_Edit->clear();
            UserNickName_Edit->clear();
            password_Edit->clear();
            Phone_Edit->clear();
            CreateTime_Edit->clear();
            Email_Edit->clear();
            Power_Edit->clear();
            UserManager_Edit->hide();
            refreshTable();
        }else{
            QMessageBox::warning(this,"上传失败","上传失败");
            UserManager_Edit->hide();
        }
        delete reply;
    });
}

void UserManager::cancel()
{
    UserId_Edit->clear();
    UserName_Edit->clear();
    UserNickName_Edit->clear();
    password_Edit->clear();
    Phone_Edit->clear();
    CreateTime_Edit->clear();
    Email_Edit->clear();
    Power_Edit->clear();
    UserManager_Edit->hide();
}

