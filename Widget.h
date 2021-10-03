#pragma once
#include <QWidget>
#include "showThread.h"

//声明一个空类Widget
namespace Ui {
class Widget;
}


class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
    void Init();

private slots:

    void ReceiveMajorImage(QImage image, int result);
    void on_pushButton1_clicked();
    void on_pushButton2_clicked();

private:
    Ui::Widget *ui;

    int err11, err19;

    showThread *imageprocessthread;
    
};

