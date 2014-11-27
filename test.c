#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define USE_SJA_X8664_SHORT_NAMES
#include "sja/sja.h"

int main()
{
    struct Buffer_uchar buf;
    struct SJA_Operation op, nop;
    memset(&nop, 0, sizeof(nop));

    INIT_BUFFER(buf);

    op = nop;
    op.inst = LEA;
    op.o[0] = RAX;
    op.o[1] = MEM(0, RNONE, RSP, 1000);

    sja_compile(op, &buf);

    fwrite(buf.buf, 1, buf.bufused, stdout);

    return 0;
}
