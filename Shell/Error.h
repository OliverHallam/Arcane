#pragma once

#include <string>

class Error
{
public:
    Error(std::wstring message);

    const std::wstring& Message() const;

private:
    std::wstring message_;
};