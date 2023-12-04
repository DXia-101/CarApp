#ifndef USERWINDOW_H
#define USERWINDOW_H

#include <QWidget>
#include <QUrl>
#include <QListWidget>
#include <QStackedWidget>

#include "homepage.h"
#include "ShopWidget.h"

namespace Ui {
class UserWindow;
}

class QPushButton;

class UserWindow : public QWidget
{
    Q_OBJECT

public:
    explicit UserWindow(QWidget *parent = nullptr);

    ~UserWindow();

    void initWindow();

    void refreshHomePage();

    void InitUserName(const QString& name);
private slots:
    void on_homeBtn_clicked();

    void on_shopBtn_clicked();

    void on_mineBtn_clicked();

private:
    Ui::UserWindow *ui;

    QString UserName;

    HomePage *m_HomePage;

    QPushButton *homeBtn;
    QPushButton *shopBtn;
    QPushButton *mineBtn;

    QStackedWidget *stackedWidget;

    ShopWidget *shopWidget;
    QWidget *mineWidget;
};

#endif // USERWINDOW_H