#include "pch.h"

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

#include <shellapi.h>

#include "resource.h"

#include "D3DRenderer.h"
#include "WasapiRenderer.h"
#include "../NesCoreCpp/NesSystem.h"

LRESULT CALLBACK WindowProc(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam);

std::unique_ptr<NesSystem> System;

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow)
{
    int argCount;
    auto args = CommandLineToArgvW(GetCommandLine(), &argCount);

    if (argCount == 1)
    {
        MessageBox(NULL, L"Please specify the ROM to load on the command line", L"No ROM Specified", MB_ICONINFORMATION | MB_OK);
        return 0;
    }
    else if (argCount != 2)
    {
        MessageBox(NULL, L"The command line parameters were invalid", L"Invalid command line parameters", MB_ICONERROR | MB_OK);
        return 0;
    }

    std::wstring romPath{ args[1] };

    LocalFree(args);

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

    try
    {
        D3DRenderer d3d{ Display::WIDTH, Display::HEIGHT };

        d3d.Initialize(wnd);
        d3d.PrepareRenderState();

        WasapiRenderer wasapi{};
        if (!wasapi.Initialize())
        {
            // failed to negotiate a sample rate.
            return -1;
        }

        std::ifstream file(romPath, std::ios::binary | std::ios::ate);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<char> buffer(size);
        if (!file.read(buffer.data(), size))
        {
            return -1;
        }

        System = std::make_unique<NesSystem>(wasapi.SampleRate());

        auto cart = TryLoadCart(reinterpret_cast<uint8_t*>(&buffer[0]), buffer.size());

        System->InsertCart(std::move(cart));
        System->Reset();

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
                System->RunFrame();

                d3d.RenderFrame(System->Display().Buffer());
                wasapi.WriteSamples(System->Apu().Samples(), System->Apu().SamplesPerFrame());
            }
        }
    }
    catch (const winrt::hresult_error& e)
    {
        std::wstringstream ss;
        ss << L"HRESULT: 0x" << std::hex << e.code() << L"\n" << e.message().c_str();

        MessageBox(wnd, ss.str().c_str(), L"Unexpected error", MB_ICONERROR | MB_OK);

        return -1;
    }
}

bool ProcessKey(WPARAM key, bool down)
{
    switch (key)
    {
    case VK_UP:
        System->Controller().Up(down);
        return true;

    case VK_DOWN:
        System->Controller().Down(down);
        return true;

    case VK_LEFT:
        System->Controller().Left(down);
        return true;

    case VK_RIGHT:
        System->Controller().Right(down);
        return true;

    case 'Z':
        System->Controller().B(down);
        return true;

    case 'X':
        System->Controller().A(down);
        return true;

    case VK_RETURN:
        System->Controller().Start(down);
        return true;

    case VK_SHIFT:
        System->Controller().Select(down);
        return true;
    }

    return false;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_KEYDOWN:
        if (ProcessKey(wParam, true))
            return 0;
        break;

    case WM_KEYUP:
        if (ProcessKey(wParam, false))
            return 0;
        break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

