void _start(void){
    __asm__ volatile (
		"xorl %edi, %edi\n"  // status = 0
        "movl $60, %eax\n"   // syscall number for exit
        "syscall\n"
    );
}
