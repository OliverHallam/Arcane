#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <fstream>
#include <iostream>

#include "resource.h"

#include "D3DRenderer.h"
#include "../NesCoreCpp/NesSystem.h"

D3DRenderer renderer{ Display::WIDTH, Display::HEIGHT };

LRESULT CALLBACK WindowProc(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam);

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow)
{
    auto icon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON));

    auto className = L"NES";

    WNDCLASS wndClass;
    wndClass.style = 0;
    wndClass.lpfnWndProc = WindowProc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = hInstance;
    wndClass.hIcon = icon;
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndClass.lpszMenuName = NULL;
    wndClass.lpszClassName = className;

    if (!RegisterClass(&wndClass))
        return -1;

    const int defaultWidth = 1024;
    const int defaultHeight = 960;

    RECT rc;
    SetRect(&rc, 0, 0, defaultWidth, defaultHeight);

    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);

    auto wnd = CreateWindow(
        className,
        L"NES",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rc.right - rc.left,
        rc.bottom - rc.top,
        NULL,
        NULL,
        hInstance,
        0);

    if (wnd == NULL)
        return -1;

    ShowWindow(wnd, nCmdShow);
    UpdateWindow(wnd);

    if (!renderer.Initialize(wnd))
        return -1;

    renderer.PrepareRenderState();

    auto path = R"(c:\roms\NESRoms\World\Super Mario Bros (JU) (PRG 0).nes)";
    auto Frames = 10000;

    std::ifstream file(path, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size))
    {
        return -1;
    }

    auto system = std::make_unique<NesSystem>();

    auto cart = TryLoadCart(reinterpret_cast<uint8_t*>(&buffer[0]), buffer.size());

    system->InsertCart(std::move(cart));
    system->Reset();

    while (true)
    {
        MSG msg;
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) != 0)
        {
            if (msg.message == WM_QUIT)
                return 0;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            system->RunFrame();

            renderer.RenderFrame(system->Display().Buffer());
        }
    }
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

