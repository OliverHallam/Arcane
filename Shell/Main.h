#pragma once

#include "Error.h"

#include "../NesCoreCpp/NesSystem.h"
#include "../NesCoreCpp/RomFile.h"

extern std::unique_ptr<NesSystem> System;

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow);

void ReportError(HWND window, const wchar_t* title, const Error& error);
void ReportError(HWND window, const wchar_t* title, const winrt::hresult_error& error);

HWND InitializeWindow(HINSTANCE hInstance, int nCmdShow);

std::unique_ptr<RomFile> LoadCart(const std::wstring& romPath);

LRESULT CALLBACK WindowProc(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam);
