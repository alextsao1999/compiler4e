//
// Created by 曹顺 on 2019/5/4.
//

#include "tools.h"

char *GetLibPath() {
    HKEY key;
    if (RegOpenKeyA(HKEY_CURRENT_USER, R"(Software\FlySky\E\Install)", &key) == 0) {
        char buffer[255];
        DWORD dwType;
        DWORD length = 255;
        RegQueryValueExA(key, "Path", nullptr, &dwType, (LPBYTE) buffer, &length);
        char *ret = new char[length + 1];
        memcpy(ret, buffer, length);
        RegCloseKey(key);
        ret[length] = '\0';
        return ret;
    }
    return nullptr;
}

std::wstring AnsiToUnicode(const char *str) {
    size_t alength = strlen(str);
    size_t len = MultiByteToWideChar(0, 0, str, alength, 0, 0);
    std::wstring gstr;
    if (len > 0) {
        gstr.resize(len);
        MultiByteToWideChar(0, 0, str, alength, &gstr.front(), len);
    }
    return gstr;
}
std::string UnicodeToAnsi(const wchar_t *str) {
    size_t llen = wcslen(str);
    size_t len = WideCharToMultiByte(0, 0, str, llen, 0, 0, 0, 0);
    std::string gstr;
    if (len > 0) {
        gstr.resize(len);
        WideCharToMultiByte(0, 0, str, llen, (LPSTR) &gstr.front(), gstr.size(), 0, 0);
    }
    return gstr;
}
