#include <stddef.h>
#include <stdint.h>

size_t _wcslen(const wchar_t* str) {
    size_t len = 0;
    while (*str != L'\0') {
        len++;
        str++; // Move to the next character
    }
    return len;
}



// Minimal UTF-32 (Linux wchar_t) to UTF-8 conversion
static int wchar_to_utf8(char *out, wchar_t wc) {
    if (wc < 0x80) {
        out[0] = wc;
        return 1;
    } else if (wc < 0x800) {
        out[0] = 0xC0 | (wc >> 6);
        out[1] = 0x80 | (wc & 0x3F);
        return 2;
    } else if (wc < 0x10000) {
        out[0] = 0xE0 | (wc >> 12);
        out[1] = 0x80 | ((wc >> 6) & 0x3F);
        out[2] = 0x80 | (wc & 0x3F);
        return 3;
    } else {
        out[0] = 0xF0 | (wc >> 18);
        out[1] = 0x80 | ((wc >> 12) & 0x3F);
        out[2] = 0x80 | ((wc >> 6) & 0x3F);
        out[3] = 0x80 | (wc & 0x3F);
        return 4;
    }
}

// Write to stdout using syscall (fd=1)
static void sys_write(const char *buf, size_t len) {
    register long rax asm("rax") = 1; // SYS_write
    register long rdi asm("rdi") = 1; // fd=1 (stdout)
    register const char *rsi asm("rsi") = buf;
    register size_t rdx asm("rdx") = len;
    asm volatile ("syscall"
        : "+r"(rax)
        : "r"(rdi), "r"(rsi), "r"(rdx)
        : "rcx", "r11", "memory");
}

// Ported _putws
void _putws(const wchar_t* str) {
    char utf8buf[8];
    for (; *str; ++str) {
        int n = wchar_to_utf8(utf8buf, *str);
        sys_write(utf8buf, n);
    }
    sys_write("\n", 1); // Linux uses \n for newline
}
void _start(void){
    _putws(L"Nothing can be done/Ничего не поделаешь");
    size_t u = _wcslen(L"Test!");
    for (size_t i = 0; i<u;i++){
        _putws(L"Character detected");
    }

	__asm__ volatile ( //return 0 (otherwise segfault)
		"xorl %edi, %edi\n"  // status = 0
        "movl $60, %eax\n"   // syscall number for exit
        "syscall\n"
    );
}
