#ifndef USERORDERINTERFACE_H
#define USERORDERINTERFACE_H

#include <QTreeWidgetItem>
#include <QWidget>
#include "common/common.h"

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
private slots:
    void on_UserTree_itemClicked(QTreeWidgetItem *item, int column);

private:
    Ui::UserOrderInterface *ui;
    Common m_cm;
    QNetworkAccessManager* m_manager;

    QVector<QString> userNamesList;
};

#endif // USERORDERINTERFACE_H
