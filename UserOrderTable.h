#ifndef USERORDERTABLE_H
#define USERORDERTABLE_H

#include <QWidget>
#include "common/common.h"
#include <QTableWidget>

struct UserOrderTableInfo{
    quint16 UserOrderTable_OrderID;
    QString UserOrderTable_Productname;
    quint16 UserOrderTable_count;
    QString UserOrderTable_time;
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

    // 初始化用户订单信息表页面
    void initTableWidget();

    // 初始化更改增加页面
    void initEditWidget();

    // 得到服务器json文件
    QStringList getCountStatus(QByteArray json);

    // 显示用户订单信息
    void refreshTable();

    // 清空用户订单列表
    void clearUserOrderList();

    // 清空所有用户订单Item
    void clearUserOrderItems();

    // 用户订单Item展示
    void refreshUserOrderItems();

    // 获取用户订单信息列表
    void getUserOrderList();

    // 获取搜索的用户订单列表
    void getSearchList();

    // 解析用户订单列表json信息，存放在文件列表中
    void getUserOrderJsonInfo(QByteArray data);

    // 设置json包
    QByteArray setGetCountJson(QString user, QString token);

    QByteArray setUserOrderListJson(QString user,QString token,int start,int count);

    //设置上传json包
    QByteArray setUploadJson();

    //将选中的表数据的行只作为json包
    QByteArray setSelectJson();

private:
    QString UserName;
    //用户订单信息数目
    long m_UserOrderCount;
    //用户订单检索数目
    long m_SearchCount;
    //用户订单信息位置起点
    int m_start;
    //每次请求信息个数
    int m_count;
    //检索信息位置起点
    int s_start;
    //每次检索信息个数
    int s_count;

    //更新或上传
    enum add_or_update{
        add_status = 0,
        update_status = 1,
    };
    int cur_status;


    //用户订单列表
    QList<UserOrderTableInfo *> m_UserOrderList;

    Common m_cm;
    QNetworkAccessManager* m_manager;

    //增加和改变的页面
    QWidget *UserOrder_Edit;

    //用户订单表
    QTableWidget *m_tableWidget;

    //增删改查四个按钮以及搜索框
    QPushButton *Add_Btn;
    QPushButton *Delete_Btn;
    QPushButton *Update_Btn;

    QPushButton *Search_Btn;
    QLineEdit *Search_LineEdit;

    //增加和修改信息输入框
    QLineEdit *UserOrder_ProductName_Edit;
    QLineEdit *UserOrder_Count_Edit;

    QPushButton *update_Save_Btn;
    QPushButton *Edit_Cancel_Btn;

signals:

private slots:
    //搜索用户订单信息
    void search();
    //删除用户订单信息
    void remove();
    //更新用户订单信息
    void update();
    //保存信息到服务器
    void update_save_info();
    //取消保存
    void cancel();

};

#endif // USERORDERTABLE_H
