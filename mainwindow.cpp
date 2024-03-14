#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QApplication>
#include <QTreeWidget>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    InitTreeWidget();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::InitTreeWidget()
{
    ui->treeWidget->setColumnCount(1);
    QStringList infoList;
    infoList<<"汽车内饰库存管理系统";
    QTreeWidgetItem *treeHead = new QTreeWidgetItem(ui->treeWidget,infoList);
    ui->treeWidget->addTopLevelItem(treeHead);
    infoList.clear();
    infoList<<"原材料库存";
    QTreeWidgetItem *twig_1 = new QTreeWidgetItem(treeHead,infoList);
    infoList.clear();
    infoList<<"产品库存";
    QTreeWidgetItem *twig_2 = new QTreeWidgetItem(treeHead,infoList);
    infoList.clear();
    infoList<<"生产记录";
    QTreeWidgetItem *twig_3 = new QTreeWidgetItem(treeHead,infoList);
    infoList.clear();
    infoList<<"采购记录";
    QTreeWidgetItem *twig_4 = new QTreeWidgetItem(treeHead,infoList);
    infoList.clear();
    infoList<<"客户订单";
    QTreeWidgetItem *twig_5 = new QTreeWidgetItem(treeHead,infoList);
    infoList.clear();
    infoList<<"项目报表";
    QTreeWidgetItem *twig_6 = new QTreeWidgetItem(treeHead,infoList);
    treeHead->addChild(twig_1);
    treeHead->addChild(twig_2);
    treeHead->addChild(twig_3);
    treeHead->addChild(twig_4);
    treeHead->addChild(twig_5);
    treeHead->addChild(twig_6);
    ui->treeWidget->expandAll();

    //初始化商品表
    wares = new Wares();
    ui->StackWidget->addWidget(wares);
    ui->StackWidget->setCurrentWidget(wares);
    product = new Product();
    ui->StackWidget->addWidget(product);
    userorder = new UserOrderInterface();
    ui->StackWidget->addWidget(userorder);
    reportforms = new ReportForms();
    ui->StackWidget->addWidget(reportforms);
    produce = new ProduceRecords();
    ui->StackWidget->addWidget(produce);

}

void MainWindow::on_Logout_Btn_clicked()
{
    exit(0);
}

void MainWindow::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    if(item->text(column)=="原材料库存"){
        ui->StackWidget->setCurrentWidget(wares);
    }
    if(item->text(column)=="产品库存"){
        ui->StackWidget->setCurrentWidget(product);
    }
    if(item->text(column)=="客户订单"){
        ui->StackWidget->setCurrentWidget(userorder);
    }
    if(item->text(column)=="项目报表"){
        ui->StackWidget->setCurrentWidget(reportforms);
    }
    if(item->text(column)=="生产记录"){
        ui->StackWidget->setCurrentWidget(produce);
    }
}

