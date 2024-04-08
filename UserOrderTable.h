#ifndef USERORDERTABLE_H
#define USERORDERTABLE_H

#include <QWidget>
#include "common/common.h"
#include <QTableWidget>

struct UserOrderTableInfo{
    quint16 OrderID;
    QString ProductName;
    quint16 count;
    QString timestamp;
};

class QPushButton;
class QLineEdit;
class QLabel;

class UserOrderTable : public QWidget
{
    Q_OBJECT
public:
    explicit UserOrderTable(QString userName,QWidget *parent = nullptr);
    ~UserOrderTable();

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
    QByteArray setSelectJson();

private:
    QString UserName;
    long m_UserOrderCount;
    long m_SearchCount;
    int m_start;
    int m_count;
    int s_start;
    int s_count;

    enum add_or_update{
        add_status = 0,
        update_status = 1,
    };
    int cur_status;

    QList<UserOrderTableInfo *> m_UserOrderList;

    Common m_cm;
    QNetworkAccessManager* m_manager;
    QWidget *UserOrder_Edit;
    QTableWidget *m_tableWidget;

    QPushButton *Add_Btn;
    QPushButton *Delete_Btn;
    QPushButton *Update_Btn;

    QPushButton *Search_Btn;
    QLineEdit *Search_LineEdit;

    QLineEdit *UserOrder_OrderID_Edit;
    QLineEdit *UserOrder_ProductName_Edit;
    QLineEdit *UserOrder_Count_Edit;

    QPushButton *update_Save_Btn;
    QPushButton *Edit_Cancel_Btn;

signals:

private slots:
    void search();
    void remove();
    void update();
    void update_save_info();
    void cancel();

};

#endif // USERORDERTABLE_H
