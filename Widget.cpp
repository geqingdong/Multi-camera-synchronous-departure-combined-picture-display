#include <QDebug>
#include <QPushButton>
#include "showThread.h"
#include "moduleIo.h"
#include "Widget.h"
#include "ui_Widget.h"

//引用g_Ctx
extern TContax g_Ctx;

/*
    *brief:         显示线程初始化
    *description:   初始化显示线程，通过connect函数把收到的图片传给ui显示
    *rturn:         应用程序
*/
void Widget::Init()
{
    err11 = err19 = 0;

    /*new一个显示线程，做初始化和启动*/
    imageprocessthread = new showThread();

    /*图片线程初始化*/
    imageprocessthread->init();
    imageprocessthread->start();

    /*connect函数*/
    connect(imageprocessthread, SIGNAL(SendMajorImageProcessing(QImage, int)),
            this, SLOT(ReceiveMajorImage(QImage, int)));
}
/*
    brief:建立ui界面函数
    description:建立QT ui界面
    rturn:应用程序
*/
Widget::Widget(QWidget *parent) : QWidget(parent),
                                  ui(new Ui::Widget)
{

    ui->setupUi(this);
}

/*
    *brief:          ui界面销毁函数
    *description:QT  ui界面退出
*/
Widget::~Widget()
{

    delete ui;
}

/*
    *brief:          接收图片函数
    *description:QT  ui界面退出
    *param image     传进来的图片
    *param result    显示图片
    *return:         result
*/
void Widget::ReceiveMajorImage(QImage image, int result)
{

    if (!image.isNull())
    {
        ui->mainlabel->clear();
        switch (result)
        {
        case 0: //Success
            err11 = err19 = 0;
            if (image.isNull())
                ui->mainlabel->setText("no frame");
            else
                ui->mainlabel->setPixmap(QPixmap::fromImage(image.scaled(ui->mainlabel->size())));

            break;
        case 11:
            err11++;
            if (err11 == 10)
            {
                ui->mainlabel->clear();
                ui->mainlabel->setText("open device, no frame");
            }
            break;
        case 19: //No such device
            err19++;
            if (err19 == 10)
            {
                ui->mainlabel->clear();
                ui->mainlabel->setText("no device");
            }
            break;
        }
    }
}

/*
    brief:开始按钮功能函数
    description:点击按钮，触发打开，开始显示
*/
void Widget::on_pushButton1_clicked()
{
    g_Ctx.pSyncu->Start();
}

/*
    brief:关闭按钮功能函数
    description:点击OFF按钮，触发关闭，结束显示
*/
void Widget::on_pushButton2_clicked()
{
    g_Ctx.pSyncu->Stop();
}
