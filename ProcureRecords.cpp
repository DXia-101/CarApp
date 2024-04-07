#include "ProcureRecords.h"
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

ProcureRecords::ProcureRecords(QWidget *parent)
    : QWidget{parent}
{

    m_manager = Common::getNetManager();
    //初始化商品信息表页面
    initTableWidget();
}

ProcureRecords::~ProcureRecords()
{
    if (m_manager) delete m_manager;
}

void ProcureRecords::initTableWidget()
{

    Search_Btn = new QPushButton(tr("搜索"), this);
    Search_LineEdit = new QLineEdit(this);
    Add_Btn = new QPushButton(tr("添加"), this);
    Delete_Btn = new QPushButton(tr("删除"), this);
    Update_Btn = new QPushButton(tr("更新"), this);


    connect(Search_Btn, &QPushButton::clicked, this, &ProcureRecords::search);
    connect(Add_Btn, &QPushButton::clicked, this, &ProcureRecords::add);
    connect(Delete_Btn, &QPushButton::clicked, this, &ProcureRecords::remove);
    connect(Update_Btn, &QPushButton::clicked, this, &ProcureRecords::update);

    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(4);
    QStringList headerLabels;
    m_tableWidget->setHorizontalHeaderLabels(QStringList()<<u8"采购编号" <<u8"原料名称"<<u8"采购数量"<<u8"采购时间");

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

void ProcureRecords::initEditWidget()
{
    Procure_Edit = new QWidget();

    QLabel *ProcureId_Label = new QLabel(tr("采购ID"));
    QLabel *MaterialName_Label = new QLabel(tr("原料名称"));
    QLabel *MaterialQuantity_Label = new QLabel(tr("采购数量"));
    QLabel *ProcureTime_Label = new QLabel(tr("采购时间"));
    ProcureId_Edit  = new QLineEdit();
    MaterialName_Edit = new QLineEdit();
    MaterialQuantity_Edit = new QLineEdit();
    ProcureTime_Edit = new QLineEdit();
    update_Save_Btn = new QPushButton(tr("保存"));
    Edit_Cancel_Btn = new QPushButton(tr("取消"));
    connect(update_Save_Btn, &QPushButton::clicked, this, &ProcureRecords::update_save_info);
    connect(Edit_Cancel_Btn, &QPushButton::clicked, this, &ProcureRecords::cancel);

    QGridLayout *EditLayout = new QGridLayout();
    EditLayout->addWidget(ProcureId_Label,0,0);
    EditLayout->addWidget(MaterialName_Label,1,0);
    EditLayout->addWidget(MaterialQuantity_Label,2,0);
    EditLayout->addWidget(ProcureTime_Label,3,0);
    EditLayout->addWidget(ProcureId_Edit,0,1);
    EditLayout->addWidget(MaterialName_Edit,1,1);
    EditLayout->addWidget(MaterialQuantity_Edit,2,1);
    EditLayout->addWidget(ProcureTime_Edit,3,1);
    EditLayout->addWidget(update_Save_Btn,4,0);
    EditLayout->addWidget(Edit_Cancel_Btn,4,1);
    Procure_Edit->setLayout(EditLayout);
}


QStringList ProcureRecords::getCountStatus(QByteArray json)
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

void ProcureRecords::refreshTable()
{
    clearProcureList();
    clearProcureItems();
    m_tableWidget->setRowCount(0);
    m_ProcureCount = 0;

    QNetworkRequest request;

    LoginInfoInstance *login = LoginInfoInstance::getInstance();
    QString url = QString("http://%1:%2/Procure?cmd=ProcureCount").arg(login->getIp()).arg(login->getPort());
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
        m_ProcureCount = list.at(1).toLong();
        clearProcureList();
        if(m_ProcureCount > 0){
            m_start = 0;
            m_count = 10;
            getProcureList();
        }else{
            refreshProcureItems();
        }
    });
}

void ProcureRecords::clearProcureList()
{
    m_tableWidget->clearContents();
}

void ProcureRecords::clearProcureItems()
{
    m_ProcureList.clear();
}

void ProcureRecords::refreshProcureItems()
{
    if(m_ProcureList.isEmpty() == false){
        int n = m_ProcureList.size();
        for(int i = 0;i < n;++i){
            ProcureInfo *tmp = m_ProcureList.at(i);
            int row = m_tableWidget->rowCount();
            m_tableWidget->insertRow(row);
            m_tableWidget->setItem(row,0,new QTableWidgetItem(QString::number(tmp->procure_id)));
            m_tableWidget->setItem(row,1,new QTableWidgetItem(tmp->material_name));
            m_tableWidget->setItem(row,2,new QTableWidgetItem(QString::number(tmp->material_quantity)));
            m_tableWidget->setItem(row,3,new QTableWidgetItem(tmp->procure_time));
        }
    }
}

void ProcureRecords::getProcureList()
{
    if(m_ProcureCount <= 0){
        refreshProcureItems();
        return;
    }else if(m_count > m_ProcureCount)
    {
        m_count = m_ProcureCount;
    }

    QNetworkRequest request;

    LoginInfoInstance *login = LoginInfoInstance::getInstance();    

    QString url = QString("http://%1:%2/Procure?cmd=ProcureNormal").arg(login->getIp()).arg(login->getPort());

    request.setUrl(QUrl(url));

    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = setProcureListJson(login->getUser(),login->getToken(),m_start,m_count);

    m_start += m_count;
    m_ProcureCount -= m_count;

    QNetworkReply * reply = m_manager->post(request,data);
    if(reply == NULL){
        cout<<"getProcureRecordsList reply == NULL";
        return;
    }
    connect(reply,&QNetworkReply::finished,[=](){
       if(reply->error() != QNetworkReply::NoError){
           cout<<"getProcureRecordsList error: "<<reply->errorString();
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
           getProcureJsonInfo(array);
           getProcureList();
       }
    });
}

void ProcureRecords::getSearchList()
{
    if(m_SearchCount <= 0) 
    {
        refreshProcureItems();
        return;
    }
    else if(s_count > m_SearchCount) 
    {
        s_count = m_SearchCount;
    }

    QNetworkRequest request; 
    LoginInfoInstance *login = LoginInfoInstance::getInstance();
    QString url;

    url = QString("http://%1:%2/Procure?cmd=ProcureResult").arg(login->getIp()).arg(login->getPort());
    request.setUrl(QUrl( url )); 

    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = setProcureListJson( login->getUser(), login->getToken(), s_start, s_count);

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
            getProcureJsonInfo(array);
            getSearchList();
        }
    });
}

void ProcureRecords::getProcureJsonInfo(QByteArray data)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data,&error);
    if(error.error == QJsonParseError::NoError){
        if(doc.isNull() || doc.isEmpty()){
            cout<<" m_ProcureRecordsList doc.isNUll() || doc.isEmpty() ";
            return;
        }
        if(doc.isObject()){
            
            QJsonObject obj = doc.object();
            
            QJsonArray array = obj.value("Procure").toArray();

            int size = array.size(); 

            for(int i = 0;i < size;++i){
                QJsonObject tmp = array[i].toObject();  
                ProcureInfo *info = new ProcureInfo;
                info->procure_id = tmp.value("procure_id").toInt();
                info->material_name = tmp.value("material_name").toString();
                info->material_quantity = tmp.value("material_quantity").toInt();
                info->procure_time = tmp.value("procure_time").toString();
                m_ProcureList.append(info);

            }
        }
    }else{
        cout<<"getProcureRecordsJsonInfo error = "<<error.errorString();
    }
}

QByteArray ProcureRecords::setGetCountJson(QString user, QString token)
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


QByteArray ProcureRecords::setProcureListJson(QString user, QString token, int start, int count)
{
    QMap<QString,QVariant> tmp;
    tmp.insert("user",user);
    tmp.insert("token",token);
    tmp.insert("start",start);
    tmp.insert("count",count);

    QJsonDocument jsonDocument = QJsonDocument::fromVariant(tmp);
    if( jsonDocument.isNull()){
        cout<<"setProcureRecordsListJson jsonDocument.isNull()";
        return "";
    }

    return jsonDocument.toJson();
}

QByteArray ProcureRecords::setUploadJson()
{
    QMap<QString,QVariant> tmp;
    tmp.insert("procure_id",ProcureId_Edit->text().toInt());
    tmp.insert("material_name",MaterialName_Edit->text());
    tmp.insert("material_quantity",MaterialQuantity_Edit->text());
    tmp.insert("procure_time",ProcureTime_Edit->text().toInt());

    QJsonDocument jsonDocument = QJsonDocument::fromVariant(tmp);
    if(jsonDocument.isNull()){
        cout<<"setUploadJson jsonDocument.isNull()";
        return "";
    }
    return jsonDocument.toJson();
}

void ProcureRecords::search()
{
    if(Search_LineEdit->text().isEmpty()){
        refreshTable();
        return;
    }
    clearProcureList();
    clearProcureItems();
    m_tableWidget->setRowCount(0);
    m_SearchCount = 0;

    QNetworkRequest request;
    LoginInfoInstance *login = LoginInfoInstance::getInstance();

    QString url = QString("http://%1:%2/Procure?cmd=ProcureSearch=%3").arg(login->getIp()).arg(login->getPort()).arg(QString::fromUtf8(Search_LineEdit->text().toUtf8().toBase64()));
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
            refreshProcureItems(); //更新表
        }
    });
}

void ProcureRecords::add()
{
    cur_status = add_status;
    initEditWidget();
    Procure_Edit->show();
}

QByteArray ProcureRecords::setSelectJson(){
    QMap<QString,QVariant> tmp;
    QModelIndexList selectedRows = m_tableWidget->selectionModel()->selectedRows();
    foreach (QModelIndex index, selectedRows) {
        int row = index.row();
        tmp.insert("procure_id",m_tableWidget->item(row, 0)->text().toInt());
        tmp.insert("material_name",m_tableWidget->item(row, 1)->text());
        tmp.insert("material_quantity",m_tableWidget->item(row, 2)->text());
        tmp.insert("procure_time",m_tableWidget->item(row, 3)->text());
    }
    QJsonDocument jsonDocument = QJsonDocument::fromVariant(tmp);
    if(jsonDocument.isNull()){
        cout<<"setSelectJson jsonDocument.isNull()";
        return "";
    }
    return jsonDocument.toJson();
}

void ProcureRecords::remove()
{
    QByteArray array = setSelectJson();

    QNetworkRequest request;
    LoginInfoInstance *login = LoginInfoInstance::getInstance();
    QString url= QString("http://%1:%2/Procure?cmd=ProcureDelete").arg(login->getIp()).arg(login->getPort());

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

void ProcureRecords::update()
{
    cur_status = update_status;
    initEditWidget();
    QModelIndexList selectedRows = m_tableWidget->selectionModel()->selectedRows();
    foreach (QModelIndex index, selectedRows) {
        int row = index.row();
        ProcureId_Edit->setText(m_tableWidget->item(row, 0)->text());
        MaterialName_Edit->setText(m_tableWidget->item(row, 1)->text());
        MaterialQuantity_Edit->setText(m_tableWidget->item(row, 2)->text());
        ProcureTime_Edit->setText(m_tableWidget->item(row, 3)->text());
    }
    Procure_Edit->show();
}

void ProcureRecords::update_save_info()
{
    QByteArray array = setUploadJson();

    QNetworkRequest request;
    LoginInfoInstance *login = LoginInfoInstance::getInstance();
    QString url;
    if(update_status == cur_status){
        url = QString("http://%1:%2/Procure?cmd=ProcureUpdate").arg(login->getIp()).arg(login->getPort());
    }else if(add_status == cur_status){
        url = QString("http://%1:%2/Procure?cmd=ProcureAdd").arg(login->getIp()).arg(login->getPort());
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
            ProcureId_Edit->clear();
            MaterialName_Edit->clear();
            MaterialQuantity_Edit->clear();
            ProcureTime_Edit->clear();
            Procure_Edit->hide();
            refreshTable();
        }else{
            QMessageBox::warning(this,"上传失败","上传失败");
            Procure_Edit->hide();
        }
        delete reply;
    });
}

void ProcureRecords::cancel()
{
    ProcureId_Edit->clear();
    MaterialName_Edit->clear();
    MaterialQuantity_Edit->clear();
    ProcureTime_Edit->clear();
    Procure_Edit->hide();
}

