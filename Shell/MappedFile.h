#pragma once

#include <cstdint>

class MappedFile
{
public:
    MappedFile();
    ~MappedFile();

    void Open(const std::wstring& path, uint32_t maxSize);

    uint8_t Data();
    uint32_t Size();

private:
    HANDLE fileHandle_;
    HANDLE fileMappingHandle_;
};