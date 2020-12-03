#pragma once

#include "Error.h"
#include "Host.h"
#include "Menu.h"
#include "SaveFile.h"

#include "D3DRenderer.h"
#include "DynamicSampleRate.h"
#include "RefreshRateSynchronization.h"
#include "WasapiRenderer.h"

#include "../NesCoreCpp/RomFile.h"

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

    std::unique_ptr<Cart> LoadGame(HWND wnd, const std::wstring& romPath);
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

    bool initialized_;

    HINSTANCE instance_;

    HWND window_;
    Host host_;

    SaveFile save_;

    D3DRenderer d3d_;
    WasapiRenderer wasapi_;

    RefreshRateSynchronization refreshRateSync_;
    DynamicSampleRate sampler_{ 800 };
};