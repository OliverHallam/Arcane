#pragma once

#include "Error.h"
#include "Host.h"
#include "Menu.h"
#include "SaveFile.h"

#include "About.h"
#include "D3DRenderer.h"
#include "DynamicSampleRate.h"
#include "WasapiRenderer.h"
#include "Input.h"

#include "../NesCore/RomFile.h"

#include <shellapi.h>

class App
{
public:
    App(HINSTANCE hInstance);

    int Run(int nCmdShow);

private:
    void StartRunning();
    void StopRunning();

    void ReportError(const wchar_t* title, const Error& error);
    void ReportError(const wchar_t* title, const winrt::hresult_error& error);

    HWND InitializeWindow(HINSTANCE hInstance, HMENU menu, int nCmdShow);

    void Unload();
    bool LoadGame(HWND wnd, const std::wstring& romPath);
    std::unique_ptr<RomFile> LoadCart(const std::wstring& romPath);

    void Open(HWND window);

    static LRESULT CALLBACK WindowProcStub(
        HWND hWnd,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam);

    LRESULT WindowProc(
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam);

    bool ProcessKey(WPARAM key, bool down);
    bool ProcessCommand(WORD command);

    bool DropFiles(HDROP drop);

    void SetFullscreen(bool fullscreen);

    bool initialized_;

    HINSTANCE instance_;

    Menu menu_;

    HWND window_;
    RECT windowRect_;

    bool splash_;
    bool startSplash_;

    bool fullscreen_;
    bool overscan_;
    bool integerScaling_;
    bool scanlines_;

    bool rewindEnabled_;

    bool foreground_;

    Host host_;

    SaveFile save_;

    D3DRenderer d3d_;
    WasapiRenderer wasapi_;
    Input input_;

    InputDevice player1Device_;
    InputDevice player2Device_;

    uint64_t emulatedTime_;
    DynamicSampleRate sampler_{ 800 };

    bool inSizeMove_{};
    bool resized_{};

    About about_{};
};