#include "pch.h"

#include "commdlg.h"
#include "resource.h"

#include "App.h"

#include "CommandLine.h"
#include "D3DRenderer.h"
#include "Menu.h"
#include "SaveFile.h"
#include "WasapiRenderer.h"
#include "QpcTimer.h"

#include "../NesCore/GameDatabase.h"
#include "../NesCore/RomFile.h"

#include <shellapi.h>
#include <dwmapi.h>

#include <cstdlib>
#include <sstream>

#include <Dbt.h>

App::App(HINSTANCE hInstance)
    : instance_{ hInstance },
    window_{ },
    windowRect_{ },
    fullscreen_{ false },
    initialized_{ false },
    foreground_{ false },
    overscan_{ false },
    integerScaling_{ true },
    scanlines_{ false },
    player1Device_{ 0 },
    player2Device_{ 0 }
{
}

int App::Run(int nCmdShow)
{
    // boost our priority to get silky smooth framerates
    auto hProcess = GetCurrentProcess();
    SetPriorityClass(hProcess, ABOVE_NORMAL_PRIORITY_CLASS);

    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);

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

        try
        {
            window_ = InitializeWindow(instance_, menu_.Get(), nCmdShow);
        }
        catch (Error& e)
        {
            ReportError(L"Error initializing window", e);
            return -1;
        }

        // For mysterious reasons, all the controllers show as disconnected, unless you do this twice with a delay in
        // between.  We'll repeat after the first frame is displayed.
        input_.CheckForNewControllers();
        bool xInputInitialized = false;

        try
        {
            d3d_.Initialize(window_, Display::WIDTH, Display::HEIGHT);
            d3d_.PrepareRenderState();
            d3d_.RenderClear();
        }
        catch (const winrt::hresult_error& e)
        {
            ReportError(L"Error initializing renderer", e);
            return -1;
        }

        try
        {
            wasapi_.Initialize();
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

        host_.SetSampleRate(wasapi_.SampleRate());

        if (romPath.size())
        {
            if (!LoadGame(window_, romPath))
                return -1;

            splash_ = false;
        }
        else
        {
            splash_ = true;
            startSplash_ = true;
        }

        d3d_.SetSplash(splash_);

        uint64_t qpcFreqeuncy = QpcTimer::Frequency();

        auto frameTime = (qpcFreqeuncy + 30) / 60;

        // allow a 5% drift in the frame rate if it means we can match the vsync.
        auto frameDrift = frameTime / 20;

        sampler_ = DynamicSampleRate { wasapi_.SampleRate() / 60 };

        initialized_ = true;
        bool running = false;
        bool hitSync = false;
        auto audioSamples = 0;

        uint64_t splashStartTime = 0;

        while (true)
        {
            MSG msg;
            if (splash_)
            {
                while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) != 0)
                {
                    if (msg.message == WM_QUIT)
                        return 0;

                    if (!TranslateAccelerator(window_, menu_.AcceleratorTable(), &msg))
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                }

                auto currentTime = QpcTimer::Current();

                if (startSplash_)
                {
                    splashStartTime = currentTime;
                    startSplash_ = 0;
                }

                auto splashTime = static_cast<float>(currentTime - splashStartTime) / qpcFreqeuncy;
                if (!d3d_.RenderSplash(splashTime))
                    splash_ = false;
            }
            else if (running)
            {
                auto frameReady = !fullscreen_ || d3d_.WaitForFrame();

                if (frameReady)
                {
                    sampler_.OnFrame(audioSamples, wasapi_.GetPosition());
                    if (hitSync)
                        host_.SetSamplesPerFrame(sampler_.SampleRate());
                }

                while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) != 0)
                {
                    if (msg.message == WM_QUIT)
                        return 0;

                    if (!TranslateAccelerator(window_, menu_.AcceleratorTable(), &msg))
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                }

                if (host_.Loaded())
                {
                    audioSamples = 0;

                    auto currentTime = QpcTimer::Current();

                    // allow ourselves to run slightly ahead if it means we nicely hit a frame boundary.
                    auto targetTime = currentTime + frameDrift;

                    // if we are more than 4 frames behind, let's resync rather than trying to catch up
                    bool outOfSync = !frameReady || (emulatedTime_ + 4 * frameTime) < targetTime;
                    if (!outOfSync)
                    {
                        if (!xInputInitialized)
                        {
                            input_.CheckForNewControllers();
                            menu_.SetControllerConnected(
                                input_.IsConnected(InputDevice::Controller0),
                                input_.IsConnected(InputDevice::Controller1),
                                input_.IsConnected(InputDevice::Controller2),
                                input_.IsConnected(InputDevice::Controller3));
                            xInputInitialized = true;
                        }

                        while (emulatedTime_ < targetTime)
                        {
                            input_.UpdateControllerState();

                            auto anyButton = Buttons::BUTTON_A | Buttons::BUTTON_B | Buttons::BUTTON_SELECT | Buttons::BUTTON_START;

                            // automatically map controllers, if unmapped
                            if (player1Device_ == InputDevice::None)
                            {
                                for (
                                    auto device = InputDevice::Keyboard;
                                    device <= InputDevice::Controller3;
                                    device = static_cast<InputDevice>(static_cast<uint32_t>(device) + 1))
                                {
                                    if (device != player2Device_ && input_.GetState(device) & anyButton)
                                    {
                                        player1Device_ = device;
                                        menu_.SetPlayer1Device(device);
                                        break;
                                    }
                                }
                            }

                            if (player2Device_ == InputDevice::None)
                            {
                                for (
                                    auto device = InputDevice::Keyboard;
                                    device <= InputDevice::Controller3;
                                    device = static_cast<InputDevice>(static_cast<uint32_t>(device) + 1))
                                {
                                    if (device != player1Device_ && (input_.GetState(device) & anyButton))
                                    {
                                        player2Device_ = device;
                                        menu_.SetPlayer2Device(device);
                                    }
                                }
                            }

                            host_.SetController1State(input_.GetState(player1Device_));
                            host_.SetController2State(input_.GetState(player2Device_));
                            host_.RunFrame();
                            emulatedTime_ += frameTime;

                            audioSamples += host_.SamplesPerFrame();
                            if (!wasapi_.WriteSamples(host_.AudioSamples(), host_.SamplesPerFrame()))
                            {
                                // our audio has somehow got too far behind
                                outOfSync = true;
                                break;
                            }
                        }
                    }

                    if (outOfSync)
                    {
                        hitSync = false;
                        emulatedTime_ = currentTime + frameTime;

                        audioSamples = 0;

                        wasapi_.Stop();
                        sampler_.Reset();

                        wasapi_.Start();
                        wasapi_.WritePadding(sampler_.TargetLatency());
                    }
                    else
                    {
                        d3d_.RenderFrame(host_.PixelData(), fullscreen_ && foreground_ ? 0 : 1);
                    }

                    // allow ourselves to run slightly behind if it means we nicely hit a frame boundary.
                    bool hitSync = emulatedTime_ > currentTime + frameTime - frameDrift;
                    if (hitSync)
                    {
                        emulatedTime_ = currentTime + frameTime;
                    }
                }

                running = host_.Running();
                if (!running)
                {
                    StopRunning();

                    if (!host_.Loaded())
                        d3d_.RenderClear();
                }
            }
            else
            {
                if (!GetMessage(&msg, NULL, 0U, 0U))
                    return 0;

                if (!TranslateAccelerator(window_, menu_.AcceleratorTable(), &msg))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }

                running = host_.Running();

                if (running)
                {
                    StartRunning();
                }
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

void App::StartRunning()
{
    auto hThread = GetCurrentThread();
    SetThreadPriority(hThread, THREAD_PRIORITY_ABOVE_NORMAL);

    // wait for a vsync to get an accurate time
    d3d_.WaitForFrame();

    emulatedTime_ = QpcTimer::Current();

    if (host_.Running())
        d3d_.RepeatLastFrame();
    else
        d3d_.RenderClear();

    wasapi_.Start();
}

void App::StopRunning()
{
    auto hThread = GetCurrentThread();
    SetThreadPriority(hThread, THREAD_PRIORITY_NORMAL);

    wasapi_.Stop();
    sampler_.Reset();
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
    ZeroMemory(&wndClass, sizeof(WNDCLASS));
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

    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, true);

    auto wnd = CreateWindow(
        className,
        L"Arcane",
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

    DragAcceptFiles(wnd, TRUE);

    return wnd;
}

void App::Unload()
{
    host_.Stop();
    host_.Unload();
    save_.Close();
    menu_.SetLoaded(false);
    SetWindowText(window_, L"Arcane");

    d3d_.ClearFrameBuffer();

    splash_ = true;
    startSplash_ = true;
    d3d_.SetSplash(true);
}

bool App::LoadGame(HWND wnd, const std::wstring& romPath)
{
    std::unique_ptr<RomFile> romFile;
    try
    {
        romFile = LoadCart(romPath);
    }
    catch (const Error& e)
    {
        ReportError(L"Error loading cartridge", e);
        return false;
    }
    catch (const winrt::hresult_error& e)
    {
        ReportError(L"Error loading cartridge", e);
        return false;
    }

    auto cart = TryCreateCart(
        romFile->Descriptor,
        std::move(romFile->PrgData),
        std::move(romFile->ChrData));

    if (!cart)
    {
        MessageBox(wnd, L"The selected game is not supported", L"Error loading game", MB_ICONERROR | MB_OK);
        return false;
    }

    // Now we've loaded the ROM, we can mount the new save file
    save_.Close();

    auto batteryBackedMemorySize = romFile->Descriptor.PrgBatteryRamSize;
    if (batteryBackedMemorySize)
    {
        try
        {
            auto dotPos = romPath.find_last_of('.');
            auto withoutExtension = romPath.substr(0, dotPos);
            auto savPath = withoutExtension + L".sav";

            save_.Create(savPath, batteryBackedMemorySize);
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

        auto batteryRam = save_.Data();
        if (batteryRam)
            cart->SetPrgBatteryRam(batteryRam);
    }

    cart->Initialize();

    host_.Load(std::move(cart));
    host_.Start();

    menu_.SetLoaded(true);

    auto slashPath = romPath.find_last_of('\\');
    auto dotPos = romPath.find_last_of('.');
    if (dotPos >= 0)
        SetWindowText(window_, (romPath.substr(slashPath + 1, dotPos - slashPath - 1) + L" - Arcane").c_str());
    else
        SetWindowText(window_, L"Arcane");

    splash_ = false;
    d3d_.SetSplash(false);

    return true;
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

    if (GetOpenFileName(&ofn))
    {
        Unload();

        std::wstring romPath(ofn.lpstrFile);
        LoadGame(window, romPath);
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
        if (initialized_)
        {
            if (inSizeMove_)
            {
                resized_ = true;
            }
            else
            {
                if (host_.Running())
                {
                    StopRunning();
                    d3d_.OnSize();
                    StartRunning();
                }
                else
                {
                    d3d_.OnSize();
                    d3d_.RepeatLastFrame();
                }
                return 0;
            }
        }
        break;

    case WM_SYSKEYDOWN:
        // ALT+ENTER
        if (wParam == VK_RETURN && (lParam & (1 << 29)) != 0)
        {
            SetFullscreen(!fullscreen_);
        }
        break;

    case WM_ENTERMENULOOP:
        if (host_.Running())
            StopRunning();
        return 0;

    case WM_EXITMENULOOP:
        if (host_.Running())
            StartRunning();
        return 0;

    case WM_ENTERSIZEMOVE:
        inSizeMove_ = true;
        resized_ = false;
        if (host_.Running())
            StopRunning();
        return 0;

    case WM_EXITSIZEMOVE:
        inSizeMove_ = false;
        if (resized_)
        {
            d3d_.OnSize();
            if (!host_.Running())
            {
                d3d_.RepeatLastFrame();
            }
        }

        if (host_.Running())
            StartRunning();
        return 0;

    case WM_ACTIVATE:
        foreground_ = wParam != WA_INACTIVE;
        break;

    case WM_DROPFILES:
        if (DropFiles(reinterpret_cast<HDROP>(wParam)))
            return 0;
        break;

    case WM_DEVICECHANGE:
        if (wParam == DBT_DEVNODES_CHANGED)
        {
            input_.CheckForNewControllers();
            menu_.SetControllerConnected(
                input_.IsConnected(InputDevice::Controller0),
                input_.IsConnected(InputDevice::Controller1),
                input_.IsConnected(InputDevice::Controller2),
                input_.IsConnected(InputDevice::Controller3));

            if (!input_.IsConnected(player1Device_))
                player1Device_ = InputDevice::None;

            if (!input_.IsConnected(player2Device_))
                player2Device_ = InputDevice::None;
        }
    }

    return DefWindowProc(window_, uMsg, wParam, lParam);
}

bool App::ProcessKey(WPARAM key, bool down)
{
    if (input_.ProcessKey(static_cast<uint32_t>(key), down))
        return true;

    switch (key)
    {
    case VK_ESCAPE:
        if (fullscreen_)
            SetFullscreen(false);
        return true;

#if DIAGNOSTIC
    case VK_PAUSE:
        if (down)
        {
            if (host_.Running())
                host_.Stop();
            else
                host_.Start();
        }
        return true;

    case VK_SPACE:
        if (down)
            host_.Step();
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

    if (command == static_cast<WORD>(MenuCommand::Close))
    {
        Unload();
        return true;
    }

    if (command == static_cast<WORD>(MenuCommand::Exit))
    {
        DestroyWindow(window_);
        return true;
    }

    if (command == static_cast<WORD>(MenuCommand::Restart))
    {
        host_.Restart();
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

    if (command == static_cast<WORD>(MenuCommand::RewindEnabled))
    {
        rewindEnabled_ = !rewindEnabled_;
        if (rewindEnabled_)
            host_.EnableRewind();
        else
            host_.DisableRewind();

        menu_.SetRewindEnabled(rewindEnabled_);
    }

    if (command == static_cast<WORD>(MenuCommand::Fullscreen))
    {
        SetFullscreen(!fullscreen_);
        return true;
    }

    if (command == static_cast<WORD>(MenuCommand::Overscan))
    {
        overscan_ = !overscan_;
        d3d_.SetOverscan(overscan_);
        menu_.SetOverscan(overscan_);
        return true;
    }

    if (command == static_cast<WORD>(MenuCommand::IntegerScaling))
    {
        integerScaling_ = !integerScaling_;
        d3d_.SetIntegerScaling(integerScaling_);
        menu_.SetIntegerScaling(integerScaling_);
        return true;
    }

    if (command == static_cast<WORD>(MenuCommand::Scanlines))
    {
        scanlines_ = !scanlines_;
        d3d_.SetScanlines(scanlines_);
        menu_.SetScanlines(scanlines_);
        return true;
    }

    if (command >= static_cast<WORD>(MenuCommand::InputPlayer1Keyboard)
        && command <= static_cast<WORD>(MenuCommand::InputPlayer1Controller3))
    {
        player1Device_ = static_cast<InputDevice>(command - static_cast<WORD>(MenuCommand::InputPlayer1Keyboard) + 1);
        menu_.SetPlayer1Device(player1Device_);
        if (player1Device_ == player2Device_)
        {
            player2Device_ = InputDevice::None;
            menu_.SetPlayer2Device(player2Device_);
        }
    }

    if (command >= static_cast<WORD>(MenuCommand::InputPlayer2Keyboard)
        && command <= static_cast<WORD>(MenuCommand::InputPlayer2Controller3))
    {
        player2Device_ = static_cast<InputDevice>(command - static_cast<WORD>(MenuCommand::InputPlayer2Keyboard) + 1);
        menu_.SetPlayer2Device(player2Device_);
        if (player2Device_ == player1Device_)
        {
            player1Device_ = InputDevice::None;
            menu_.SetPlayer1Device(player1Device_);
        }
    }

    if (command == static_cast<WORD>(MenuCommand::About))
    {
        about_.Show(window_);
    }

    return false;
}

bool App::DropFiles(HDROP drop)
{
    auto count = DragQueryFile(drop, 0xffffffff, NULL, 0);

    if (count != 1)
        return false;

    wchar_t path[MAX_PATH];
    if (!DragQueryFile(drop, 0, path, MAX_PATH))
        return false;

    DragFinish(drop);

    Unload();

    std::wstring romPath(path);
    LoadGame(window_, romPath);

    return true;
}

void App::SetFullscreen(bool fullscreen)
{
    if (fullscreen == fullscreen_)
        return;

    fullscreen_ = fullscreen;

    if (fullscreen)
    {
        GetWindowRect(window_, &windowRect_);
        auto fullscreenRect = d3d_.GetFullscreenRect();

        SetWindowLong(window_, GWL_STYLE, WS_OVERLAPPED);
        SetWindowPos(
            window_,
            HWND_NOTOPMOST,
            fullscreenRect.left,
            fullscreenRect.top,
            fullscreenRect.right - fullscreenRect.left,
            fullscreenRect.bottom - fullscreenRect.top,
            SWP_FRAMECHANGED | SWP_NOACTIVATE);
        SetMenu(window_, NULL);
        ShowWindow(window_, SW_MAXIMIZE);
    }
    else
    {
        SetWindowLong(window_, GWL_STYLE, WS_OVERLAPPEDWINDOW);
        SetWindowPos(
            window_,
            HWND_NOTOPMOST,
            windowRect_.left,
            windowRect_.top,
            windowRect_.right - windowRect_.left,
            windowRect_.bottom - windowRect_.top,
            SWP_FRAMECHANGED | SWP_NOACTIVATE);
        SetMenu(window_, menu_.Get());
        ShowWindow(window_, SW_NORMAL);
    }
}
