/*
 * SJA general header
 *
 * Copyright (c) 2015 Gregor Richards
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

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
void sja_compile(struct SJA_Operation op, struct Buffer_uchar *buf, size_t *frel);

/* patch an frel entry to point to the next instruction */
void sja_patchFrel(struct Buffer_uchar *buf, size_t frel);

#endif
