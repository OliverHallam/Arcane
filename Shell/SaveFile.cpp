#include "pch.h"

#include "Error.h"

#include "SaveFile.h"

void SaveFile::Create(const std::wstring& path, uint32_t size)
{
    size_ = size;

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
        if (fileSize.QuadPart != size)
            throw Error(L"Unexpected save file size");
    }

    mapping_.attach(
        CreateFileMapping(
            hFile_.get(),
            NULL,
            PAGE_READWRITE,
            0,
            size,
            NULL));
    if (!mapping_)
        winrt::throw_last_error();

    data_ = reinterpret_cast<uint8_t*>(MapViewOfFile(mapping_.get(), FILE_MAP_ALL_ACCESS, 0, 0, size));
    if (data_ == nullptr)
        winrt::throw_last_error();
}

void SaveFile::Close()
{
    if (data_)
    {
        FlushViewOfFile(data_, size_);
        UnmapViewOfFile(data_);
        data_ = nullptr;
    }

    mapping_.close();
    hFile_.close();

    if (GetLastError() != 0)
        winrt::throw_last_error();
}

uint8_t* SaveFile::Data()
{
    return data_;
}

void SaveFile::Flush()
{
    if (data_)
        FlushViewOfFile(data_, size_);
}
