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

    // 初始化商品信息表页面
    void initTableWidget();

    // 初始化更改增加页面
    void initEditWidget();

    // 得到服务器json文件
    QStringList getCountStatus(QByteArray json);

    // 显示商品信息
    void refreshTable();

    // 清空商品列表
    void clearWaresList();

    // 清空所有商品Item
    void clearWaresItems();

    // 商品Item展示
    void refreshWaresItems();

    // 获取商品信息列表
    void getWaresList();

    // 获取搜索的商品列表
    void getSearchList();

    // 解析商品列表json信息，存放在文件列表中
    void getWaresJsonInfo(QByteArray data);

    // 设置json包
    QByteArray setGetCountJson(QString user, QString token);

    QByteArray setWaresListJson(QString user,QString token,int start,int count);

    //设置上传json包
    QByteArray setUploadJson();

    //将选中的表数据的行只作为json包
    QByteArray setSelectJson();

private:
    //商品信息数目
    long m_WaresCount;
    //商品检索数目
    long m_SearchCount;
    //商品信息位置起点
    int m_start;
    //每次请求信息个数
    int m_count;
    //检索信息位置起点
    int s_start;
    //每次检索信息个数
    int s_count;

    //更新或上传
    enum add_or_update{
        add_status = 0,
        update_status = 1,
    };
    int cur_status;


    //商品列表
    QList<WaresInfo *> m_waresList;

    Common m_cm;
    QNetworkAccessManager* m_manager;

    //增加和改变的页面
    QWidget *Wares_Edit;

    //商品表
    QTableWidget *m_tableWidget;

    //增删改查四个按钮以及搜索框
    QPushButton *Add_Btn;
    QPushButton *Delete_Btn;
    QPushButton *Update_Btn;

    QPushButton *Search_Btn;
    QLineEdit *Search_LineEdit;

    //增加和修改信息输入框
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
    //搜索商品信息
    void search();
    //添加商品信息
    void add();
    //删除商品信息
    void remove();
    //更新商品信息
    void update();
    //保存信息到服务器
    void update_save_info();
    //取消保存
    void cancel();

};

#endif // WARES_H
