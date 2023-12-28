#ifndef USERORDERINTERFACE_H
#define USERORDERINTERFACE_H

#include <QTreeWidgetItem>
#include <QWidget>
#include "common/common.h"
#include <QVector>
#include "UserOrderTable.h"

namespace Ui {
class UserOrderInterface;
}

class UserOrderInterface : public QWidget
{
    Q_OBJECT

public:
    explicit UserOrderInterface(QWidget *parent = nullptr);
    ~UserOrderInterface();

protected:
    void InitUserTree();
    void GetUserNames();
    void AddNameToTree(const QVector<QString>& namesList);
    void CreateUserOrderTable(const QVector<QString>& namesList);
private slots:
    void on_UserTree_itemClicked(QTreeWidgetItem *item, int column);

private:
    Ui::UserOrderInterface *ui;
    Common m_cm;
    QNetworkAccessManager* m_manager;

    QVector<QString> userNamesList;
    QVector<UserOrderTable*> userOrderTableList;
};

#endif // USERORDERINTERFACE_H
