#pragma once

class SaveFile
{
public:
    void Create(const std::wstring& path, uint32_t size);
    void Close();

    uint8_t* Data();

    void Flush();

private:
    winrt::file_handle hFile_;
    winrt::handle mapping_;
    uint8_t* data_{};
    uint32_t size_{};
};