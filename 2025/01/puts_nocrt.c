#include <windows.h>

size_t _wcslen(const wchar_t* str) {
    size_t len = 0;
    while (*str != L'\0') {
        len++;
        str++; // Move to the next character
    }
    return len;
}

void _putws(const wchar_t* str) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD strLen = _wcslen(str);
    DWORD charsWritten;
    WriteConsoleW(
        hConsole,             // console handle
        str,                  // string to write
        strLen,               // length of the string
        &charsWritten,        // number of characters written
        NULL                  // Reserved, must be NULL
    );
    const wchar_t newline[] = L"\r\n";
    WriteConsoleW(hConsole, newline, 2, &charsWritten, NULL);
}

void _start(void){
    _putws(L"Nothing can be done/Ничего не поделаешь");
    size_t u = _wcslen(L"Test!");
    for (size_t i = 0; i<u;i++){
        _putws(L"Character detected");
    }
}