#include "ProduceRecords.h"
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

ProduceRecords::ProduceRecords(QWidget *parent)
    : QWidget{parent}
{
    m_manager = Common::getNetManager();
    initTableWidget();
}

ProduceRecords::~ProduceRecords()
{
    if (m_manager) delete m_manager;
}

void ProduceRecords::initTableWidget()
{

    Search_Btn = new QPushButton(tr("搜索"), this);
    Search_LineEdit = new QLineEdit(this);
    Add_Btn = new QPushButton(tr("添加"), this);
    Delete_Btn = new QPushButton(tr("删除"), this);
    Update_Btn = new QPushButton(tr("更新"), this);

    connect(Search_Btn, &QPushButton::clicked, this, &ProduceRecords::search);
    connect(Add_Btn, &QPushButton::clicked, this, &ProduceRecords::add);
    connect(Delete_Btn, &QPushButton::clicked, this, &ProduceRecords::remove);
    connect(Update_Btn, &QPushButton::clicked, this, &ProduceRecords::update);

    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(5);
    QStringList headerLabels;
    m_tableWidget->setHorizontalHeaderLabels(QStringList() <<u8"生产编号" <<u8"产品名称"<<u8"生产数量"<<u8"生产时间"<<u8"消耗原材料");

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

void ProduceRecords::initEditWidget()
{
    Produce_Edit = new QWidget();
    QLabel* id_Label = new QLabel(tr("生产ID"));
    QLabel* name_Label = new QLabel(tr("产品名称"));
    QLabel* number_Label = new QLabel(tr("生产数量"));
    QLabel* date_Label = new QLabel(tr("生产时间"));
    QLabel* material_Label = new QLabel(tr("消耗原料"));
    id_Edit = new QLineEdit();
    name_Edit = new QLineEdit();
    number_Edit = new QLineEdit();
    date_Edit = new QLineEdit();
    material_table = new QTableWidget();
    update_Save_Btn = new QPushButton(tr("保存"));
    Edit_Cancel_Btn = new QPushButton(tr("取消"));
    add_Material_Btn = new QPushButton(tr("添加原料"));
    delete_Material_Btn = new QPushButton(tr("删除原料"));

    connect(update_Save_Btn, &QPushButton::clicked, this, &ProduceRecords::update_save_info);
    connect(Edit_Cancel_Btn, &QPushButton::clicked, this, &ProduceRecords::cancel);
    connect(add_Material_Btn, &QPushButton::clicked, this, &ProduceRecords::add_material);
    connect(delete_Material_Btn, &QPushButton::clicked, this, &ProduceRecords::delete_material);

    material_table->setColumnCount(2);
    material_table->setHorizontalHeaderLabels(QStringList() << "原料名" << "数量");

    QGridLayout* EditLayout = new QGridLayout();
    EditLayout->addWidget(id_Label, 0, 0);
    EditLayout->addWidget(name_Label, 1, 0);
    EditLayout->addWidget(number_Label, 2, 0);
    EditLayout->addWidget(date_Label, 3, 0);
    EditLayout->addWidget(material_Label, 4, 0);
    EditLayout->addWidget(id_Edit, 0, 1);
    EditLayout->addWidget(name_Edit, 1, 1);
    EditLayout->addWidget(number_Edit, 2, 1);
    EditLayout->addWidget(date_Edit, 3, 1);
    EditLayout->addWidget(material_table, 4, 1);
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    EditLayout->addLayout(buttonLayout,5,1);
    buttonLayout->addWidget(add_Material_Btn);
    buttonLayout->addWidget(delete_Material_Btn);
    EditLayout->addWidget(update_Save_Btn, 6, 0);
    EditLayout->addWidget(Edit_Cancel_Btn, 6, 1);

    Produce_Edit->setLayout(EditLayout);
}


QStringList ProduceRecords::getCountStatus(QByteArray json)
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

void ProduceRecords::refreshTable()
{
    clearProduceList();
    clearProduceItems();

    m_tableWidget->setRowCount(0);

    m_ProduceCount = 0;

    QNetworkRequest request;
    LoginInfoInstance *login = LoginInfoInstance::getInstance();

    QString url = QString("http://%1:%2/Produce?cmd=ProduceCount").arg(login->getIp()).arg(login->getPort());
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

        m_ProduceCount = list.at(1).toLong();
        qDebug()<<"m_ProduceCount : "<<m_ProduceCount;

        clearProduceList();

        if(m_ProduceCount > 0){

            m_start = 0;
            m_count = 10;
            getProduceList();
        }else{
            refreshProduceItems();
        }
    });
}

void ProduceRecords::clearProduceList()
{
    m_tableWidget->clearContents();
}

void ProduceRecords::clearProduceItems()
{
    m_ProduceList.clear();
}

void ProduceRecords::refreshProduceItems()
{
    if(m_ProduceList.isEmpty() == false){
        int n = m_ProduceList.size();
        for(int i = 0;i < n;++i){
            ProduceInfo *tmp = m_ProduceList.at(i);
            int row = m_tableWidget->rowCount();
            m_tableWidget->insertRow(row);
            m_tableWidget->setItem(row,0,new QTableWidgetItem(QString::number(tmp->produce_id)));
            m_tableWidget->setItem(row,1,new QTableWidgetItem(tmp->product_name));
            m_tableWidget->setItem(row,2,new QTableWidgetItem(QString::number(tmp->produce_id)));
            m_tableWidget->setItem(row,3,new QTableWidgetItem(tmp->product_time));
            QJsonDocument jsonDoc(tmp->RawMaterial);
            m_tableWidget->setItem(row,4,new QTableWidgetItem(jsonDoc.toJson(QJsonDocument::Compact)));
        }
    }
}

void ProduceRecords::getProduceList()
{
    if(m_ProduceCount <= 0){
        refreshProduceItems();
        return;
    }else if(m_count > m_ProduceCount)
    {
        m_count = m_ProduceCount;
    }

    QNetworkRequest request;

    LoginInfoInstance *login = LoginInfoInstance::getInstance();

    QString url = QString("http://%1:%2/Produce?cmd=ProduceNormal").arg(login->getIp()).arg(login->getPort());

    request.setUrl(QUrl(url));

    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = setProduceListJson(login->getUser(),login->getToken(),m_start,m_count);

    m_start += m_count;
    m_ProduceCount -= m_count;

    QNetworkReply * reply = m_manager->post(request,data);
    if(reply == NULL){
        cout<<"getProduceRecordsList reply == NULL";
        return;
    }

    connect(reply,&QNetworkReply::finished,[=](){
       if(reply->error() != QNetworkReply::NoError){
           cout<<"getProduceRecordsList error: "<<reply->errorString();
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
           getProduceJsonInfo(array);
           getProduceList();
       }
    });
}

void ProduceRecords::getSearchList()
{

    if(m_SearchCount <= 0)
    {
        refreshProduceItems();
        return;
    }
    else if(s_count > m_SearchCount)
    {
        s_count = m_SearchCount;
    }


    QNetworkRequest request;


    LoginInfoInstance *login = LoginInfoInstance::getInstance();

    QString url;

    url = QString("http://%1:%2/Produce?cmd=ProduceResult").arg(login->getIp()).arg(login->getPort());
    request.setUrl(QUrl( url ));

    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = setProduceListJson( login->getUser(), login->getToken(), s_start, s_count);

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
            getProduceJsonInfo(array);
            getSearchList();
        }
    });
}

void ProduceRecords::getProduceJsonInfo(QByteArray data)
{
    QJsonParseError error;

    QJsonDocument doc = QJsonDocument::fromJson(data,&error);
    if(error.error == QJsonParseError::NoError){
        if(doc.isNull() || doc.isEmpty()){
            cout<<" m_ProduceList doc.isNUll() || doc.isEmpty() ";
            return;
        }
        if(doc.isObject()){

            QJsonObject obj = doc.object();
            QJsonArray array = obj.value("Produce").toArray();
            int size = array.size();

            for(int i = 0;i < size;++i){
                QJsonObject tmp = array[i].toObject();
                ProduceInfo *info = new ProduceInfo;
                info->produce_id = tmp.value("produce_id").toInt();
                info->product_name = tmp.value("product_name").toString();
                info->product_quantity = tmp.value("product_quantity").toInt();
                info->product_time = tmp.value("product_time").toString();
                info->RawMaterial = tmp.value("RawMaterial").toArray();

                m_ProduceList.append(info);
            }
        }
    }else{
        cout<<"getProduceRecordsJsonInfo error = "<<error.errorString();
    }
}

QByteArray ProduceRecords::setGetCountJson(QString user, QString token)
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

QByteArray ProduceRecords::setProduceListJson(QString user, QString token, int start, int count)
{
    QMap<QString,QVariant> tmp;
    tmp.insert("user",user);
    tmp.insert("token",token);
    tmp.insert("start",start);
    tmp.insert("count",count);

    QJsonDocument jsonDocument = QJsonDocument::fromVariant(tmp);
    if( jsonDocument.isNull()){
        cout<<"setProduceRecordsListJson jsonDocument.isNull()";
        return "";
    }

    return jsonDocument.toJson();
}

QByteArray ProduceRecords::setUploadJson()
{
    QMap<QString, QVariant> tmp;
    tmp.insert("produce_id", id_Edit->text().toInt());
    tmp.insert("product_name", name_Edit->text());
    tmp.insert("product_quantity", number_Edit->text().toInt());
    tmp.insert("product_time", date_Edit->text());

    QJsonArray jsonArray;

    int rowCount = m_tableWidget->rowCount();
    for (int row = 0; row < rowCount; ++row) {
        QString materialName = m_tableWidget->item(row, 0)->text();
        int quantity = m_tableWidget->item(row, 1)->text().toInt();

        QJsonObject jsonObject;
        jsonObject["material_name"] = materialName;
        jsonObject["quantity"] = quantity;

        jsonArray.append(jsonObject);
    }

    QJsonObject jsonDocumentObject;
    jsonDocumentObject["data"] = QJsonObject::fromVariantMap(tmp);
    jsonDocumentObject["material_table"] = jsonArray;

    QJsonDocument jsonDocument(jsonDocumentObject);

    if (jsonDocument.isNull()) {
        cout << "setUploadJson jsonDocument.isNull()";
        return "";
    }

    return jsonDocument.toJson();
}

void ProduceRecords::search()
{
    if(Search_LineEdit->text().isEmpty()){
        refreshTable();
        return;
    }
    clearProduceList();
    clearProduceItems();

    m_tableWidget->setRowCount(0);

    m_SearchCount = 0;

    QNetworkRequest request;
    LoginInfoInstance *login = LoginInfoInstance::getInstance();
    QString url = QString("http://%1:%2/Produce?cmd=ProduceSearch=%3").arg(login->getIp()).arg(login->getPort()).arg(QString::fromUtf8(Search_LineEdit->text().toUtf8().toBase64()));
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
            refreshProduceItems();
        }
    });
}

void ProduceRecords::add()
{
    cur_status = add_status;
    initEditWidget();
    Produce_Edit->show();
}

QByteArray ProduceRecords::setSelectJson(){
    QMap<QString,QVariant> tmp;
    QModelIndexList selectedRows = m_tableWidget->selectionModel()->selectedRows();
    foreach (QModelIndex index, selectedRows) {
        int row = index.row();
        tmp.insert("produce_id",m_tableWidget->item(row, 0)->text().toInt());
        tmp.insert("product_name",m_tableWidget->item(row, 1)->text());
        tmp.insert("product_quantity",m_tableWidget->item(row, 2)->text().toInt());
        tmp.insert("product_time",m_tableWidget->item(row, 3)->text());
        tmp.insert("RawMaterial",m_tableWidget->item(row, 4)->text());
    }
    QJsonDocument jsonDocument = QJsonDocument::fromVariant(tmp);
    if(jsonDocument.isNull()){
        cout<<"setSelectJson jsonDocument.isNull()";
        return "";
    }
    return jsonDocument.toJson();
}

void ProduceRecords::remove()
{
    QByteArray array = setSelectJson();

    QNetworkRequest request;
    LoginInfoInstance *login = LoginInfoInstance::getInstance();
    QString url= QString("http://%1:%2/Produce?cmd=ProduceDelete").arg(login->getIp()).arg(login->getPort());

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

void ProduceRecords::update()
{
    cur_status = update_status;
    initEditWidget();
    QModelIndexList selectedRows = m_tableWidget->selectionModel()->selectedRows();
    foreach (QModelIndex index, selectedRows) {
        int row = index.row();
        id_Edit->setText(m_tableWidget->item(row, 0)->text());
        name_Edit->setText(m_tableWidget->item(row, 1)->text());
        number_Edit->setText(m_tableWidget->item(row, 2)->text());
        date_Edit->setText(m_tableWidget->item(row, 3)->text());
        // 获取 JSON 数据并解析为 QJsonArray
        QTableWidgetItem* jsonItem = m_tableWidget->item(row, 4);
        QString jsonString = jsonItem->text();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8());
        QJsonArray jsonArray = jsonDoc.array();

        // 清空原来的 material_table
        material_table->clear();
        material_table->setRowCount(0);
        material_table->setColumnCount(2);
        material_table->setHorizontalHeaderLabels(QStringList() << "原料名" << "数量");

        int materialRowCount = jsonArray.size();
        material_table->setRowCount(materialRowCount);

        for (int i = 0; i < materialRowCount; ++i) {
            QJsonObject jsonObject = jsonArray[i].toObject();
            QString materialName = jsonObject.value("material_name").toString();
            int materialQuantity = jsonObject.value("quantity").toInt();

            QTableWidgetItem* nameItem = new QTableWidgetItem(materialName);
            QTableWidgetItem* quantityItem = new QTableWidgetItem(QString::number(materialQuantity));

            material_table->setItem(i, 0, nameItem);
            material_table->setItem(i, 1, quantityItem);
        }
    }

    Produce_Edit->show();
}

void ProduceRecords::update_save_info()
{
    QByteArray array = setUploadJson();

    QNetworkRequest request;
    LoginInfoInstance *login = LoginInfoInstance::getInstance();
    QString url;
    if(update_status == cur_status){
        url = QString("http://%1:%2/Produce?cmd=ProduceUpdate").arg(login->getIp()).arg(login->getPort());
    }else if(add_status == cur_status){
        url = QString("http://%1:%2/Produce?cmd=ProduceAdd").arg(login->getIp()).arg(login->getPort());
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
            id_Edit->clear();
            name_Edit->clear();
            number_Edit->clear();
            date_Edit->clear();
            material_table->clear();
            Produce_Edit->hide();
            refreshTable();
        }else{
            QMessageBox::warning(this,"上传失败","上传失败");
            Produce_Edit->hide();
        }
        delete reply;
    });
}

void ProduceRecords::cancel()
{
    id_Edit->clear();
    name_Edit->clear();
    number_Edit->clear();
    date_Edit->clear();
    material_table->clear();
    Produce_Edit->hide();
}

void ProduceRecords::SaveMaterialInfo()
{
    QJsonArray jsonArray;

    for (int row = 0; row < material_table->rowCount(); ++row) {
        QTableWidgetItem *nameItem = material_table->item(row, 0);
        QTableWidgetItem *quantityItem = material_table->item(row, 1);

        if (nameItem && quantityItem) {
            QString name = nameItem->text();
            int quantity = quantityItem->text().toInt();

            QJsonObject jsonObject;
            jsonObject["material_name"] = name;
            jsonObject["number"] = quantity;

            jsonArray.append(jsonObject);
        }
    }

    materialInfo = jsonArray;
}

void ProduceRecords::add_material()
{
// 创建一个新的 QWidget 用于输入原料名和数量
    QWidget* addMaterialWidget = new QWidget();
    QLabel* nameLabel = new QLabel(tr("原料名"));
    QLabel* quantityLabel = new QLabel(tr("数量"));
    QLineEdit* nameLineEdit = new QLineEdit();
    QLineEdit* quantityLineEdit = new QLineEdit();
    QPushButton* confirmButton = new QPushButton(tr("确认"));
    QPushButton* cancelButton = new QPushButton(tr("取消"));

    connect(confirmButton, &QPushButton::clicked, [=]() {
        QString materialName = nameLineEdit->text();
        QString quantityStr = quantityLineEdit->text();
        int quantity = quantityStr.toInt();

        // 在 material_table 中插入新的数据
        int rowCount = material_table->rowCount();
        material_table->insertRow(rowCount);
        QTableWidgetItem* nameItem = new QTableWidgetItem(materialName);
        QTableWidgetItem* quantityItem = new QTableWidgetItem(quantityStr);
        material_table->setItem(rowCount, 0, nameItem);
        material_table->setItem(rowCount, 1, quantityItem);

        addMaterialWidget->close();
    });

    connect(cancelButton, &QPushButton::clicked, [=]() {
        addMaterialWidget->close();
    });

    QGridLayout* layout = new QGridLayout();
    layout->addWidget(nameLabel, 0, 0);
    layout->addWidget(quantityLabel, 1, 0);
    layout->addWidget(nameLineEdit, 0, 1);
    layout->addWidget(quantityLineEdit, 1, 1);
    layout->addWidget(confirmButton, 2, 0);
    layout->addWidget(cancelButton, 2, 1);
    addMaterialWidget->setLayout(layout);

    addMaterialWidget->show();

}

void ProduceRecords::delete_material()
{
    QList<QTableWidgetItem*> selectedItems = material_table->selectedItems();
    foreach (QTableWidgetItem* item, selectedItems) {
        material_table->removeRow(item->row());
    }
}
