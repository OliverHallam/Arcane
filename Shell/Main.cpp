#include "pch.h"

#include "Main.h"

#include "App.h"

#include <memory>

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow)
{
    auto app = std::make_unique<App>(hInstance);

    return app->Run(nCmdShow);
}
