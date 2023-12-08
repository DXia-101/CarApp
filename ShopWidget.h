#ifndef SHOPWIDGET_H
#define SHOPWIDGET_H

#include <QVBoxLayout>
#include <QWidget>

namespace Ui {
class ShopWidget;
}

#include "common/common.h"

class ShopWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ShopWidget(QWidget *parent = nullptr);
    ~ShopWidget();

    void refreshPurchaseItems();

    void SetUserName(const QString& name);
protected:
    QByteArray GetAllPurchaseJson();
private slots:
    void on_settlementBtn_clicked();

    void on_cancelBtn_clicked();

private:
    Ui::ShopWidget *ui;

    QString UserName;

    Common m_cm;
    QNetworkAccessManager* m_manager;

    QVBoxLayout *layout;
    int totalHeight;
};

#endif // SHOPWIDGET_H
