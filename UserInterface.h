#ifndef USERINTERFACE_H
#define USERINTERFACE_H

#include <QWidget>
#include "common/common.h"

namespace Ui {
class UserInterface;
}

struct UserOrderTableInfo{
    quint16 OrderID;
    QString ProductName;
    quint16 count;
    QString timestamp;
    QString deliver;
};

class UserInterface : public QWidget
{
    Q_OBJECT

public:
    explicit UserInterface(QString Name,QWidget *parent = nullptr);
    ~UserInterface();

    void initTableWidget();
    void initEditWidget();
    QStringList getCountStatus(QByteArray json);
    void refreshTable();
    void clearUserOrderList();
    void clearUserOrderItems();
    void refreshUserOrderItems();
    void getUserOrderList();
    void getSearchList();
    void getUserOrderJsonInfo(QByteArray data);
    QByteArray setGetCountJson(QString user, QString token);
    QByteArray setUserOrderListJson(QString user,QString token,int start,int count);
    QByteArray setUploadJson();
private:
    Ui::UserInterface *ui;

    QString UserName;
    long m_UserOrderCount;
    long m_SearchCount;
    int m_start;
    int m_count;
    int s_start;
    int s_count;

    QList<UserOrderTableInfo *> m_UserOrderList;

    Common m_cm;
    QNetworkAccessManager* m_manager;

private slots:
    void search();
    void update();
    void update_save_info();
};

#endif // USERINTERFACE_H
