#include "pch.h"
#include "resource.h"

#include "About.h"

#include "Error.h"

#include "Version.h"

#include <CommCtrl.h>

About::About()
    : window_{}
{
}

void About::Show(HWND parent)
{
    if (!window_)
    {
        auto hInstance = GetModuleHandle(NULL);
        auto icon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON));

        auto className = L"About";

        WNDCLASS wndClass;
        ZeroMemory(&wndClass, sizeof(WNDCLASS));
        wndClass.style = 0;
        wndClass.lpfnWndProc = WindowProc;
        wndClass.cbClsExtra = 0;
        wndClass.cbWndExtra = 0;
        wndClass.hInstance = hInstance;
        wndClass.hIcon = icon;
        wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
        wndClass.hbrBackground = WHITE_BRUSH;
        wndClass.lpszMenuName = NULL;
        wndClass.lpszClassName = className;

        if (!RegisterClass(&wndClass))
            throw Error(L"Failed to register window class");

        auto width = 480;
        auto height = 300;

        auto dpi = GetDpiForWindow(parent);

        RECT rc;
        SetRect(&rc, 0, 0, width, height);
        ScaleRectForDpi(&rc, dpi);
        AdjustWindowRectExForDpi(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_EX_DLGMODALFRAME, FALSE, NULL, dpi);

        window_ = CreateWindowEx(
            WS_EX_DLGMODALFRAME | WS_EX_APPWINDOW,
            className,
            L"About Arcane",
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            rc.right - rc.left,
            rc.bottom - rc.top,
            parent,
            NULL,
            hInstance,
            NULL);

        if (window_ == NULL)
            throw Error(L"Failed to create window");
    }

    ShowWindow(window_, SW_SHOW);
    SetForegroundWindow(window_);
    UpdateWindow(window_);
}

LRESULT About::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CLOSE:
    {
        ShowWindow(hWnd, SW_HIDE);
        return 0;
    }

    case WM_CREATE:
    {
        auto defaultFont = GetStockObject(DEFAULT_GUI_FONT);

        auto instance = GetModuleHandle(NULL);
        auto dpi = GetDpiForWindow(hWnd);

        RECT rc;
        SetRect(&rc, 200, 240, 280, 265);
        ScaleRectForDpi(&rc, dpi);

        auto button = CreateWindow(
            L"BUTTON",
            L"OK",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            rc.left,
            rc.top,
            rc.right - rc.left,
            rc.bottom - rc.top,
            hWnd,
            reinterpret_cast<HMENU>(IDOK),
            instance,
            NULL);

        LOGFONT logFont;
        GetObject(defaultFont, sizeof(LOGFONT), &logFont);

        logFont.lfHeight = MulDiv(15, dpi, 96);
        logFont.lfWeight = FW_BOLD;

        auto largeFont = CreateFontIndirect(&logFont);

        SendMessage(button, WM_SETFONT, reinterpret_cast<WPARAM>(largeFont), TRUE);


        return 0;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT rc;

        SetRect(&rc, 0, 20, 480, 60);
        DrawTextBlock(hWnd, hdc, rc, true, L"Arcane");

        SetRect(&rc, 0, 80, 480, 95);
        DrawTextBlock(hWnd, hdc, rc, false, L"A NES emulator for Windows");

        SetRect(&rc, 0, 110, 480, 125);
        DrawTextBlock(hWnd, hdc, rc, false, L"Version " VERSION);

        SetRect(&rc, 0, 140, 480, 155);
        DrawTextBlock(hWnd, hdc, rc, false, L"Copyright © 2021 Oliver Hallam");

        SetRect(&rc, 0, 190, 480, 205);
        DrawTextBlock(hWnd, hdc, rc, false, L"https://github.com/OliverHallam/Arcane");

        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_COMMAND:
        if (wParam == IDOK)
        {
            ShowWindow(hWnd, SW_HIDE);
            return 0;
        }

    case WM_KEYDOWN:
        if (wParam == VK_RETURN)
        {
            ShowWindow(hWnd, SW_HIDE);
            return 0;
        }
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void About::ScaleRectForDpi(RECT* rc, UINT dpi)
{
    rc->left = MulDiv(rc->left, dpi, 96);
    rc->top = MulDiv(rc->top, dpi, 96);
    rc->right = MulDiv(rc->right, dpi, 96);
    rc->bottom = MulDiv(rc->bottom, dpi, 96);
}

void About::DrawTextBlock(HWND hwnd, HDC hdc, RECT rc, bool bold, const wchar_t* text)
{
    auto dpi = GetDpiForWindow(hwnd);

    ScaleRectForDpi(&rc, dpi);

    auto defaultFont = GetStockObject(DEFAULT_GUI_FONT);

    LOGFONT logFont;
    GetObject(defaultFont, sizeof(LOGFONT), &logFont);

    logFont.lfWeight = bold ? FW_BOLD : FW_NORMAL;
    logFont.lfHeight = rc.bottom - rc.top;

    auto largeFont = CreateFontIndirect(&logFont);

    auto prevFont = static_cast<HFONT>(SelectObject(hdc, largeFont));
    DrawText(hdc, text, -1, &rc, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
    SelectObject(hdc, prevFont);

    DeleteObject(largeFont);
}
