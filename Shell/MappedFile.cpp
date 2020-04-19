#include "pch.h"

#include "Error.h"
#include "MappedFile.h"

MappedFile::MappedFile()
    : fileHandle_{ INVALID_HANDLE_VALUE },
    fileMappingHandle_{ INVALID_HANDLE_VALUE }
{
}

MappedFile::~MappedFile()
{
    if (fileMappingHandle_ != INVALID_HANDLE_VALUE)
        CloseHandle(fileMappingHandle_);

    if (fileHandle_ != INVALID_HANDLE_VALUE)
        CloseHandle(fileHandle_);
}

void MappedFile::Open(const std::wstring& path, uint32_t maxSize)
{
    fileHandle_ = CreateFile(
        path.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    if (fileHandle_ == INVALID_HANDLE_VALUE)
        winrt::throw_last_error();

    LARGE_INTEGER fileSize;
    if (GetFileSizeEx(fileHandle_, &fileSize) == FALSE)
        winrt::throw_last_error();

    if (fileSize.QuadPart > maxSize)
        throw Error(L"File is suspiciously large!");

    auto mapping = CreateFileMapping(
        fileHandle_,
        NULL,
        PAGE_READONLY,
        0,
        0,
        NULL);
    if (mapping == NULL)
        winrt::throw_last_error();

}
