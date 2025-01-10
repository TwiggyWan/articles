#include <windows.h>
#include <stdio.h>
int main(void){
    putws(L"Nothing can be done/Ничего не поделаешь");
    size_t u = wcslen(L"Test!");
    for (size_t i = 0; i<u;i++){
        putws(L"Character detected");
    }
    return 0;
}