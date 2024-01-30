#ifndef PURCHASE_H
#define PURCHASE_H

#include <QMap>
#include <QString>

class Purchase
{
public:
    static Purchase *getInstance();

    static void destory();

    int GetPurchaseCount(const QString &name);

    bool InsertPurchase(const QString &name,int count);

    bool RemovePurchase(const QString &name);

    void UpdatePurchase(const QString &name,int value);

    int GetPurchaseSize();
    bool purchaseisEmpty();
    QString purchaseAt(int index);

    void RemoveAllPurchase();
private:
    static Purchase *instance;

    QMap<QString,int> purchase;
    QVector<QString> purchaseNames;

    Purchase();
};

#endif // PURCHASE_H
