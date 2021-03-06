//
// Created by ��˳ on 2019/5/3.
//

#ifndef PARSE_E_FILE_FILEBUFFER_H
#define PARSE_E_FILE_FILEBUFFER_H

#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <codecvt>
#include <windows.h>
#include "llvm/ADT/StringRef.h"
using StringRef = llvm::StringRef;
struct FixedData {
    char *data{nullptr};
    size_t length{0};
    FixedData(char *data, size_t length) : data(data), length(length) {}
    FixedData() = default;
    inline bool empty() { return length == 0; }
    std::wstring toUnicode();
    std::string toUtf8();
    std::string toString() {
        return std::string(data, length);
    }
    StringRef toStringRef() { return {data, length}; }
    inline bool operator==(const FixedData&cmp) {
        return length == cmp.length && memcmp(data, cmp.data, length) == 0;
    }
    inline bool operator<(const FixedData&cmp) {
        return length < cmp.length;
    }
    inline bool operator==(const char *str) {
        int len = strlen(str);
        return length == len && memcmp(data, str, length) == 0;
    }
    inline char operator[](const int &val) { return data[val]; }
    inline int count(char ch) {
        int ref = 0;
        while (ref < length && data[ref] == '*') {
            ref++;
        }
        return ref;
    }
};

class FileBuffer {
public:
    uint8_t *code{nullptr};
    size_t length{0};
    size_t pos{0};
    explicit FileBuffer(const char *file) {
        FILE *f = fopen(file, "rb");
        struct stat st{};
        stat(file, &st);
        length = static_cast<size_t>(st.st_size);
        code = (uint8_t *) malloc(length);
        if (fread(code, sizeof(uint8_t), length, f) < length) {
            std::cout << "read error"  << std::endl;
        }
        fclose(f);
    }
    explicit FileBuffer(const wchar_t *file) {
        HANDLE hFile = CreateFileW(file, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
        length = GetFileSize(hFile, NULL);
        code = (uint8_t *) malloc(length);
        DWORD haveReadByte;
        SetFilePointer(hFile, 10, NULL, FILE_BEGIN);
        ReadFile(hFile, (LPVOID *) code, length, &haveReadByte, 0);
        CloseHandle(hFile);
    }
    FileBuffer(char *data, size_t length) : code((uint8_t *) data), length(length) {}
    void free() {
        ::free(code);
    }
    inline bool Good() { return pos < length; }
    inline void Skip(int step) { pos += step; }
    int ReadInt() {
        // С���ֽ���
        pos += 4;
        return (code[pos - 4] | (code[pos - 3] << 8) | (code[pos - 2] << 16) | (code[pos - 1] << 24));
    }

    short ReadShort() {
        // С���ֽ���
        pos += 2;
        return (code[pos - 2] | (code[pos - 1] << 8));
    }

    uint8_t ReadByte() {
        return code[pos++];
    }

    inline FixedData ReadFixedData() {
        FixedData data((char *) &code[pos], ReadInt());
        if (data.length > 0) {
            pos += data.length;
            return data;
        }
        return {};
    }

    inline FixedData ReadString() {
        FixedData data((char *) &code[pos], strlen((char *) &code[pos]));
        pos += data.length + 1;
        return data;
    }

    inline FixedData Read(int len) {
        FixedData data((char *) &code[pos], len);
        pos += data.length;
        return data;
    }

    inline bool Match(uint8_t byte) {
        if (code[pos] == byte) {
            pos++;
            return true;
        }
        return false;
    }

    inline bool Check(uint8_t byte) { return code[pos] == byte; }

};


#endif //PARSE_E_FILE_FILEBUFFER_H
