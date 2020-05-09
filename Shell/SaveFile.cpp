#include "pch.h"

#include "Error.h"

#include "SaveFile.h"

void SaveFile::Create(const std::wstring& path)
{
    hFile_.attach(
        CreateFile(
            path.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ,
            NULL,
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL));

    auto fileExists = GetLastError() == ERROR_ALREADY_EXISTS;

    winrt::check_bool(bool{ hFile_ });

    LARGE_INTEGER fileSize;
    if (GetFileSizeEx(hFile_.get(), &fileSize) == FALSE)
        winrt::throw_last_error();

    if (fileExists)
    {
        if (fileSize.QuadPart != 0x2000)
            throw Error(L"Unexpected save file size");
    }

    mapping_.attach(
        CreateFileMapping(
            hFile_.get(),
            NULL,
            PAGE_READWRITE,
            0,
            0x2000,
            NULL));
    winrt::check_bool(bool{ mapping_ });

    data_ = reinterpret_cast<uint8_t*>(MapViewOfFile(mapping_.get(), FILE_MAP_ALL_ACCESS, 0, 0, 0x2000));
    if (data_ == nullptr)
        winrt::throw_last_error();
}

uint8_t* SaveFile::Data()
{
    return data_;
}

void SaveFile::Flush()
{
    if (data_)
        FlushViewOfFile(data_, 0x2000);
}
