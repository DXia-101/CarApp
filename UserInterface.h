#ifndef USERINTERFACE_H
#define USERINTERFACE_H

#include <QWidget>

namespace Ui {
class UserInterface;
}

class UserInterface : public QWidget
{
    Q_OBJECT

public:
    explicit UserInterface(QString Name,QWidget *parent = nullptr);
    ~UserInterface();

private:
    Ui::UserInterface *ui;
    QString userName;
};

#endif // USERINTERFACE_H
