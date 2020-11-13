#pragma once

#include "Error.h"
#include "Host.h"

#include "../NesCoreCpp/NesSystem.h"
#include "../NesCoreCpp/RomFile.h"

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow);

void ReportError(HWND window, const wchar_t* title, const Error& error);
void ReportError(HWND window, const wchar_t* title, const winrt::hresult_error& error);

HWND InitializeWindow(HINSTANCE hInstance, int nCmdShow);

std::unique_ptr<Cart> LoadGame(HWND wnd, const std::wstring& romPath);
std::unique_ptr<RomFile> LoadCart(const std::wstring& romPath);

void Open(HWND window);

LRESULT CALLBACK WindowProc(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam);

bool ProcessKey(HWND window, WPARAM key, bool down);
