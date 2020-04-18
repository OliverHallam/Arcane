#include "pch.h"

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

#include "resource.h"

#include "D3DRenderer.h"
#include "WasapiRenderer.h"
#include "../NesCoreCpp/NesSystem.h"

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

        //auto path = R"(c:\roms\NESRoms\World\Super Mario Bros (JU) (PRG 0).nes)";
        auto path = R"(c:\roms\NESRoms\World\Donkey Kong (JU).nes)";
        auto Frames = 10000;

        std::ifstream file(path, std::ios::binary | std::ios::ate);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<char> buffer(size);
        if (!file.read(buffer.data(), size))
        {
            return -1;
        }

        auto system = std::make_unique<NesSystem>(wasapi.SampleRate());

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

                d3d.RenderFrame(system->Display().Buffer());
                wasapi.WriteSamples(system->Apu().Samples(), system->Apu().SamplesPerFrame());
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

