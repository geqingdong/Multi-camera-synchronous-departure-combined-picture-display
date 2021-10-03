#include "Widget.h"
#include <QApplication>
#include <QPushButton>

/*
    function:main函数
    Description:初始化窗口系统，使用在argv中的argc个命令行参数构造一个应用程序对象
    Return:应用程序
*/
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Widget w;
    w.Init();
    w.show();

    return app.exec();
}
