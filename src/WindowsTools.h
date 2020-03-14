#ifndef WINDOWSTOOLS_H
#define WINDOWSTOOLS_H

#pragma execution_character_set("utf-8")

#include <Windows.h>
#include <ShellScalingApi.h>

#pragma comment(lib, "Shcore.lib")

// 获取桌面句柄，用于在桌面上播放视频
inline HWND GetDesktopHwnd() noexcept
{
    auto hWnd = FindWindow(L"Progman", L"Program Manager");
    SendMessageTimeout(hWnd, 0x52C, 0, 0, SMTO_NORMAL, 1000, nullptr);	// 不知道是否可以为空指针

    HWND hWndWorkW = nullptr;
    do
    {
        hWndWorkW = FindWindowEx(nullptr, hWndWorkW, L"WorkerW", nullptr);
        if (hWndWorkW)
        {
            if (FindWindowEx(hWndWorkW, nullptr, L"SHELLDLL_DefView", nullptr))
            {
                auto h = FindWindowEx(nullptr, hWndWorkW, L"WorkerW", nullptr);
                while (h)
                {
                    SendMessage(h, WM_CLOSE, 0, 0);
                    h = FindWindowEx(nullptr, hWndWorkW, L"WorkerW", nullptr);
                }

                break;
            }
        }
    } while (true);

    return hWnd;
}

#endif // WINDOWSTOOLS_H
