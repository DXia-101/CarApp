#ifndef PRODUCT_H
#define PRODUCT_H

#include <QWidget>
#include "common/common.h"
#include <QTableWidget>


struct productInfo{
    int product_id;
    QString product_name;
    QString product_store_unit;
    quint16 product_amount;
    QString product_sell_unit;
    double product_price;
};

class QPushButton;
class QLineEdit;
class QLabel;

class Product : public QWidget
{
    Q_OBJECT
public:
    explicit Product(QWidget *parent = nullptr);
    ~Product();

    void initTableWidget();
    void initEditWidget();
    QStringList getCountStatus(QByteArray json);
    void refreshTable();
    void clearproductList();
    void clearproductItems();
    void refreshproductItems();
    void getproductList();
    void getSearchList();
    void getproductJsonInfo(QByteArray data);
    QByteArray setGetCountJson(QString user, QString token);
    QByteArray setproductListJson(QString user,QString token,int start,int count);
    QByteArray setUploadJson();
    QByteArray setSelectJson();

private:
    long m_productCount;
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

    QList<productInfo *> m_productList;

    Common m_cm;
    QNetworkAccessManager* m_manager;
    QWidget *product_Edit;
    QTableWidget *m_tableWidget;
    QPushButton *Add_Btn;
    QPushButton *Delete_Btn;
    QPushButton *Update_Btn;

    QPushButton *Search_Btn;
    QLineEdit *Search_LineEdit;

    QLineEdit *id_Edit;
    QLineEdit *name_Edit;
    QLineEdit *store_Edit;
    QLineEdit *amount_Edit;
    QLineEdit *sell_Edit;
    QLineEdit *price_Edit;
    QPushButton *update_Save_Btn;
    QPushButton *Edit_Cancel_Btn;

signals:

private slots:
    void search();
    void add();
    void remove();
    void update();
    void update_save_info();
    void cancel();

};

#endif // product_H
