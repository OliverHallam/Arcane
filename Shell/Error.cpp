#include "pch.h"

#include "Error.h"

Error::Error(std::wstring message)
    : message_(std::move(message))
{
}

const std::wstring& Error::Message() const
{
    return message_;
}
