//
// Created by 曹顺 on 2019/5/4.
//

#ifndef PARSE_E_FILE_TOOLS_H
#define PARSE_E_FILE_TOOLS_H

#include <windows.h>
#include <string>
char *GetLibPath();

std::wstring AnsiToUnicode(const char *str);
std::string UnicodeToAnsi(const wchar_t *str);
#define A2W(ansi) (AnsiToUnicode((const char *)(ansi)).c_str())
#define W2A(unicode) (UnicodeToAnsi((const wchar_t *)(unicode)).c_str())


#endif //PARSE_E_FILE_TOOLS_H
