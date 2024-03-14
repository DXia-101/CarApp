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
    //http管理类对象
    m_manager = Common::getNetManager();
    //初始化生产记录信息表页面
    initTableWidget();
}

ProduceRecords::~ProduceRecords()
{
    if (m_manager) delete m_manager;
}

void ProduceRecords::initTableWidget()
{
    // 创建按钮和搜索框
    Search_Btn = new QPushButton(tr("搜索"), this);
    Search_LineEdit = new QLineEdit(this);
    Add_Btn = new QPushButton(tr("添加"), this);
    Delete_Btn = new QPushButton(tr("删除"), this);
    Update_Btn = new QPushButton(tr("更新"), this);

    // 连接按钮的点击信号到槽函数
    connect(Search_Btn, &QPushButton::clicked, this, &ProduceRecords::search);
    connect(Add_Btn, &QPushButton::clicked, this, &ProduceRecords::add);
    connect(Delete_Btn, &QPushButton::clicked, this, &ProduceRecords::remove);
    connect(Update_Btn, &QPushButton::clicked, this, &ProduceRecords::update);

    m_tableWidget = new QTableWidget(this);
    // 创建生产记录表格
    m_tableWidget->setColumnCount(5);
    QStringList headerLabels;
    m_tableWidget->setHorizontalHeaderLabels(QStringList() <<u8"生产编号" <<u8"产品名称"<<u8"生产数量"<<u8"生产时间"<<u8"消耗原材料");
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

    // 将水平布局和生产记录表添加到垂直布局中
    vLayout->addLayout(hLayout);
    vLayout->addWidget(m_tableWidget);

    //显示生产记录信息
    refreshTable();
}

void ProduceRecords::initEditWidget()
{
    ProduceRecords_Edit = new QWidget();

    QLabel *id_Label = new QLabel(tr("生产ID"));
    QLabel *name_Label = new QLabel(tr("产品名称"));
    QLabel *number_Label = new QLabel(tr("生产数量"));
    QLabel *date_Label = new QLabel(tr("生产时间"));
    QLabel *material_Label = new QLabel(tr("消耗原料"));
    id_Edit  = new QLineEdit();
    name_Edit = new QLineEdit();
    number_Edit = new QLineEdit();
    date_Edit = new QLineEdit();
    material_table = new QTableWidget();
    update_Save_Btn = new QPushButton(tr("保存"));
    Edit_Cancel_Btn = new QPushButton(tr("取消"));
    connect(update_Save_Btn, &QPushButton::clicked, this, &ProduceRecords::update_save_info);
    connect(Edit_Cancel_Btn, &QPushButton::clicked, this, &ProduceRecords::cancel);

    material_table->setColumnCount(2);
    material_table->setHorizontalHeaderLabels(QStringList() << "原料名" << "数量");

    QGridLayout *EditLayout = new QGridLayout();
    EditLayout->addWidget(id_Label,0,0);
    EditLayout->addWidget(name_Label,1,0);
    EditLayout->addWidget(number_Label,2,0);
    EditLayout->addWidget(date_Label,3,0);
    EditLayout->addWidget(material_Label,4,0);
    EditLayout->addWidget(id_Edit,0,1);
    EditLayout->addWidget(name_Edit,1,1);
    EditLayout->addWidget(number_Edit,2,1);
    EditLayout->addWidget(date_Edit,3,1);
    EditLayout->addWidget(material_table,4,1);
    EditLayout->addWidget(update_Save_Btn,5,0);
    EditLayout->addWidget(Edit_Cancel_Btn,5,1);
    ProduceRecords_Edit->setLayout(EditLayout);
}

// 得到服务器json文件
QStringList ProduceRecords::getCountStatus(QByteArray json)
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
void ProduceRecords::refreshTable()
{
    // 清空文件列表信息
    clearProduceRecordsList();

    //将之前的ProduceRecordslist清空
    clearProduceRecordsItems();

    //将表格行数清零
    m_tableWidget->setRowCount(0);

    //获取生产记录信息数目
    m_ProduceRecordsCount = 0;

    QNetworkRequest request;

    // 获取登陆信息实例
    // 获取单例
    LoginInfoInstance *login = LoginInfoInstance::getInstance();

    // 127.0.0.1:80/ProduceRecords?cmd=ProduceRecordscount
    // 获取生产记录信息数目
    QString url = QString("http://%1:%2/ProduceRecords?cmd=ProduceRecordsCount").arg(login->getIp()).arg(login->getPort());
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
        m_ProduceRecordsCount = list.at(1).toLong();

        // 清空文件列表信息
        clearProduceRecordsList();

        if(m_ProduceRecordsCount > 0){
            // 说明任然有生产记录
            m_start = 0;    //从0开始
            m_count = 10;   //每次请求10个

            // 获取新的生产记录列表信息
            getProduceRecordsList();
        }else{//没有生产记录
            refreshProduceRecordsItems(); //更新Items
        }
    });
}

// 清空生产记录列表
void ProduceRecords::clearProduceRecordsList()
{
    m_tableWidget->clear();
}

// 清空所有生产记录Item
void ProduceRecords::clearProduceRecordsItems()
{
    m_ProduceRecordsList.clear();
}

// 生产记录Item展示
void ProduceRecords::refreshProduceRecordsItems()
{
    //如果文件列表不为空，显示生产记录列表
    if(m_ProduceRecordsList.isEmpty() == false){
        int n = m_ProduceRecordsList.size();
        for(int i = 0;i < n;++i){
            ProduceInfo *tmp = m_ProduceRecordsList.at(i);
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

void ProduceRecords::getProduceRecordsList()
{
    // 遍历数目，结束条件处理
    if(m_ProduceRecordsCount <= 0){ // 函数递归的结束条件
        refreshProduceRecordsItems();// 更新表单
        return;
    }else if(m_count > m_ProduceRecordsCount) // 如果请求文件数量大于生产记录数目
    {
        m_count = m_ProduceRecordsCount;
    }

    QNetworkRequest request; // 请求对象

    //获取登录信息实例
    LoginInfoInstance *login = LoginInfoInstance::getInstance();    // 获取单例

    QString tmp = QString("ProduceRecordsnormal");

    QString url = QString("http://%1:%2/ProduceRecords?cmd=%3").arg(login->getIp()).arg(login->getPort()).arg(tmp);

    request.setUrl(QUrl(url));

    // 设置请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = setProduceRecordsListJson(login->getUser(),login->getToken(),m_start,m_count);

    //改变文件起始点位置
    m_start += m_count;
    m_ProduceRecordsCount -= m_count;

    //发送post请求
    QNetworkReply * reply = m_manager->post(request,data);
    if(reply == NULL){
        cout<<"getProduceRecordsList reply == NULL";
        return;
    }

    //获取请求的数据完成时，就会发送信号SIGNAL(finished())
    connect(reply,&QNetworkReply::finished,[=](){
       if(reply->error() != QNetworkReply::NoError){
           cout<<"getProduceRecordsList error: "<<reply->errorString();
           reply->deleteLater(); // 释放资源
           return;
       }

       // 服务器返回用户的数据
       QByteArray array = reply->readAll();
       //cout<<" ProduceRecords info: "<<array;

       reply->deleteLater();

       //token验证失败
       if("111" == m_cm.getCode(array)){
           QMessageBox::warning(this,"账户异常","请重新登录！");

           return;
       }

       // 不是错误码就处理文件列表json信息
       if("015" != m_cm.getCode(array)){
           // 解析生产记录列表json信息，存放在文件列表中
           getProduceRecordsJsonInfo(array);

           //继续获取生产记录信息列表
           getProduceRecordsList();
       }
    });
}

void ProduceRecords::getSearchList()
{
    //遍历数目，结束条件处理
    if(m_SearchCount <= 0) //结束条件，这个条件很重要，函数递归的结束条件
    {
        refreshProduceRecordsItems(); //更新item
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

    url = QString("http://%1:%2/ProduceRecords?cmd=ProduceRecordsresult").arg(login->getIp()).arg(login->getPort());
    request.setUrl(QUrl( url )); //设置url

    //qt默认的请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = setProduceRecordsListJson( login->getUser(), login->getToken(), s_start, s_count);

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
            // 解析生产记录列表json信息，存放在文件列表中
            getProduceRecordsJsonInfo(array);

            //继续获取生产记录信息列表
            getSearchList();
        }
    });
}

// 解析生产记录列表json信息，存放在文件列表中
void ProduceRecords::getProduceRecordsJsonInfo(QByteArray data)
{
    QJsonParseError error;

    // 将来源数据json转化为JsonDocument
    // 由QByteArray 对象构成一个QJsonDocument对象，用于读写操作
    QJsonDocument doc = QJsonDocument::fromJson(data,&error);
    if(error.error == QJsonParseError::NoError){
        if(doc.isNull() || doc.isEmpty()){
            cout<<" m_ProduceRecordsList doc.isNUll() || doc.isEmpty() ";
            return;
        }
        if(doc.isObject()){
            //QJsonObject json对象，描述json数据中{}括起来部分
            QJsonObject obj = doc.object();//取得最外层这个大对象

            //获取ProduceRecords对应的数组
            //QJsonArray json数组,描述json数据中[]括起来的部分
            QJsonArray array = obj.value("ProduceRecords").toArray();

            int size = array.size(); // 数组个数

            for(int i = 0;i < size;++i){
                QJsonObject tmp = array[i].toObject();  // 取第i个对象
                ProduceInfo *info = new ProduceInfo;
                info->produce_id = tmp.value("produce_id").toInt();
                info->product_name = tmp.value("product_name").toString();
                info->product_quantity = tmp.value("product_quantity").toInt();
                info->product_time = tmp.value("product_time").toString();
                info->RawMaterial = tmp.value("RawMaterial").toArray();  //--------------------------这里有待商榷
                //List添加节点
                m_ProduceRecordsList.append(info);

            }
        }
    }else{
        cout<<"getProduceRecordsJsonInfo error = "<<error.errorString();
    }
}

// 设置json包
QByteArray ProduceRecords::setGetCountJson(QString user, QString token)
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
QByteArray ProduceRecords::setProduceRecordsListJson(QString user, QString token, int start, int count)
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

//设置上传json包
QByteArray ProduceRecords::setUploadJson()
{
    QMap<QString,QVariant> tmp;
    tmp.insert("produce_id",id_Edit->text().toInt());
    tmp.insert("product_name",name_Edit->text());
    tmp.insert("product_quantity",number_Edit->text().toInt());
    tmp.insert("product_time",date_Edit->text());
    //tmp.insert("RawMaterial",sell_Edit->text());//--------------------------这里有待商榷

    QJsonDocument jsonDocument = QJsonDocument::fromVariant(tmp);
    if(jsonDocument.isNull()){
        cout<<"setUploadJson jsonDocument.isNull()";
        return "";
    }
    return jsonDocument.toJson();
}

//搜索生产记录信息
void ProduceRecords::search()
{
    // 清空文件列表信息
    clearProduceRecordsList();

    //将之前的ProduceRecordslist清空
    clearProduceRecordsItems();

    //将表格行数清零
    m_tableWidget->setRowCount(0);

    //获取生产记录信息数目
    m_SearchCount = 0;

    QNetworkRequest request;

    // 获取登陆信息实例
    // 获取单例
    LoginInfoInstance *login = LoginInfoInstance::getInstance();

    // 127.0.0.1:80/ProduceRecords?cmd=search
    // 获取生产记录信息数目
    QString url = QString("http://%1:%2/ProduceRecords?cmd=ProduceRecordssearch=%3").arg(login->getIp()).arg(login->getPort()).arg(QString::fromUtf8(Search_LineEdit->text().toUtf8().toBase64()));
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
            // 说明任然有生产记录
            s_start = 0;    //从0开始
            s_count = 10;   //每次请求10个

            // 获取新的生产记录列表信息
            getSearchList();
        }else{//没有文件
            refreshProduceRecordsItems(); //更新表
        }
    });
}

//添加生产记录信息
void ProduceRecords::add()
{
    cur_status = add_status;
    initEditWidget();
    ProduceRecords_Edit->show();
}

//将选中的表数据的行只作为json包
QByteArray ProduceRecords::setSelectJson(){
    QMap<QString,QVariant> tmp;
    QModelIndexList selectedRows = m_tableWidget->selectionModel()->selectedRows();
    foreach (QModelIndex index, selectedRows) {
        int row = index.row();
        tmp.insert("ProduceRecords_id",m_tableWidget->item(row, 0)->text().toInt());
        tmp.insert("ProduceRecords_name",m_tableWidget->item(row, 1)->text());
        tmp.insert("ProduceRecords_store_unit",m_tableWidget->item(row, 2)->text());
        tmp.insert("ProduceRecords_amount",m_tableWidget->item(row, 3)->text());
        tmp.insert("ProduceRecords_sell_unit",m_tableWidget->item(row, 4)->text());
        tmp.insert("ProduceRecords_price",m_tableWidget->item(row, 5)->text());
    }
    QJsonDocument jsonDocument = QJsonDocument::fromVariant(tmp);
    if(jsonDocument.isNull()){
        cout<<"setSelectJson jsonDocument.isNull()";
        return "";
    }
    return jsonDocument.toJson();
}

//删除生产记录信息
void ProduceRecords::remove()
{
    //将要上传的信息打包为json格式.
    QByteArray array = setSelectJson();

    QNetworkRequest request;
    LoginInfoInstance *login = LoginInfoInstance::getInstance();
    QString url= QString("http://%1:%2/ProduceRecords?cmd=ProduceRecordsdelete").arg(login->getIp()).arg(login->getPort());

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

//更新生产记录信息
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
        //material_table->setText(m_tableWidget->item(row, 4)->text());//--------------------------这里有待商榷

    }
    ProduceRecords_Edit->show();
}

void ProduceRecords::update_save_info()
{
    //将要上传的信息打包为json格式.
    QByteArray array = setUploadJson();

    QNetworkRequest request;
    LoginInfoInstance *login = LoginInfoInstance::getInstance();
    QString url;
    if(update_status == cur_status){
        url = QString("http://%1:%2/ProduceRecords?cmd=ProduceRecordsupdate").arg(login->getIp()).arg(login->getPort());
    }else if(add_status == cur_status){
        url = QString("http://%1:%2/ProduceRecords?cmd=ProduceRecordsadd").arg(login->getIp()).arg(login->getPort());
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
            number_Edit->clear();
            date_Edit->clear();
            material_table->clear();
            ProduceRecords_Edit->hide();
            refreshTable();
        }else{
            QMessageBox::warning(this,"上传失败","上传失败");
            ProduceRecords_Edit->hide();
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
    ProduceRecords_Edit->hide();
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
