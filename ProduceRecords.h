#ifndef PRODUCERECORDS_H
#define PRODUCERECORDS_H

#include <QWidget>

struct ProduceInfo{
    int produce_id;
    QString product_name;
    int product_quantity;
    QString product_time;
    QJsonArray RawMaterial;
};


class ProduceRecords : public QWidget
{
    Q_OBJECT
public:
    explicit ProduceRecords(QWidget *parent = nullptr);
    ~ProduceRecords();

    void initTableWidget();
    void initEditWidget();
    QStringList getCountStatus(QByteArray json);
    void refreshTable();
    void clearProduceRecordsList();
    void clearProduceRecordsItems();
    void refreshProduceRecordsItems();
    void getProduceRecordsList();
    void getSearchList();
    void getProduceRecordsJsonInfo(QByteArray data);
    QByteArray setGetCountJson(QString user, QString token);
    QByteArray setProduceRecordsListJson(QString user,QString token,int start,int count);
    QByteArray setUploadJson();
    QByteArray setSelectJson();
private:
    long m_ProduceRecordsCount;
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

    QList<productInfo *> m_ProduceRecordsList;

    Common m_cm;
    QNetworkAccessManager* m_manager;
    QWidget *ProduceRecords_Edit;
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

#endif // PRODUCERECORDS_H
