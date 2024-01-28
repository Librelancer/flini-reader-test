#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

extern "C" class INI_Reader
{
public:

    INI_Reader(void* external = 0);
    ~INI_Reader();

    bool open(LPCSTR path, bool throwExceptionOnFail);
    LPCSTR get_header_ptr();
    LPCSTR get_name_ptr();
    UINT get_num_parameters() const;
    bool read_header();
    bool is_header(LPCSTR header);
    bool read_value();
    bool is_value(LPCSTR value);
    bool get_value_bool(UINT index);
    int get_value_int(UINT index);
    float get_value_float(UINT index);
    LPCSTR get_value_string(UINT index);
    void close();

private:
    BYTE data[0x1568];
};

INI_Reader *WrapIniReader(void* external);
