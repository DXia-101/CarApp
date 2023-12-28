#include "UserOrderInterface.h"
#include "ui_UserOrderInterface.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QNetworkRequest>
#include "common/logininfoinstance.h"

UserOrderInterface::UserOrderInterface(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UserOrderInterface)
{
    ui->setupUi(this);
    m_manager = Common::getNetManager();
    InitUserTree();
}

UserOrderInterface::~UserOrderInterface()
{
    delete ui;
}

void UserOrderInterface::InitUserTree()
{
    GetUserNames();

}

void UserOrderInterface::GetUserNames()
{
    QNetworkRequest request;
    LoginInfoInstance *login = LoginInfoInstance::getInstance();
    QString url = QString("http://%1:%2/UserOrder?cmd=GetNames").arg(login->getIp()).arg(login->getPort());
    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    QNetworkReply* reply = m_manager->get(request);
    if(reply == NULL){
        qDebug()<<"reply == NULL";
        return;
    }
    connect(reply,&QNetworkReply::finished,[=](){
        if(reply->error() != QNetworkReply::NoError)
        {
            cout<<reply->errorString();
            reply->deleteLater();//释放资源
            return;
        }
        QByteArray array = reply->readAll();
        reply->deleteLater();
        QJsonDocument jsonDocument = QJsonDocument::fromJson(array);
        QJsonArray jsonArray = jsonDocument.array();
        for(const QJsonValue& value : jsonArray){
            QString string = value.toString();
            if("kevin_666"!=string){
                userNamesList.push_back(string);
            }
        }
        AddNameToTree(userNamesList);
        CreateUserOrderTable(userNamesList);
    });
}

void UserOrderInterface::AddNameToTree(const QVector<QString>& namesList)
{
    ui->UserTree->setColumnCount(1);
    QStringList infoList;
    infoList<<"客户名称";
    QTreeWidgetItem *treeHead = new QTreeWidgetItem(ui->UserTree,infoList);
    ui->UserTree->addTopLevelItem(treeHead);
    for(int i = 0;i < namesList.size();++i){
        infoList.clear();
        infoList<<namesList[i];
        QTreeWidgetItem *twig = new QTreeWidgetItem(treeHead,infoList);
        treeHead->addChild(twig);
    }
    ui->UserTree->expandAll();
}

void UserOrderInterface::CreateUserOrderTable(const QVector<QString> &namesList)
{
    for(int i = 0;i < namesList.size();++i){
        UserOrderTable* tempTable = new UserOrderTable(namesList[i]);
        ui->stackedWidget->addWidget(tempTable);
        userOrderTableList.emplace_back(tempTable);
    }
}

void UserOrderInterface::on_UserTree_itemClicked(QTreeWidgetItem *item, int column)
{
    for(int i = 0;i < userNamesList.size();++i){
        if(item->text(column) == userNamesList[i]){
            ui->stackedWidget->setCurrentWidget(userOrderTableList[i]);
        }
    }
}

