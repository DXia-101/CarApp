#ifndef PRODUCTITEM_H
#define PRODUCTITEM_H

#include <QUrl>
#include <QWidget>

namespace Ui {
class ProductItem;
}

class ProductItem : public QWidget
{
    Q_OBJECT

public:
    explicit ProductItem(int id,const QUrl& imageUrl,const QString& name,QWidget *parent = nullptr);
    ~ProductItem();

    int GetCommodityID();
    QString GetCommodityName();

private slots:
    void on_confirm_btn_clicked();

private:
    void setImageAndName(const QUrl& imageUrl, const QString& name);

    Ui::ProductItem *ui;

    int Commodity_id;
    QString Commodity_name;
    QUrl Commodity_url;

    int Commodity_count;
};

#endif // PRODUCTITEM_H
