#ifndef CUSTOMERORDERS_H
#define CUSTOMERORDERS_H

#include <QWidget>
#include "common/common.h"

namespace Ui {
class CustomerOrders;
}

class CustomerOrders : public QWidget
{
    Q_OBJECT

public:
    explicit CustomerOrders(QWidget *parent = nullptr);
    ~CustomerOrders();

protected:
    //初始化客户订单页面,主要加载客户名,具体是去获取服务器中不属于管理员的用户名
    void InitCustomerTree();

    //加载客户订单表
    void LoadCustomerOrderTable(const QString& UserName);


private slots:
    void on_CompleteBtn_clicked();

private:
    Ui::CustomerOrders *ui;

    Common m_cm;
    QNetworkAccessManager* m_manager;
};

#endif // CUSTOMERORDERS_H
