#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <QWidget>
#include <QTimer>
#include <QVBoxLayout>
#include "common/common.h"

struct ShowProInfo{
    int showpro_id;
    QString showpro_name;
    QString showpro_url;
};


namespace Ui {
class HomePage;
}

class HomePage : public QWidget
{
    Q_OBJECT

public:
    explicit HomePage(QWidget *parent = nullptr);
    ~HomePage();

    void initListWidget();
    void initServerSQL();

    void getShowProList();

    void getshowproJsonInfo(QByteArray data);
    void clearshowproItems();
    void refreshshowproItems();

    QByteArray setGetCountJson(QString user, QString token);
    QStringList getCountStatus(QByteArray json);
    QByteArray setshowproListJson(QString user, QString token, int start, int count);
private:
    Ui::HomePage *ui;
    Common m_cm;
    QNetworkAccessManager* m_manager;

    QList<ShowProInfo *> m_showproList;

    long m_ShowProCount;
    int m_start;
    int m_count;

    int totalHeight = 0;

    QVBoxLayout *layout;
};

#endif // HOMEPAGE_H
