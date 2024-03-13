#ifndef WARES_H
#define WARES_H

#include <QWidget>
#include "common/common.h"
#include <QTableWidget>


struct WaresInfo{
    int wares_id;
    QString wares_name;
    QString wares_store_unit;
    quint16 wares_amount;
    QString wares_sell_unit;
    double wares_price;
};



class QPushButton;
class QLineEdit;
class QLabel;

class Wares : public QWidget
{
    Q_OBJECT
public:
    explicit Wares(QWidget *parent = nullptr);
    ~Wares();

    void initTableWidget();
    void initEditWidget();
    QStringList getCountStatus(QByteArray json);
    void refreshTable();
    void clearWaresList();
    void clearWaresItems();
    void refreshWaresItems();
    void getWaresList();
    void getSearchList();
    void getWaresJsonInfo(QByteArray data);
    QByteArray setGetCountJson(QString user, QString token);
    QByteArray setWaresListJson(QString user,QString token,int start,int count);
    QByteArray setUploadJson();
    QByteArray setSelectJson();

private:
    long m_WaresCount;
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

    QList<WaresInfo *> m_waresList;

    Common m_cm;
    QNetworkAccessManager* m_manager;
    QWidget *Wares_Edit;
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

#endif // WARES_H
