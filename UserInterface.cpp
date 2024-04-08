#include "UserInterface.h"
#include "ui_UserInterface.h"

UserInterface::UserInterface(QString Name,QWidget *parent)
    :QWidget(parent)
    , ui(new Ui::UserInterface)
{
    ui->setupUi(this);

}

UserInterface::~UserInterface()
{
    delete ui;
}
