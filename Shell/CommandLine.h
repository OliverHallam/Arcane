#pragma once

#include <string>

class CommandLine
{
public:
    CommandLine();
    ~CommandLine();

    void Parse();

    std::wstring RomPath() const;

private:
    std::wstring romPath_;

    int argCount_;
    LPWSTR* args_;
};