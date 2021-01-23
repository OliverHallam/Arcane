#pragma once

class About
{
public:
    About();

    void Show(HWND parent);

private:
    static LRESULT CALLBACK WindowProc(
        HWND hWnd,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam);

    static void ScaleRectForDpi(RECT* rc, UINT dpi);

    static void DrawTextBlock(HWND hwnd, HDC hdc, RECT rc, bool bold, const wchar_t* text);

    HWND window_;
};