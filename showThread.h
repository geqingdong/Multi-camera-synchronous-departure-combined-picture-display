#pragma once
#include <string>
#include <iostream>
#include <QThread>
#include <QImage>
#include <QDebug>

class showThread:public QThread
{
    Q_OBJECT
public:
    showThread();
    ~showThread();

    QImage majorImage;
    void stop();
    void init(int index);

protected:
    void run();

private:
    volatile int majorindex;
    volatile bool stopped;

signals:
    void SendMajorImageProcessing(QImage image, int result);
};

