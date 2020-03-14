#include "MainWindow.h"

#include <QApplication>
#include <QOperatingSystemVersion>

int main(int argc, char *argv[])
{
    auto mutexHandle = CreateMutexA(nullptr, false, "VideoWallpaper_Mutex");
    if (!mutexHandle || GetLastError() == ERROR_ALREADY_EXISTS)    // 使用互斥锁防止进程多开，该锁可跨进程获取
    {
        CloseHandle(mutexHandle);   // 其实不写也没事，后面else为了代码好看点就没写

        if (QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows8_1)
        {
            // https://docs.microsoft.com/en-us/windows/win32/api/shellscalingapi/nf-shellscalingapi-setprocessdpiawareness
            // 根据MSDN文档，SetProcessDpiAwareness在Win8.1之后才存在
            SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);  // 适配高DPI
        }
        else
        {
            // Win8.1之前的版本就使用这个函数作为替代高DPI适配，至于能否实现功能全凭运气
            SetProcessDPIAware();
        }
        MessageBox(nullptr, L"已有另一VideoWallpaper实例正在运行", L"提示", MB_ICONINFORMATION | MB_OK);

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
