#include "Purchase.h"
#include <QDebug>
Purchase* Purchase::instance = new Purchase;

Purchase *Purchase::getInstance()
{
    return instance;
}

void Purchase::destory()
{
    if(NULL != Purchase::instance){
        delete Purchase::instance;
        Purchase::instance = NULL;
        qDebug()<<"Purchase instance is delete";
    }
}

int Purchase::GetPurchaseCount(const QString &name)
{
    return purchase[name];
}

bool Purchase::InsertPurchase(const QString &name, int count)
{
    auto it = purchase.insert(name,count);
    if (it != purchase.end()) {
        purchaseNames.push_back(name);
        return true;
    } else {
        return false;
    }
}

bool Purchase::RemovePurchase(const QString &name)
{
    int count = purchase.remove(name);
    if (count > 0) {
        purchaseNames.removeOne(name);
        return true;
    } else {
        return false;
    }
}

int Purchase::GetPurchaseSize()
{
    return purchaseNames.size();
}

bool Purchase::purchaseisEmpty()
{
    return purchaseNames.isEmpty();
}

QString Purchase::purchaseAt(int index)
{
    return purchaseNames.at(index);
}

bool Purchase::RemoveAllPurchase()
{
    purchase.clear();
}

Purchase::Purchase()
{

}
