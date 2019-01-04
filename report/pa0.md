1. 
```
src/cpu/exec/exec.c: In function 'exec_wrapper':
src/cpu/exec/exec.c:241:3: error: 'strcat' accessing 81 or more bytes at offsets 264 and 184 may overlap 1 byte at offset 264 [-Werror=restrict]
   strcat(decoding.asm_buf, decoding.assembly);
   ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
cc1: all warnings being treated as errors
make: *** [Makefile:33: build/obj/cpu/exec/exec.o] Error 1
```
change the gcc to clang


2. where to put the xxx.pdf

3. master branch is useless ?
