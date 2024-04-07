#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QWidget>
#include "common/common.h"
#include <QTableWidget>

struct UserManagerInfo{
    int UserId;
    QString UserName;
    QString UserNickName;
    QString password;
    QString Phone;
    QString CreateTime;
    QString Email;
    int Power;
};

class QPushButton;
class QLineEdit;
class QLabel;
class QCheckBox;

class UserManager : public QWidget
{
    Q_OBJECT
public:
    explicit UserManager(QWidget *parent = nullptr);
    ~UserManager();
    void initTableWidget();
    void initEditWidget();
    QStringList getCountStatus(QByteArray json);
    void refreshTable();
    void clearUserManagerList();
    void clearUserManagerItems();
    void refreshUserManagerItems();
    void getUserManagerList();
    void getSearchList();
    void getUserManagerJsonInfo(QByteArray data);
    QByteArray setGetCountJson(QString user, QString token);
    QByteArray setUserManagerListJson(QString user,QString token,int start,int count);
    QByteArray setUploadJson();
    QByteArray setSelectJson();


private:
    long m_UserManagerCount;
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

    QList<UserManagerInfo *> m_UserManagerList;

    Common m_cm;
    QNetworkAccessManager* m_manager;
    QWidget *UserManager_Edit;
    QTableWidget *m_tableWidget;
    QPushButton *Add_Btn;
    QPushButton *Delete_Btn;
    QPushButton *Update_Btn;

    QPushButton *Search_Btn;
    QLineEdit *Search_LineEdit;

    QLineEdit *UserId_Edit;
    QLineEdit *UserName_Edit;
    QLineEdit *UserNickName_Edit;
    QLineEdit *password_Edit;
    QLineEdit *Phone_Edit;
    QLineEdit *CreateTime_Edit;
    QLineEdit *Email_Edit;
    QLineEdit *Power_Edit;
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

#endif // USERMANAGER_H
