#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include "qtablewidget.h"
#include "wares.h"
#include "product.h"
#include "UserOrderInterface.h"
#include "ReportForms.h"
#include "ProduceRecords.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QTableWidget;
class QTreeWidgetItem;

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    //初始化选项树
    void InitTreeWidget();

private slots:
    //退出按钮
    void on_Logout_Btn_clicked();
    void on_treeWidget_itemClicked(QTreeWidgetItem *item, int column);

private:
    Ui::MainWindow *ui;

    Wares* wares;
    Product* product;
    UserOrderInterface* userorder;
    ReportForms* reportforms;
    ProduceRecords* produce;
};
#endif // MAINWINDOW_H
