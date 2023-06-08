#ifndef TITLEWIDGET_H
#define TITLEWIDGET_H

#include <QWidget>

namespace Ui {
class TitleWidget;
}

class TitleWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TitleWidget(QWidget *parent = nullptr);
    ~TitleWidget();

    void setParent(QWidget *parent);

protected:
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);

signals:
    void showSetWidget();
    void closeWindow();

private:
    Ui::TitleWidget *ui;

    // 保存鼠标按下时的坐标
    QPoint m_pos;

    // 父窗口指针
    QWidget *m_parent;
};

#endif // TITLEWIDGET_H
