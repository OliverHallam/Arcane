#include "pch.h"

#include "resource.h"

#include "Main.h"

#include "CommandLine.h"
#include "D3DRenderer.h"
#include "Error.h"
#include "SaveFile.h"
#include "WasapiRenderer.h"

#include "../NesCoreCpp/GameDatabase.h"
#include "../NesCoreCpp/RomFile.h"

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

Host NesHost;

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow)
{
    HWND wnd = NULL;
    try
    {
        std::wstring romPath;
        try
        {
            CommandLine commandLine{};
            commandLine.Parse();
            romPath = commandLine.RomPath();
        }
        catch (const Error& e)
        {
            ReportError(wnd, L"Invalid command line parameters", e);
            return -1;
        }

        try
        {
            wnd = InitializeWindow(hInstance, nCmdShow);
        }
        catch (Error& e)
        {
            ReportError(wnd, L"Error initializing window", e);
            return -1;
        }

        D3DRenderer d3d{ Display::WIDTH, Display::HEIGHT };
        try
        {
            d3d.Initialize(wnd);
            d3d.PrepareRenderState();
        }
        catch (const winrt::hresult_error& e)
        {
            ReportError(wnd, L"Error initializing renderer", e);
            return -1;
        }

        WasapiRenderer wasapi{};
        try
        {
            wasapi.Initialize();
        }
        catch (const Error& e)
        {
            ReportError(wnd, L"Error initializing audio", e);
            return -1;
        }
        catch (const winrt::hresult_error& e)
        {
            ReportError(wnd, L"Error initializing audio", e);
            return -1;
        }

        std::unique_ptr<RomFile> romFile;
        try
        {
            romFile = LoadCart(romPath);
        }
        catch (const Error& e)
        {
            ReportError(wnd, L"Error loading cartridge", e);
            return -1;
        }
        catch (const winrt::hresult_error& e)
        {
            ReportError(wnd, L"Error loading cartridge", e);
            return -1;
        }

        SaveFile saveFile;
        auto batteryBackedMemorySize = romFile->Descriptor.PrgBatteryRamSize;
        if (batteryBackedMemorySize)
        {
            try
            {
                auto dotPos = romPath.find_last_of('.');
                auto withoutExtension = romPath.substr(0, dotPos);
                auto savPath = withoutExtension + L".sav";

                saveFile.Create(savPath);
            }
            catch (const Error& e)
            {
                auto message = std::wstring(L"There was a problem loading the save file.  The game will run without saving.\n" + e.Message());
                MessageBox(wnd, e.Message().c_str(), L"Error loading save file", MB_ICONERROR | MB_OK);
            }
            catch (const winrt::hresult_error& e)
            {
                std::wstringstream ss;
                ss << L"There was a problem loading the save file.  The game will run without saving.\n"
                    << L"HRESULT: 0x" << std::hex << e.code() << L"\n" << e.message().c_str();

                MessageBox(wnd, ss.str().c_str(), L"Error loading save file", MB_ICONERROR | MB_OK);
            }
        }

        auto cart = TryCreateCart(
            romFile->Descriptor,
            std::move(romFile->PrgData),
            std::move(romFile->ChrData),
            saveFile.Data());

        if (!cart)
        {
            MessageBox(wnd, L"The selected game is not supported", L"Error loading game", MB_ICONERROR | MB_OK);
            return -1;
        }

        NesHost.SetSampleRate(wasapi.SampleRate());
        NesHost.Load(std::move(cart));
        NesHost.Start();

        while (true)
        {
            MSG msg;
            if (NesHost.Running())
            {
                if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) != 0)
                {
                    if (msg.message == WM_QUIT)
                        return 0;

                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
                else
                {
                    NesHost.RunFrame();

                    d3d.RenderFrame(NesHost.PixelData());
                    wasapi.WriteSamples(NesHost.AudioSamples(), NesHost.SamplesPerFrame());
                }
            }
            else
            {
                if (!GetMessage(&msg, NULL, 0U, 0U))
                    return 0;

                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }
    catch (const Error& e)
    {
        ReportError(wnd, L"Unexpected error", e);
        return -1;
    }
    catch (const winrt::hresult_error& e)
    {
        ReportError(wnd, L"Unexpected error", e);
        return -1;
    }
}

void ReportError(HWND window, const wchar_t* title, const Error& error)
{
    MessageBox(window, error.Message().c_str(), title, MB_ICONERROR | MB_OK);
}

void ReportError(HWND window, const wchar_t* title, const winrt::hresult_error& error)
{
    std::wstringstream ss;
    ss << L"HRESULT: 0x" << std::hex << error.code() << L"\n" << error.message().c_str();
    MessageBox(window, ss.str().c_str(), title, MB_ICONERROR | MB_OK);
}

bool ProcessKey(WPARAM key, bool down)
{
    switch (key)
    {
    case VK_UP:
        NesHost.Up(down);
        return true;

    case VK_DOWN:
        NesHost.Down(down);
        return true;

    case VK_LEFT:
        NesHost.Left(down);
        return true;

    case VK_RIGHT:
        NesHost.Right(down);
        return true;

    case 'Z':
        NesHost.B(down);
        return true;

    case 'X':
        NesHost.A(down);
        return true;

    case VK_RETURN:
        NesHost.Start(down);
        return true;

    case VK_SHIFT:
        NesHost.Select(down);
        return true;

    case VK_ESCAPE:
        if (down)
        {
            if (NesHost.Running())
                NesHost.Stop();
            else
                NesHost.Start();
        }
        return true;

#if DIAGNOSTIC
    case VK_SPACE:
        if (down)
            NesHost.Step();
        return true;
#endif
    }

    return false;
}

HWND InitializeWindow(HINSTANCE hInstance, int nCmdShow)
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
        throw Error(L"Failed to register window class");

    const int defaultWidth = Display::WIDTH * 4;
    const int defaultHeight = Display::HEIGHT * 4;

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
        throw Error(L"Failed to create window");

    ShowWindow(wnd, nCmdShow);
    UpdateWindow(wnd);

    return wnd;
}

std::unique_ptr<RomFile> LoadCart(const std::wstring& romPath)
{
    winrt::file_handle hFile {
        CreateFile(
            romPath.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL) };
    winrt::check_bool(bool{ hFile });

    LARGE_INTEGER fileSize;
    if (GetFileSizeEx(hFile.get(), &fileSize) == FALSE)
        winrt::throw_last_error();

    // maximum size of 4mb for security reassons
    const uint32_t maxFileSize = 4 * 1024 * 1024;
    if (fileSize.QuadPart > maxFileSize)
        throw Error(L"ROM File is suspiciously large!");

    winrt::handle mapping {
        CreateFileMapping(
            hFile.get(),
            NULL,
            PAGE_READONLY,
            0,
            0,
            NULL) };
    winrt::check_bool(bool{ mapping });

    auto data = MapViewOfFile(mapping.get(), FILE_MAP_READ, 0, 0, 0);
    if (data == NULL)
        winrt::throw_last_error();

    auto romFile = TryLoadINesFile(reinterpret_cast<uint8_t*>(data), fileSize.LowPart);
    UnmapViewOfFile(data);
    if (!romFile)
        throw Error(L"Unsupported or invalid ROM file: " + romPath);

    auto goodDescriptor = GameDatabase::Lookup(romFile->PrgData, romFile->ChrData);

    if (goodDescriptor)
        romFile->Descriptor = *goodDescriptor.release();

    return std::move(romFile);
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

