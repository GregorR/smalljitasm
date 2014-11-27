#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define USE_SJA_X8664_SHORT_NAMES
#include "sja/sja.h"

int main()
{
    struct Buffer_uchar buf;
    struct SJA_Operation op;

    INIT_BUFFER(buf);

    memset(&op, 0, sizeof(op));
    op.inst = ADD;
    op.o[0] = RAX;
    op.o[1] = MEM(0, RNONE, RAX, 1);

    sja_compile(op, &buf);

    fwrite(buf.buf, 1, buf.bufused, stdout);

    return 0;
}
