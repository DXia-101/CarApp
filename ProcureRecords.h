#ifndef PROCURERECORDS_H
#define PROCURERECORDS_H

#include <QWidget>
#include "common/common.h"
#include <QTableWidget>

struct ProcureInfo{
    int procure_id;
    QString material_name;
    int material_quantity;
    QString procure_time;
};

class QPushButton;
class QLineEdit;
class QLabel;

class ProcureRecords : public QWidget
{
    Q_OBJECT
public:
    explicit ProcureRecords(QWidget *parent = nullptr);
    ~ProcureRecords();

    void initTableWidget();
    void initEditWidget();
    QStringList getCountStatus(QByteArray json);
    void refreshTable();
    void clearProcureList();
    void clearProcureItems();
    void refreshProcureItems();
    void getProcureList();
    void getSearchList();
    void getProcureJsonInfo(QByteArray data);
    QByteArray setGetCountJson(QString user, QString token);
    QByteArray setProcureListJson(QString user,QString token,int start,int count);
    QByteArray setUploadJson();
    QByteArray setSelectJson();


private:
    long m_ProcureCount;
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

    QList<ProcureInfo *> m_ProcureList;

    Common m_cm;
    QNetworkAccessManager* m_manager;
    QWidget *Procure_Edit;
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

#endif // PROCURERECORDS_H
