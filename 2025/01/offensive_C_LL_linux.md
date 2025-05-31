# Offensive Security resource constrained evasive payloads linux edition
Created Friday 30 May 2025

@c

forké ici <https://github.com/libyanlake/articles>

minimal program with c runtime
==============================
the article starts with compiling empty c program:
	
	int main(void){
		return 0;
	}


	
	$ cd ~/workspace/foss/articles/2025/01
	$ gcc minimal_crt.c -o minimal_crt
	$ file minimal
	minimal: ELF 64-bit LSB pie executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, BuildID[sha1]=c2b4b252ad38131862be3fc759a4e4cd96c7474c, for GNU/Linux 3.2.0, not stripped
	

	$ objdump -T minimal_crt
	
	minimal:     file format elf64-x86-64
	
	DYNAMIC SYMBOL TABLE:
	0000000000000000      DF *UND*  0000000000000000 (GLIBC_2.34) __libc_start_main
	0000000000000000  w   D  *UND*  0000000000000000  Base        _ITM_deregisterTMCloneTable
	0000000000000000  w   D  *UND*  0000000000000000  Base        gmon_start
	0000000000000000  w   D  *UND*  0000000000000000  Base        _ITM_registerTMCloneTable
	0000000000000000  w   DF *UND*  0000000000000000 (GLIBC_2.2.5) __cxa_finalize
<https://refspecs.linuxbase.org/LSB_3.1.1/LSB-Core-generic/LSB-Core-generic/baselib---libc-start-main-.html> 

pourquoi __libc_start_main dit glibc 2.34 quand j'ai 2.36 d'installé?
---------------------------------------------------------------------
	
	$ ldd --version
	ldd (Debian GLIBC 2.36-9+deb12u10) 2.36

	0000000000000000      DF *UND*  0000000000000000 (GLIBC_2.34) __libc_start_main

ici ce que ça dit c'est que __libc_start_main est fourni à partir de la glibc_2.34.
ça veut pas dire que c'est cette glibc que j'ai sur mon système!

on le voit dans le code de la glibc: <https://sourceware.org/git/?p=glibc.git;a=blob;f=csu/libc-start.c;hb=HEAD#l373>
	 363 /* Starting with glibc 2.34, the init parameter is always NULL.  Older
	 364    libcs are not prepared to handle that.  The macro
	 365    DEFINE_LIBC_START_MAIN_VERSION creates GLIBC_2.34 alias, so that
	 366    newly linked binaries reflect that dependency.  The macros below
	 367    expect that the exported function is called
	 368    __libc_start_main_impl.  */
	[...]
	 373   versioned_symbol (libc, libc_start_main_alias_2, libc_start_main, \
	 374                     GLIBC_2_34);
	

le linker peut nous le dire aussi:
	$ gcc -c minimal_crt.c
	$ gcc minimal_crt.o -o minimal_crt -Wl,--trace-symbol=__libc_start_main
	/usr/bin/ld: /usr/lib/gcc/x86_64-linux-gnu/12/../../../x86_64-linux-gnu/Scrt1.o: reference to __libc_start_main
	/usr/bin/ld: /lib/x86_64-linux-gnu/libc.so.6: definition of __libc_start_main # donc là on sait quelle librairie est utilisée
	$ readelf -sW /lib/x86_64-linux-gnu/libc.so.6 | grep __libc_start_main
	  1758: 0000000000027280   321 FUNC    GLOBAL DEFAULT   16 __libc_start_main@@GLIBC_2.34 # symbole par défaut
	  1760: 0000000000027280   321 FUNC    GLOBAL DEFAULT   16 __libc_start_main@GLIBC_2.2.5

pourquoi ya deux __libc_start_main dans libc.so.6?
--------------------------------------------------
il y en a 2 mais un avec @@  et l'autre avec @ . le manuel de readelf nous explique : 
	
		   --syms
			   Displays the entries in symbol table section of the file, if it has
			   one.  If a symbol has version information associated with it then
			   this is displayed as well.  The version string is displayed as a
			   suffix to the symbol name, preceded by an @ character.  For example
			   foo@VER_1.  If the version is the default version to be used when
			   resolving unversioned references to the symbol then it is displayed
			   as a suffix preceded by two @ characters.  For example foo@@VER_2.
	

pourquoi __libc_start_main est mentionné 2 fois dans notre programme minimal_crt?
---------------------------------------------------------------------------------
Enfin, on note une subtilité présente dans notre programme:

	$ readelf --syms -W minimal_crt
	
	Symbol table '.dynsym' contains 6 entries:
	   Num:    Value          Size Type    Bind   Vis      Ndx Name
	
	     1: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND __libc_start_main@GLIBC_2.34 (2)
	
	
	Symbol table '.symtab' contains 35 entries:
	   Num:    Value          Size Type    Bind   Vis      Ndx Name
	
	    18: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND __libc_start_main@GLIBC_2.34
	

le symbole est mentionné 2 fois!
Explication :  <https://blogs.oracle.com/solaris/post/inside-elf-symbol-tables>

	The dynsym is a smaller version of the symtab that only contains global symbols. The information found in the dynsym is therefore also found in the symtab, while the reverse is not necessarily true. 

-> c'est bien le même symbole!


Revenons à nos moutons dans l'article
-------------------------------------
sous windows, on avait 112kb
sous linux, 

	$ ls -alh minimal_crt
	-rwxr-xr-x 1 asemmar asemmar 16K May 30 23:20 minimal_crt
on a vu précédemment qu'on ne linkait que 6 symboles dynamiques. on a expliqué d'où vient __libc_start_main.
avec -Wl,--trace-symbol= on peut trouver d'où viennent les autres:

/usr/bin/ld: /lib/x86_64-linux-gnu/libc.so.6: definition of __cxa_finalize -> libc

les 3 trucs Base sont référencés mais pas définis:

	/usr/bin/ld: /usr/lib/gcc/x86_64-linux-gnu/12/crtbeginS.o: reference to _ITM_deregisterTMCloneTable
	/usr/bin/ld: /usr/lib/gcc/x86_64-linux-gnu/12/../../../x86_64-linux-gnu/libgcc_s.so.1: reference to _ITM_deregisterTMCloneTable
	/usr/bin/ld: /usr/lib/gcc/x86_64-linux-gnu/12/../../../x86_64-linux-gnu/libgcc_s.so.1: reference to _ITM_deregisterTMCloneTable
	

j'ai pas tout pigé mais ce sont des symboles weak, donc si le loader les trouve pas quand on lance le programme ils sont ignorés.

	$ LD_DEBUG=all ./minimal_crt 2>&1 | grep _ITM_deregisterTMCloneTable
		278184:     symbol=_ITM_deregisterTMCloneTable;  lookup in file=./minimal_crt [0]
		278184:     symbol=_ITM_deregisterTMCloneTable;  lookup in file=/lib/x86_64-linux-gnu/libc.so.6 [0]
		278184:     symbol=_ITM_deregisterTMCloneTable;  lookup in file=/lib64/ld-linux-x86-64.so.2 [0]
	
	$ LD_DEBUG=all ./minimal_crt 2>&1 | grep __libc_start_main
		278457:     symbol=__libc_start_main;  lookup in file=./minimal_crt [0]
		278457:     symbol=__libc_start_main;  lookup in file=/lib/x86_64-linux-gnu/libc.so.6 [0]
		278457:     binding file ./minimal_crt [0] to /lib/x86_64-linux-gnu/libc.so.6 [0]: normal symbol `__libc_start_main' [GLIBC_2.34]

dans cet exemple, le loader ne dit pas 'binding file' pour le symbole ``_ITM_deregisterTMCloneTable``
donc il est ignoré
un peu magique mais bon passons. plus d'infos: <http://dbp-consulting.com/tutorials/debugging/linuxProgramStartup.html>

En tout cas, on voit bien qu'on inclut beaucoup moins de merdes que le programme windows.

on rentre par _start dans le programme:
	
	$ objdump -f minimal_crt
	
	minimal:     file format elf64-x86-64
	architecture: i386:x86-64, flags 0x00000150:
	HAS_SYMS, DYNAMIC, D_PAGED
	start address 0x0000000000001040 # ici
	
	$ readelf --syms -W minimal | grep 0000000000001040
		28: 0000000000001040    34 FUNC    GLOBAL DEFAULT   14 _start
	

le linker nous dit d'où ça vient:

	$ gcc minimal_crt.o -o minimal_crt -Wl,--trace-symbol=_start
	/usr/bin/ld: /usr/lib/gcc/x86_64-linux-gnu/12/../../../x86_64-linux-gnu/Scrt1.o: definition of _start

attention  à pas se faire avoir, ça ne vient PAS des sources de gcc, mais de la libc:

	$ realpath /usr/lib/gcc/x86_64-linux-gnu/12/../../../x86_64-linux-gnu/Scrt1.o
	/usr/lib/x86_64-linux-gnu/Scrt1.o
	$ dpkg -S /usr/lib/x86_64-linux-gnu/Scrt1.o
	libc6-dev:amd64: /usr/lib/x86_64-linux-gnu/Scrt1.o
	

dans le code de la libc:
	$ cd ~/workspace/foss/glibc
	$ git checkout glibc-2.36.9000
	# là je triche parce que le LLM m'a aidé à trouver où aller
	# vim /sysdeps/x86_64/start.S
	ENTRY (_start) # le fichier est super bien commenté mais j'ai coupé dans cet extrait, go voir les sources direct
	
		xorl %ebp, %ebp
		mov %RDX_LP, %R9_LP	/* Address of the shared library termination
					   function.  */
	#ifdef __ILP32__
		mov (%rsp), %esi	/* Simulate popping 4-byte argument count.  */
		add $4, %esp
	#else
		popq %rsi		/* Pop the argument count.  */
	#endif
		/* argv starts just at the current stack top.  */
		mov %RSP_LP, %RDX_LP
	[...]
	

ça correspond à ce que je vois avec objdump, c'est beau:

	
	
	$ objdump -d minimal_crt
	0000000000001040 <_start>:
		1040:       31 ed                   xor    %ebp,%ebp
		1042:       49 89 d1                mov    %rdx,%r9
		1045:       5e                      pop    %rsi
		1046:       48 89 e2                mov    %rsp,%rdx
	[...]

second programme: minimal_nocrt
===============================
	$ ls -lh minimal minimal_nocrt
	-rwxr-xr-x 1 asemmar asemmar 16K May 31 02:12 minimal
	-rwxr-xr-x 1 asemmar asemmar 14K May 31 02:38 minimal_nocrt
	cat: /home/asemmar/.kube/config: No such file or directory
	0 02:39 asemmar@okn-69420:~/workspace/foss/articles/2025/01   (tsh: teleport.tooling.owkin.tech) (main =)
	$ ./minimal_nocrt
	Segmentation fault
	

pas une grosse diff de taille
et le programme ne fonctionne plus -> <https://stackoverflow.com/questions/39554933/why-does-returning-from-start-segfault> (je pige rien à l'asm hein)
	
	void _start(void){
	    __asm__ volatile (
		   "xorl %edi, %edi\n"  // status = 0
	        "movl $60, %eax\n"   // syscall number for exit
	        "syscall\n"
	    );
	}
	
ça segfault plus
	
avec les optims:

	$ gcc minimal_nocrt.c -nostartfiles -e _start -O2 -s -o minimal_nocrt_O2
	$ ls -lh minimal_nocrt_O2
	-rwxr-xr-x 1 asemmar asemmar 13K May 31 02:52 minimal_nocrt_O2
on est encore à 13K, beaucoup plus que les 3.5k de l'article. je suis assez surpris

il y a moins de symboles par contre
	#avant
	$ readelf --syms -W minimal_crt | grep contains
	Symbol table '.dynsym' contains 6 entries:
	Symbol table '.symtab' contains 35 entries:
	

	#apres
	$ readelf --syms -W minimal_nocrt | grep contains
	Symbol table '.dynsym' contains 1 entry:
	Symbol table '.symtab' contains 9 entries:
	


3. puts programs
================
puts_crt porté:
	
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
	$ gcc puts_crt.c -o puts_crt -Wall
	$ ./puts_crt
	Nothing can be done/Nichego ne podelaesh`
	Character detected
	Character detected
	Character detected
	Character detected
	Character detected
	$ ls -lh puts_crt
	-rwxr-xr-x 1 asemmar asemmar 16K May 31 03:21 puts_crt # au lieu des 115 kb sous windows mentionnés dans l'article
	$ readelf --syms -W puts_crt | grep contains
	Symbol table '.symtab' contains 38 entries: # 38 imports, pareil que sous windows
	$ ldd puts_crt
	        linux-vdso.so.1 (0x00007ffeb83de000)
	        libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007fcce7982000) # pareil qu'avant, on link que la libc
	        /lib64/ld-linux-x86-64.so.2 (0x00007fcce7b86000)

puts_nocrt porté (merci le LLM putain)
	
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
	
		asm volatile ( //return 0 (otherwise segfault)
			"xorl %edi, %edi\n"  // status = 0
			"movl $60, %eax\n"   // syscall number for exit
			"syscall\n"
		);
	}

	
	$ gcc puts_nocrt.c -nostartfiles -e _start -Wall -o puts_nocrt -nolibc -O2 -s
	$ ldd puts_nocrt
	        statically linked #no dynamic symbols
	$ readelf --syms -W puts_nocrt | grep contains
	Symbol table '.symtab' contains 13 entries:
	$ ls -lh puts_nocrt
	-rwxr-xr-x 1 asemmar asemmar 13K May 31 04:04 puts_nocrt # beaucoup plus que sous windows (6.2kb), toujours 
	$ ./puts_nocrt
	Nothing can be done/Ничего не поделаешь
	Character detected
	Character detected
	Character detected
	Character detected
	Character detected


conclusion: glibc vs CRT:
-------------------------
on observe pas la différence drastique de taille qu'il y a sous windows. donc le bénéfice de se passer de la libc pour échapper à la détection des antivirus semble moins évident.

Partie avec metasploit
======================
### installation de metasploit
<https://docs.metasploit.com/docs/using-metasploit/getting-started/nightly-installers.html#installing-metasploit-on-linux--macos>
on prend le script ici pour ajouter le ppa debian, je le fais à la main parce que j'aime pas curl | bash
	
	sudo -s
	print_pgp_key() {
	  cat <<-EOF
	-----BEGIN PGP PUBLIC KEY BLOCK-----
	
	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
	2vTa24LE0J2DQg==
	=Qa/n
	-----END PGP PUBLIC KEY BLOCK-----
	EOF
	}
	
	LIST_FILE=/etc/apt/sources.list.d/metasploit-framework.list
	PREF_FILE=/etc/apt/preferences.d/pin-metasploit.pref
	DOWNLOAD_URI=https://downloads.metasploit.com/data/releases/metasploit-framework
	echo "deb [signed-by=/usr/share/keyrings/metasploit-framework.gpg] $DOWNLOAD_URI/apt lucid main" > $LIST_FILE
	  print_pgp_key | gpg --dearmor -o /usr/share/keyrings/metasploit-framework.gpg
	mkdir -p /etc/apt/preferences.d/
		cat > $PREF_FILE <<EOF
	Package: metasploit*
	Pin: origin downloads.metasploit.com
	Pin-Priority: 1000
	EOF
	apt update
	apt install metasploit-framework
	
	


le payload qu'on va utiliser: 
	$ msfvenom --platform linux -p linux/x64/exec CMD=x-terminal-emulator -e x64/zutto_dekiru -i 3 -f elf > metasploit_elf
	$ chmod +x metasploit_elf
	$ ./metasploit_elf
	You need to run terminator in an X environment. Make sure $DISPLAY is properly set

-> ça fonctionne, on le fait en tableau c: 

	$ msfvenom --platform linux -p linux/x64/exec CMD=x-terminal-emulator -e x64/zutto_dekiru -i 3 -f c
	
le code cf ~/workspace/foss/articles/2025/01/inject.c
	
	$ gcc inject_crt.c -o inject_crt
	$ ls -lh inject_crt
	-rwxr-xr-x 1 asemmar asemmar 16K May 31 05:18 inject_crt
	$ readelf --syms -W inject_crt | grep symtab
	Symbol table '.symtab' contains 37 entries:
	$ ./inject_crt
	$ ldd inject_crt
	        linux-vdso.so.1 (0x00007ffce4dc8000)
	        libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007fd12dbb6000)
	        /lib64/ld-linux-x86-64.so.2 (0x00007fd12ddba000)

détecté par aucun security vendor dans virustotal lmao



### version sans la libc
cf inject_nocrt.c

	
	$ gcc inject_nocrt.c -o inject_nocrt -nolibc -nostartfiles -e _start
	$ ls -lh inject_nocrt
	-rwxr-xr-x 1 asemmar asemmar 14K May 31 05:42 inject_nocrt
	$ ldd inject_nocrt
			statically linked
	$ readelf --syms -W inject_nocrt | grep symtab
	Symbol table '.symtab' contains 12 entries:
	$ ./inject_nocrt
	You need to run terminator in an X environment. Make sure $DISPLAY is properly set


toujours 0 détections virustotal lmao

à ma connaissance l'exemple avec l'api NT n'a pas d'équivalent sous linux (on utilise tout le temps la libc ya pas de conneries kernel32 vs nt api)

annexe: pourquoi j'ai pas le manuel gcc sous debian?
----------------------------------------------------

$ apt install gcc-doc

