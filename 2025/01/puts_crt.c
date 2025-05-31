#include <stdio.h>
#include <wchar.h>
int main(void){
    fputws(L"Nothing can be done/Ничего не поделаешь\n", stdout);
    size_t u = wcslen(L"Test!");
    for (size_t i = 0; i<u;i++){
        fputws(L"Character detected\n", stdout);
    }
    return 0;
}
