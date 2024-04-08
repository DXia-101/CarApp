#include "login.h"
#include "ui_login.h"
#include "userwindow.h"

#include "common/des.h"
#include "common/logininfoinstance.h"

#include <QPainter>
#include <QDebug>
#include <QFile>
#include <QApplication>
#include <QMessageBox>
#include <QRegularExpression>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>


Login::Login(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Login)
{
    ui->setupUi(this);

    m_manager = Common::getNetManager();

    this->setWindowIcon(QIcon(":/images/logo.ico"));

    this->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint);
    this->setFont(QFont("新宋体", 12, QFont::Bold, false));
    ui->login_pwd->setEchoMode(QLineEdit::Password);
    ui->reg_pwd->setEchoMode(QLineEdit::Password);
    ui->reg_surepwd->setEchoMode(QLineEdit::Password);

    ui->stackedWidget->setCurrentIndex(0);
    ui->login_usr->setFocus();

    ui->login_usr->setToolTip("合法字符:[a-z|A-Z|#|@|0-9|-|_|*],字符个数: 3~16");
    ui->reg_usr->setToolTip("合法字符:[a-z|A-Z|#|@|0-9|-|_|*],字符个数: 3~16");
    ui->login_pwd->setToolTip("合法字符:[a-z|A-Z|#|@|0-9|-|_|*],字符个数: 6~18");
    ui->reg_pwd->setToolTip("合法字符:[a-z|A-Z|#|@|0-9|-|_|*],字符个数: 6~18");
    ui->reg_surepwd->setToolTip("合法字符:[a-z|A-Z|#|@|0-9|-|_|*],字符个数: 6~18");

    readCfg();

#if 0
    ui->reg_usr->setText("kevin_666");
    ui->reg_nickname->setText("kevin@666");
    ui->reg_pwd->setText("123456");
    ui->reg_surepwd->setText("123456");
    ui->reg_phone->setText("11111111111");
    ui->reg_mail->setText("abc@qq.com");
#endif

    // 注册
    connect(ui->login_register_btn, &QToolButton::clicked, [=]()
    {
        // 切换到注册界面
        ui->stackedWidget->setCurrentWidget(ui->register_page);
        ui->reg_usr->setFocus();
    });
    // 设置按钮
    connect(ui->title_widget, &TitleWidget::showSetWidget, [=]()
    {
        // 切换到设置界面
        ui->stackedWidget->setCurrentWidget(ui->set_page);
        ui->address_server->setFocus();
    });
    // 关闭按钮
    connect(ui->title_widget, &TitleWidget::closeWindow, [=]()
    {
        // 如果是注册窗口
        if(ui->stackedWidget->currentWidget() == ui->register_page)
        {
            // 清空数据
            ui->reg_mail->clear();
            ui->reg_usr->clear();
            ui->reg_nickname->clear();
            ui->reg_pwd->clear();
            ui->reg_surepwd->clear();
            ui->reg_phone->clear();
            // 窗口切换
            ui->stackedWidget->setCurrentWidget(ui->login_page);
            ui->login_usr->setFocus();
        }
        // 如果是设置窗口
        else if(ui->stackedWidget->currentWidget() == ui->set_page)
        {
            // 清空数据
            ui->address_server->clear();
            ui->port_server->clear();
            // 窗口切换
            ui->stackedWidget->setCurrentWidget(ui->login_page);
            ui->login_usr->setFocus();
        }
        // 如果是登录窗口
        else if(ui->stackedWidget->currentWidget() == ui->login_page)
        {
            close();
        }
    });
}

Login::~Login()
{
    delete ui;
}

// 登陆用户需要使用的json数据包
QByteArray Login::setLoginJson(QString user, QString pwd)
{
    QMap<QString,QVariant> login;
    login.insert("user",user);
    login.insert("pwd",pwd);

    QJsonDocument jsonDocument = QJsonDocument::fromVariant(login);
    if(jsonDocument.isNull()){
        cout<<" jsonDocument.isNull()";
        return "";
    }

    return jsonDocument.toJson();
}

// 注册用户需要使用的json数据包
QByteArray Login::setRegisterJson(QString userName, QString nickName, QString firstPwd, QString phone, QString email)
{
    QMap<QString, QVariant> reg;
    reg.insert("userName", userName);
    reg.insert("nickName", nickName);
    reg.insert("firstPwd", firstPwd);
    reg.insert("phone", phone);
    reg.insert("email", email);

    QJsonDocument jsonDocument = QJsonDocument::fromVariant(reg);
    if ( jsonDocument.isNull() )
    {
        cout << " jsonDocument.isNull() ";
        return "";
    }

    return jsonDocument.toJson();
}

// 得到服务器回复的登陆状态， 状态码返回值为 "000", 或 "001"，还有登陆section
QStringList Login::getLoginStatus(QByteArray json)
{
    QJsonParseError error;
    QStringList list;

    
    // 由QByteArray对象构造一个QJsonDocument对象，用于我们的读写操作
    QJsonDocument doc = QJsonDocument::fromJson(json,&error);
    if (error.error == QJsonParseError::NoError)
    {
        if (doc.isNull() || doc.isEmpty())
        {
            cout << "doc.isNull() || doc.isEmpty()";
            return list;
        }

        if( doc.isObject() )
        {
            
            QJsonObject obj = doc.object();
            cout << "服务器返回的数据" << obj;
            //状态码
            list.append( obj.value( "code" ).toString() );
            //登陆token
            list.append( obj.value( "token" ).toString() );
        }
    }
    else
    {
        cout << "err = " << error.errorString();
    }

    return list;
}

// 读取配置信息，设置默认登录状态，默认设置信息
void Login::readCfg()
{
    QString user = m_cm.getCfgValue("login", "user");
    QString pwd = m_cm.getCfgValue("login", "pwd");
    QString remeber = m_cm.getCfgValue("login", "remember");
    int ret = 0;

    if(remeber == "yes")//记住密码
    {
        //密码解密
        unsigned char encPwd[512] = {0};
        int encPwdLen = 0;
        //toLocal8Bit(), 转换为本地字符集，默认windows则为gbk编码，linux为utf-8编码
        QByteArray tmp = QByteArray::fromBase64( pwd.toLocal8Bit());
        ret = DesDec( (unsigned char *)tmp.data(), tmp.size(), encPwd, &encPwdLen);
        if(ret != 0)
        {
            cout << "DesDec";
            return;
        }

    #ifdef _WIN32 //如果是windows平台
        ui->login_pwd->setText( QString::fromLocal8Bit( (const char *)encPwd, encPwdLen ) );
    #else //其它平台
        ui->login_pwd->setText( (const char *)encPwd );
    #endif

        ui->rember_pwd->setChecked(true);

    }
    else //没有记住密码
    {
        ui->login_pwd->setText("");
        ui->rember_pwd->setChecked(false);
    }

    //用户解密
    unsigned char encUsr[512] = {0};
    int encUsrLen = 0;
    //toLocal8Bit(), 转换为本地字符集，如果windows则为gbk编码，如果linux则为utf-8编码
    QByteArray tmp = QByteArray::fromBase64( user.toLocal8Bit());
    ret = DesDec( (unsigned char *)tmp.data(), tmp.size(), encUsr, &encUsrLen);
    if(ret != 0)
    {
        cout << "DesDec";
        return;
    }

    #ifdef _WIN32 //如果是windows平台
        //fromLocal8Bit(), 本地字符集转换为utf-8
        ui->login_usr->setText( QString::fromLocal8Bit( (const char *)encUsr, encUsrLen ) );
    #else //其它平台
        ui->login_usr->setText( (const char *)encUsr );
    #endif

    QString ip = m_cm.getCfgValue("web_server", "ip");
    QString port = m_cm.getCfgValue("web_server", "port");
    ui->address_server->setText(ip);
    ui->port_server->setText(port);
}

void Login::on_login_btn_clicked()
{
    // 获取用户登录信息
    QString user = ui->login_usr->text();
    QString pwd = ui->login_pwd->text();
    QString address = ui->address_server->text();
    QString port = ui->port_server->text();

#if 0
    // 数据校验
    QRegularExpression regexp(USER_REG);
    if(!regexp.match(user).hasMatch())
    {
        QMessageBox::warning(this, "警告", "用户名格式不正确");
        ui->login_usr->clear();
        ui->login_usr->setFocus();
        return;
    }

    regexp.setPattern(PASSWD_REG);
    if(!regexp.match(pwd).hasMatch())
    {
        QMessageBox::warning(this, "警告", "密码格式不正确");
        ui->login_pwd->clear();
        ui->login_pwd->setFocus();
        return;
    }
#endif

    // 登录信息写入配置文件cfg.json
//    QString address = ui->address_server->text();
//    QString port = ui->port_server->text();

    QJsonObject loginObj;
    loginObj["user"] = user;
    loginObj["pwd"] = m_cm.getStrMd5(pwd);
    loginObj["remember"] = ui->rember_pwd->isChecked() ? "yes" : "no";

    QJsonObject cfgObj;
    cfgObj["login"] = loginObj;
    cfgObj["type_path"] = QJsonObject{{"path", "conf/fileType"}};
    cfgObj["web_server"] = QJsonObject{{"ip", ui->address_server->text()}, {"port", ui->port_server->text()}};

    QJsonDocument doc(cfgObj);
    QFile file("cfg.json");
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
    }

    // 登陆信息加密
    m_cm.writeLoginInfo(user, pwd, ui->rember_pwd->isChecked());
    // 设置登陆信息json包, 密码经过md5加密， getStrMd5()
    QByteArray array = setLoginJson(user, m_cm.getStrMd5(pwd));
    // 设置登录的url
    QNetworkRequest request;
    QString url = QString("http://%1:%2/login").arg(address).arg(port);
    request.setUrl(QUrl(url));
    // 请求头信息
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));
    request.setHeader(QNetworkRequest::ContentLengthHeader, QVariant(array.size()));
    // 向服务器发送post请求
    QNetworkReply* reply = m_manager->post(request, array);
    cout << "post url:" << url << "post data: " << array;

    // 接收服务器发回的http响应消息
    connect(reply, &QNetworkReply::finished, [=]()
    {
        // 出错了
        if (reply->error() != QNetworkReply::NoError)
        {
            cout << reply->errorString();
            
            reply->deleteLater();
            return;
        }

        // 将server回写的数据读出
        QByteArray json = reply->readAll();
        /*
            登陆 - 服务器回写的json数据包格式：
                用户：{"code":"000"}
                管理员：{"code":"001"}
                失败：{"code":"101"}
        */
        cout << "server return value: " << json;
        QStringList tmpList = getLoginStatus(json); //common.h
        if( tmpList.at(0) == "001" )
        {
            cout << "管理员登陆成功";

            // 设置登陆信息，显示文件列表界面需要使用这些信息
            LoginInfoInstance *p = LoginInfoInstance::getInstance();
            p->setLoginInfo(user, address, port, tmpList.at(1));
            cout << p->getUser().toUtf8().data() << ", " << p->getIp() << ", " << p->getPort() << tmpList.at(1);
            qDebug()<<"user:"<<p->getUser()<<" token:"<<p->getToken();
            // 当前窗口隐藏
            this->hide();
            // 主界面窗口显示
            m_mainWin = new MainWindow;
            m_mainWin->setWindowIcon(QIcon(":/images/logo.ico"));
            m_mainWin->show();
        }
        else if(tmpList.at(0) == "000")
        {
            cout << "用户登陆成功";

            // 设置登陆信息，显示文件列表界面需要使用这些信息
            LoginInfoInstance *p = LoginInfoInstance::getInstance();
            p->setLoginInfo(user, address, port, tmpList.at(1));
            cout << p->getUser().toUtf8().data() << ", " << p->getIp() << ", " << p->getPort() << tmpList.at(1);
            qDebug()<<"user:"<<p->getUser()<<" token:"<<p->getToken();
            // 当前窗口隐藏
            this->hide();
            m_userWin = new UserWindow(p->getUser());
            m_userWin->show();
        }
        else if(tmpList.at(0) == "101")
        {
            QMessageBox::warning(this, "登录失败", "用户名或密码不正确！！！");
        }

        reply->deleteLater(); 
    });
}

// 用户注册操作
void Login::on_register_btn_clicked()
{
    // 从控件中取出用户输入的数据
    QString userName = ui->reg_usr->text();
    QString nickName = ui->reg_nickname->text();
    QString firstPwd = ui->reg_pwd->text();
    QString surePwd = ui->reg_surepwd->text();
    QString phone = ui->reg_phone->text();
    QString email = ui->reg_mail->text();


    // 数据校验
//    QRegularExpression regexp(USER_REG);
//    if(!regexp.match(userName).hasMatch())
//    {
//        QMessageBox::warning(this, "警告", "用户名格式不正确");
//        ui->reg_usr->clear();
//        ui->reg_usr->setFocus();
//        return;
//    }
//    if(!regexp.match(nickName).hasMatch())
//    {
//        QMessageBox::warning(this, "警告", "昵称格式不正确");
//        ui->reg_nickname->clear();
//        ui->reg_nickname->setFocus();
//        return;
//    }
//    regexp.setPattern(PASSWD_REG);
//    if(!regexp.match(firstPwd).hasMatch())
//    {
//        QMessageBox::warning(this, "警告", "密码格式不正确");
//        ui->reg_pwd->clear();
//        ui->reg_pwd->setFocus();
//        return;
//    }
//    if(surePwd != firstPwd)
//    {
//        QMessageBox::warning(this, "警告", "两次输入的密码不匹配, 请重新输入");
//        ui->reg_surepwd->clear();
//        ui->reg_surepwd->setFocus();
//        return;
//    }
//    regexp.setPattern(PHONE_REG);
//    if(!regexp.match(phone).hasMatch())
//    {
//        QMessageBox::warning(this, "警告", "手机号码格式不正确");
//        ui->reg_phone->clear();
//        ui->reg_phone->setFocus();
//        return;
//    }
//    regexp.setPattern(EMAIL_REG);
//    if(!regexp.match(email).hasMatch())
//    {
//        QMessageBox::warning(this, "警告", "邮箱码格式不正确");
//        ui->reg_mail->clear();
//        ui->reg_mail->setFocus();
//        return;
//    }


    // 将注册信息打包为json格式
    QByteArray array = setRegisterJson(userName, nickName, m_cm.getStrMd5(firstPwd), phone, email);
    qDebug() << "register json data" << array;
    // 设置连接服务器要发送的url
    QNetworkRequest request;
    QString url = QString("http://%1:%2/reg").arg(ui->address_server->text()).arg(ui->port_server->text());
    request.setUrl(QUrl(url));
    // 设置请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));
    request.setHeader(QNetworkRequest::ContentLengthHeader, QVariant(array.size()));
    // 发送数据
    QNetworkReply* reply = m_manager->post(request, array);
    // 判断请求是否被成功处理
    connect(reply, &QNetworkReply::readyRead, [=]()
    {
        // 读sever回写的数据
        QByteArray jsonData = reply->readAll();
        /*
        注册 - server端返回的json格式数据：
            成功:         {"code":"002"}
            该用户已存在：  {"code":"003"}
            失败:         {"code":"004"}
        */
        // 判断状态码
        if("002" == m_cm.getCode(jsonData))
        {
            //注册成功
            QMessageBox::information(this, "注册成功", "注册成功，请登录");

            //清空行编辑内容
            ui->reg_usr->clear();
            ui->reg_nickname->clear();
            ui->reg_pwd->clear();
            ui->reg_surepwd->clear();
            ui->reg_phone->clear();
            ui->reg_mail->clear();

            //设置登陆窗口的登陆信息
            ui->login_usr->setText(userName);
            ui->login_pwd->setText(firstPwd);
            ui->rember_pwd->setChecked(true);

            //切换到登录界面
            ui->stackedWidget->setCurrentWidget(ui->login_page);
        }
        else if("003" == m_cm.getCode(jsonData))
        {
            QMessageBox::warning(this, "注册失败", QString("[%1]该用户已经存在!!!").arg(userName));
        }
        else if("004" == m_cm.getCode(jsonData))
        {
            QMessageBox::warning(this, "注册失败", "注册失败！！！");
        }
        // 释放资源
        delete reply;
    });

}

void Login::on_set_ok_btn_clicked()
{
    QString ip = ui->address_server->text();
    QString port = ui->port_server->text();

    // 数据判断
    // 服务器IP
    // \\d 和 \\. 中第一个\是转义字符, 这里使用的是标准正则
    QRegularExpression regexp(IP_REG);//IP_REG为正则表达式
    if(!regexp.match(ip).hasMatch())
    {
    QMessageBox::warning(this, "警告", "您输入的IP格式不正确, 请重新输入!");
    return;
    }
    // 端口号
    regexp.setPattern(PORT_REG);
    if(!regexp.match(port).hasMatch())
    {
        QMessageBox::warning(this, "警告", "您输入的端口格式不正确, 请重新输入!");
        return;
    }
    // 跳转到登陆界面
    ui->stackedWidget->setCurrentWidget(ui->login_page);
    // 将配置信息写入配置文件中
    m_cm.writeWebInfo(ip, port);
}

