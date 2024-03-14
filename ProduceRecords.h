#ifndef PRODUCERECORDS_H
#define PRODUCERECORDS_H

#include <QWidget>
#include "common/common.h"
#include <QJsonArray>
#include <QTableWidget>

struct ProduceInfo{
    int produce_id;
    QString product_name;
    int product_quantity;
    QString product_time;
    QJsonArray RawMaterial;
};

class QPushButton;
class QLineEdit;
class QLabel;

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
    QJsonArray materialInfo;

    enum add_or_update{
        add_status = 0,
        update_status = 1,
    };

    int cur_status;

    QList<ProduceInfo *> m_ProduceRecordsList;

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
    QLineEdit *number_Edit;
    QLineEdit *date_Edit;
    QTableWidget *material_table;
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
    void SaveMaterialInfo();
};

#endif // PRODUCERECORDS_H
