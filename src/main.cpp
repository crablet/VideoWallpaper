#include "MainWindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    auto mutexHandle = CreateMutexA(nullptr, false, "VideoWallpaper_Mutex");
    if (!mutexHandle || GetLastError() == ERROR_ALREADY_EXISTS)    // 防止进程多开
    {
        CloseHandle(mutexHandle);   // 其实不写也没事，后面else为了代码好看点就没写

        return 0;
    }
    else
    {
        QApplication a(argc, argv);
        MainWindow w;
        w.show();

        return a.exec();    // 无需手动CloseHandle()，因为操作系统会自动回收mutexHandle
    }
}
