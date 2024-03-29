#ifndef PURCHASEITEM_H
#define PURCHASEITEM_H

#include <QWidget>

namespace Ui {
class PurchaseItem;
}

class PurchaseItem : public QWidget
{
    Q_OBJECT

public:
    explicit PurchaseItem(const QString &name,QWidget *parent = nullptr);
    ~PurchaseItem();

private slots:
    void on_cancel_btn_clicked();

    void on_Commodity_count_SpBox_valueChanged(int arg1);

private:
    Ui::PurchaseItem *ui;

    QString Commodity_name;
};

#endif // PURCHASEITEM_H
