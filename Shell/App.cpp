#include "pch.h"

#include "commdlg.h"
#include "resource.h"

#include "App.h"

#include "CommandLine.h"
#include "D3DRenderer.h"
#include "Menu.h"
#include "SaveFile.h"
#include "WasapiRenderer.h"

#include "../NesCoreCpp/GameDatabase.h"
#include "../NesCoreCpp/RomFile.h"

#include <sstream>

App::App(HINSTANCE hInstance)
    : instance_{ hInstance },
    window_{ }
{
}

int App::Run(int nCmdShow)
{
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
            ReportError(L"Invalid command line parameters", e);
            return -1;
        }

        Menu menu;

        try
        {
            window_ = InitializeWindow(instance_, menu.Get(), nCmdShow);
        }
        catch (Error& e)
        {
            ReportError(L"Error initializing window", e);
            return -1;
        }

        try
        {
            d3d.Initialize(window_, Display::WIDTH, Display::HEIGHT);
            d3d.PrepareRenderState();
            d3d.RenderClear();
        }
        catch (const winrt::hresult_error& e)
        {
            ReportError(L"Error initializing renderer", e);
            return -1;
        }

        WasapiRenderer wasapi{};
        try
        {
            wasapi.Initialize();
        }
        catch (const Error& e)
        {
            ReportError(L"Error initializing audio", e);
            return -1;
        }
        catch (const winrt::hresult_error& e)
        {
            ReportError(L"Error initializing audio", e);
            return -1;
        }

        host_.SetSampleRate(wasapi.SampleRate());

        if (romPath.size())
        {
            auto cart = LoadGame(window_, romPath);
            if (!cart)
                return -1;

            host_.Load(std::move(cart));
            host_.Start();
        }
        else
        {
            Open(window_);
        }

        bool running = host_.Running();
        while (true)
        {
            MSG msg;
            if (running)
            {
                if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) != 0)
                {
                    if (msg.message == WM_QUIT)
                        return 0;

                    if (!TranslateAccelerator(window_, menu.AcceleratorTable(), &msg))
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                }
                else
                {
                    if (host_.Loaded())
                    {
                        host_.RunFrame();

                        d3d.RenderFrame(host_.PixelData());
                        wasapi.WriteSamples(host_.AudioSamples(), host_.SamplesPerFrame());
                    }
                    else
                    {
                        d3d.RenderClear();
                    }

                    running = host_.Running();
                }
            }
            else
            {
                if (!GetMessage(&msg, NULL, 0U, 0U))
                    return 0;

                if (!TranslateAccelerator(window_, menu.AcceleratorTable(), &msg))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }

                running = host_.Running();
            }
        }
    }
    catch (const Error& e)
    {
        ReportError(L"Unexpected error", e);
        return -1;
    }
    catch (const winrt::hresult_error& e)
    {
        ReportError(L"Unexpected error", e);
        return -1;
    }
}

void App::ReportError(const wchar_t* title, const Error& error)
{
    MessageBox(window_, error.Message().c_str(), title, MB_ICONERROR | MB_OK);
}

void App::ReportError(const wchar_t* title, const winrt::hresult_error& error)
{
    std::wstringstream ss;
    ss << L"HRESULT: 0x" << std::hex << error.code() << L"\n" << error.message().c_str();
    MessageBox(window_, ss.str().c_str(), title, MB_ICONERROR | MB_OK);
}

HWND App::InitializeWindow(HINSTANCE hInstance, HMENU menu, int nCmdShow)
{
    auto icon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON));

    auto className = L"NES";

    WNDCLASS wndClass;
    wndClass.style = 0;
    wndClass.lpfnWndProc = WindowProcStub;
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
        menu,
        hInstance,
        this);

    if (wnd == NULL)
        throw Error(L"Failed to create window");

    ShowWindow(wnd, nCmdShow);
    UpdateWindow(wnd);

    return wnd;
}

std::unique_ptr<Cart> App::LoadGame(HWND wnd, const std::wstring& romPath)
{
    std::unique_ptr<RomFile> romFile;
    try
    {
        romFile = LoadCart(romPath);
    }
    catch (const Error& e)
    {
        ReportError(L"Error loading cartridge", e);
        return nullptr;
    }
    catch (const winrt::hresult_error& e)
    {
        ReportError(L"Error loading cartridge", e);
        return nullptr;
    }

    auto batteryBackedMemorySize = romFile->Descriptor.PrgBatteryRamSize;
    if (batteryBackedMemorySize)
    {
        try
        {
            auto dotPos = romPath.find_last_of('.');
            auto withoutExtension = romPath.substr(0, dotPos);
            auto savPath = withoutExtension + L".sav";

            save_.Create(savPath);
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
        save_.Data());

    if (!cart)
    {
        MessageBox(wnd, L"The selected game is not supported", L"Error loading game", MB_ICONERROR | MB_OK);
        return nullptr;
    }

    return cart;
}

std::unique_ptr<RomFile> App::LoadCart(const std::wstring& romPath)
{
    winrt::file_handle hFile{
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

    winrt::handle mapping{
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

void App::Open(HWND window)
{
    WCHAR fileName[MAX_PATH];
    ZeroMemory(fileName, sizeof(fileName));

    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = window;
    ofn.hInstance = NULL;
    ofn.lpstrFilter = L"iNES ROM File\0*.nes\0";
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 0;
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = L"Open ROM";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = L"nes";
    ofn.lCustData = NULL;
    ofn.lpfnHook = NULL;
    ofn.lpTemplateName = NULL;
    ofn.pvReserved = NULL;
    ofn.dwReserved = NULL;
    ofn.FlagsEx = 0;

    if (!GetOpenFileName(&ofn))
        return;

    host_.Stop();
    host_.Unload();
    save_.Close();

    std::wstring romPath(ofn.lpstrFile);
    auto cart = LoadGame(window, romPath);

    if (cart)
    {
        host_.Load(std::move(cart));
        host_.Start();
    }
}

LRESULT CALLBACK App::WindowProcStub(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    auto app = reinterpret_cast<App*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    if (app)
        return app->WindowProc(uMsg, wParam, lParam);

    if (uMsg == WM_NCCREATE)
    {
        auto createStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
        app = reinterpret_cast<App*>(createStruct->lpCreateParams);
        app->window_ = hWnd;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT App::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
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

    case WM_COMMAND:
        if (ProcessCommand(LOWORD(wParam)))
            return 0;
        break;

    case WM_SIZE:
        d3d.OnSize();
    }

    return DefWindowProc(window_, uMsg, wParam, lParam);
}

bool App::ProcessKey(WPARAM key, bool down)
{
    switch (key)
    {
    case VK_UP:
        host_.Up(down);
        return true;

    case VK_DOWN:
        host_.Down(down);
        return true;

    case VK_LEFT:
        host_.Left(down);
        return true;

    case VK_RIGHT:
        host_.Right(down);
        return true;

    case 'Z':
        host_.B(down);
        return true;

    case 'X':
        host_.A(down);
        return true;

    case VK_RETURN:
        host_.Start(down);
        return true;

    case VK_SHIFT:
        host_.Select(down);
        return true;

#if DIAGNOSTIC
    case VK_ESCAPE:
        if (down)
        {
            if (NesHost.Running())
                NesHost.Stop();
            else
                NesHost.Start();
        }
        return true;

    case VK_SPACE:
        if (down)
            NesHost.Step();
        return true;
#endif
    }

    return false;
}

bool App::ProcessCommand(WORD command)
{
    if (command == static_cast<WORD>(MenuCommand::Open))
    {
        Open(window_);
        return true;
    }

    if (command == static_cast<WORD>(MenuCommand::Snapshot))
    {
        host_.Snapshot();
        return true;
    }

    if (command == static_cast<WORD>(MenuCommand::Restore))
    {
        host_.Restore();
        return true;
    }

    return false;
}