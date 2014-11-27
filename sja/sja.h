#ifndef SJA_SJA_H
#define SJA_SJA_H 1

#include <stdlib.h>
#include <sys/types.h>

#include "buffer.h"

/* choose the appropriate backend for our architecture */
#if defined(__x86_64__)
#define SJA_ARCH_X86_64 1
#include "x8664.h"

#else
#error Unsupported architecture!

#endif

BUFFER(uchar, unsigned char);

/* append an operation to a program fragment */
void sja_compile(struct SJA_Operation op, struct Buffer_uchar *buf);

#endif
