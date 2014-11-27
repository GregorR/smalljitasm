#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>

#define USE_SJA_X8664_SHORT_NAMES
#include "sja/sja.h"

int main()
{
    struct Buffer_uchar buf;
    struct SJA_Operation op;
    unsigned char *xbuf;
    int (*xfun)(int, int);

    INIT_BUFFER(buf);

#define C3(x, o1, o2, o3)   sja_compile(OP3(x, o1, o2, o3), &buf)
#define C2(x, o1, o2)       sja_compile(OP2(x, o1, o2), &buf)
#define C1(x, o1)           sja_compile(OP1(x, o1), &buf)
#define C0(x)               sja_compile(OP0(x), &buf)
    C2(ENTER, IMM(0), IMM(0));
    C2(LEA, RAX, MEM(RDI, 1, RSI, 0));
    C0(LEAVE);
    C0(RET);

    /*fwrite(buf.buf, 1, buf.bufused, stdout);*/

    xbuf = mmap(NULL, 4096, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_ANON, -1, 0);
    fprintf(stderr, "%p\n", (void *) xbuf);
    xfun = (int(*)(int,int)) xbuf;
    memcpy(xbuf, buf.buf, buf.bufused);
    printf("Hello %d\n", xfun(42, 8));

    return 0;
}
