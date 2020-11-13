#include "pch.h"

#include "CommandLine.h"
#include "Error.h"

#include <shellapi.h>

CommandLine::CommandLine()
    : args_{ NULL }
{
    args_ = CommandLineToArgvW(GetCommandLineW(), &argCount_);
}

CommandLine::~CommandLine()
{
    LocalFree(args_);
}

void CommandLine::Parse()
{
    if (argCount_ > 1)
    {
        romPath_ = args_[1];
    }

    if (argCount_ > 2)
    {
        throw Error(L"The command line parameters were invalid");
    }
}


std::wstring CommandLine::RomPath() const
{
    return romPath_;
}
