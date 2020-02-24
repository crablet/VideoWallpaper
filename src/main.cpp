#include "MainWindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    auto mutexHandle = CreateMutexA(nullptr, false, "VideoWallpaper_Mutex");
    if (!mutexHandle || GetLastError() == ERROR_ALREADY_EXISTS)    // 使用互斥锁防止进程多开，该锁可跨进程获取
    {
        CloseHandle(mutexHandle);   // 其实不写也没事，后面else为了代码好看点就没写

        return 0;
    }
    else    // 能够成功获取锁，证明是唯一运行的实体，则允许继续执行
    {
        QApplication a(argc, argv);
        MainWindow w;
        w.show();

        return a.exec();    // 无需手动CloseHandle()，因为操作系统会自动回收mutexHandle
    }
}
