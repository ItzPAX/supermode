#pragma once
#include "inc.h"

namespace overlay
{
    HDC dc;
    std::vector<comm::box> boxes;

    LRESULT CALLBACK wndproc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_PAINT:
            PAINTSTRUCT ps;
            dc = BeginPaint(hWnd, &ps);
            render::dc = dc;
            render::hwnd = hWnd;

            RECT rc;
            GetClientRect(hWnd, &rc);

            HBRUSH hBrush = CreateSolidBrush(TRANSPARENT_COLOR);
            FillRect(dc, &rc, hBrush);
            DeleteObject(hBrush);

            menu::render_info(dc);
            menu::render_features();

            if (menu::esp)
            {
                for (auto& box : boxes)
                {
                    render::rectangle(box.xy.x, box.xy.y, box.size.x, box.size.y, box.col, true);
                }
            }

            EndPaint(hWnd, &ps);
            return 0;
        }

        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    
    void update_lines()
    {
        while (true)
        {
            boxes = comm::read_draw_data();
            Sleep(1);
        }
    }

    int main_thread(HWND hwGame)
    {
        WNDCLASSEX wc;
        memset(&wc, 0, sizeof(wc));
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = (WNDPROC)wndproc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.lpszClassName = "XPerfStatistics";

        RegisterClassEx(&wc);

        RECT r;
        GetWindowRect(hwGame, &r);
        globals::overlay_w = r.right; globals::overlay_h = r.bottom;

        HWND hwOverlay = CreateWindowEx(WS_EX_LAYERED | WS_EX_TRANSPARENT, "XPerfStatistics", "XPerfStatistics", WS_POPUP | WS_VISIBLE, 0, 0, globals::overlay_w, globals::overlay_h, NULL, NULL, GetModuleHandle(NULL), NULL);
        if (!hwOverlay)
            return -2;

        SetLayeredWindowAttributes(hwOverlay, TRANSPARENT_COLOR, 0, LWA_COLORKEY);
        ShowWindow(hwOverlay, SW_SHOW);

        dc = GetDC(hwOverlay);

        HANDLE h = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)update_lines, NULL, NULL, NULL);
        if (h)
            CloseHandle(h);

        menu::init();

        MSG msg = { 0 };
        while (msg.message != WM_QUIT)
        {
            RECT r;
            GetWindowRect(hwGame, &r);
            globals::overlay_w = r.right; globals::overlay_h = r.bottom;
            SetWindowPos(hwOverlay, nullptr, r.left, r.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
            SetWindowPos(hwOverlay, nullptr, 0, 0, r.right - r.left, r.bottom - r.top, SWP_NOMOVE | SWP_NOZORDER);

            if (hwGame == GetForegroundWindow())
            {
                ShowWindow(hwOverlay, SW_RESTORE);
                SetWindowPos(hwOverlay, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
                SetWindowPos(hwOverlay, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

                // NON TOPMOST MAGIC.
                SetWindowPos(hwGame, hwOverlay, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
                InvalidateRect(hwOverlay, &r, true);
                UpdateWindow(hwOverlay);
            }
            else
            {
                ShowWindow(hwOverlay, SW_HIDE);
            }
        }

        ReleaseDC(hwOverlay, dc);
        UnregisterClassA(wc.lpszClassName, wc.hInstance);

        return (0);
    }
}