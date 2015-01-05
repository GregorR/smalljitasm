/*
 * SJA: x86_64 assembler.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "sja/sja.h"

/* append an operation to a program fragment */
void sja_compile(struct SJA_Operation op, struct Buffer_uchar *buf, size_t *frel)
{
    size_t oi, ii, si;
    int bad;
    unsigned char sz, needRex, *rex;
    struct SJA_X8664_Encoding *enc;

    /* figure out the encoding for this instruction */
    for (ii = 0; ii < op.inst->encodingCt; ii++) {
        enc = &op.inst->encodings[ii];

        /* right number of operands? */
        if (!!enc->otypes[0] != !!op.o[0].type ||
            !!enc->otypes[1] != !!op.o[1].type ||
            !!enc->otypes[2] != !!op.o[2].type) continue;

        /* operand types compatible? */
        if (!((enc->otypes[0] & op.o[0].type) == op.o[0].type &&
              (enc->otypes[1] & op.o[1].type) == op.o[1].type &&
              (enc->otypes[2] & op.o[2].type) == op.o[2].type))
            continue;

        /* operand sizes compatible? */
        bad = 0;
        for (si = 0; si < 3; si++) {
            switch (op.o[si].type) {
                case SJA_X8664_OTYPE_IMM:
                    /* make sure it's small enough */
                    switch (enc->osz[si]) {
                        case 1: if (op.o[si].imm >= 0x80   || op.o[si].imm < -0x80)   bad = 1; break;
                        case 2: if (op.o[si].imm >= 0x8000 || op.o[si].imm < -0x8000) bad = 1; break;
                        /* FIXME: 32 */
                    }
                    break;

                case SJA_X8664_OTYPE_RREL:
                case SJA_X8664_OTYPE_FREL:
                    /* since RREL sizes are unpredictable (and I'm lazy) and
                     * FREL sizes aren't known, it must be at least 32 bits */
                    if (enc->osz[si] < 4) bad = 1;
                    break;

                case SJA_X8664_OTYPE_REG:
                    /* make sure the register size is supported */
                    if (!(enc->osz[si] & op.o[si].reg.sz)) bad = 1;
                    break;

                case SJA_X8664_OTYPE_MEM:
                    /* make sure the memory size is supported */
                    if (!(enc->osz[si] & op.o[si].sz)) bad = 1;
                    break;
            }
            if (bad) break;
        }
        if (!bad) break;
    }

    if (ii == op.inst->encodingCt) {
        fprintf(stderr, "Invalid operation!\n");
        abort();
    }

    /* choose the size to use for registers/memory locations,
     * and figure out if we need a REX */
    needRex = 0;
    sz = 0;
#define SETSZ(to) if (sz == 0) sz = (to); else if (sz != (to)) { bad = 1; break; }
    for (si = 0; si < 3; si++) {
        switch (op.o[si].type) {
            case SJA_X8664_OTYPE_REG:
                SETSZ(op.o[si].reg.sz);
                if (op.o[si].reg.reg >= SJA_X8664_R8 ||
                    op.o[si].reg.sz > 4) needRex = 1;
                break;

            case SJA_X8664_OTYPE_MEM:
                SETSZ(op.o[si].sz);
                if (op.o[si].sz > 4 ||
                    op.o[si].index.reg >= SJA_X8664_R8 ||
                    op.o[si].reg.reg >= SJA_X8664_R8 ||
                    op.o[si].index.sz > 4 ||
                    op.o[si].reg.sz > 4) needRex = 1;
                break;
        }
    }
    if (bad) {
        /* multiple register sizes! */
        fprintf(stderr, "Invalid register use!\n");
        exit(1);
    }
#undef SETSZ

    /* if we need a rex, do that first */
    if (needRex) {
        WRITE_ONE_BUFFER(*buf, 0x40);
        rex = &buf->buf[buf->bufused-1];

        if (sz > 4) {
            /* set the rex 'W' bit (i.e., write 64 bits) */
            *rex |= (1<<3);
        }
    }

    /* some macros for setting the rex bits */
#define REXB *rex |= 0x1
#define REXX *rex |= 0x2
#define REXR *rex |= 0x4

    /* need a specifier for 16-bit too */
    if (sz == 2)
        WRITE_ONE_BUFFER(*buf, 0x66);

    /* now write the opcode */
    WRITE_ONE_BUFFER(*buf, enc->opcode);

    /* then go step-by-step */
    for (si = 0;; si++) {
        unsigned char step = enc->steps[si];
        if (step == SJA_X8664_ES_END) break;
        switch (step) {
            case SJA_X8664_ES_FIX:
            {
                unsigned char arg = enc->steps[++si];
                WRITE_ONE_BUFFER(*buf, arg);
                break;
            }

            case SJA_X8664_ES_ADDREG:
            {
                unsigned char arg = enc->steps[++si];
                buf->buf[buf->bufused-1] += op.o[arg].reg.reg;
                break;
            }

            case SJA_X8664_ES_IMM8:
            case SJA_X8664_ES_IMM16:
            case SJA_X8664_ES_IMM32:
            {
                unsigned char arg = enc->steps[++si];
                WRITE_ONE_BUFFER(*buf, op.o[arg].imm);
                if (step == SJA_X8664_ES_IMM8) break;
                WRITE_ONE_BUFFER(*buf, op.o[arg].imm >> 8);
                if (step == SJA_X8664_ES_IMM16) break;
                WRITE_ONE_BUFFER(*buf, op.o[arg].imm >> 16);
                WRITE_ONE_BUFFER(*buf, op.o[arg].imm >> 24);
                break;
            }

            case SJA_X8664_ES_RREL8:
            case SJA_X8664_ES_RREL16:
            case SJA_X8664_ES_RREL32:
            {
                unsigned char arg = enc->steps[++si];
                long rel = op.o[arg].imm;

                /* rel is currently relative to the beginning of the buffer.
                 * Relocate it to where we are now */
                rel -= buf->bufused;

                /* then relocate it past the bits we're about to use */
                switch (step) {
                    case SJA_X8664_ES_RREL8:  rel -= 1; break;
                    case SJA_X8664_ES_RREL16: rel -= 2; break;
                    case SJA_X8664_ES_RREL32: rel -= 4; break;
                }

                /* then write it out */
                WRITE_ONE_BUFFER(*buf, rel);
                if (step == SJA_X8664_ES_RREL8) break;
                WRITE_ONE_BUFFER(*buf, rel >> 8);
                if (step == SJA_X8664_ES_RREL16) break;
                WRITE_ONE_BUFFER(*buf, rel >> 16);
                WRITE_ONE_BUFFER(*buf, rel >> 24);
                break;
            }

            case SJA_X8664_ES_FREL32:
            {
                *frel = buf->bufused;
                WRITE_BUFFER(*buf, "\0\0\0\0", 4);
                break;
            }

            case SJA_X8664_ES_MRM0:
            case SJA_X8664_ES_MRM1:
            case SJA_X8664_ES_MRM2:
            case SJA_X8664_ES_MRM3:
            case SJA_X8664_ES_MRM4:
            case SJA_X8664_ES_MRM5:
            case SJA_X8664_ES_MRM6:
            case SJA_X8664_ES_MRM7:
            case SJA_X8664_ES_MRMR:
            {
                unsigned char rarg = 0, arg;
                unsigned char m = 0, sib = 0, disp = 0;
                unsigned char mrmv, sibv;

                /* do we have a register argument? */
                if (step == SJA_X8664_ES_MRMR)
                    rarg = enc->steps[++si];
                arg = enc->steps[++si];

                /* is it memory? */
                if (op.o[arg].type == SJA_X8664_OTYPE_MEM) {
                    m = 1;

                    /* how shall we handle our displacement? */
                    if (op.o[arg].disp >= 0x80 || op.o[arg].disp < -0x80)
                        disp = 4;
                    else if (op.o[arg].disp)
                        disp = 1;
                    else
                        disp = 0;

                    /* certainly we need a sib if there's an index... */
                    if (op.o[arg].imm) sib = 1;

                    /* but we also need a sib if the register isn't otherwise supported */
                    else if (op.o[arg].reg.reg == SJA_X8664_SP ||
                             op.o[arg].reg.reg == SJA_X8664_R12) sib = 1;

                    /* and we need a sib to do absolute addressing */
                    else if (op.o[arg].reg.reg == SJA_X8664_RNONE) sib = 1;
                }

                /* OK, we know which bytes we need. Now make them */
                mrmv = 0;

                /* first two bits are the mode */
                if (op.o[arg].type == SJA_X8664_OTYPE_REG) {
                    /* mode 11b = register */
                    mrmv |= (0x3 << 6);

                } else {
                    /* memory. The modes are weird to say the least:
                     * 00: Four possibilities:
                     *     1. Displacement but no base
                     *     2. Base but no displacement
                     *     3. Neither base nor displacement
                     *     4. RIP-relative addressing
                     * 01: 8-bit displacement
                     * 10: 32-bit displacement
                     *
                     * Because of nonsense addressing, RBP must always have a
                     * displacement. */
                    if (op.o[arg].reg.reg == SJA_X8664_BP || op.o[arg].reg.reg == SJA_X8664_R13) {
                        /* BP MUST have a displacement */
                        if (!disp) disp = 1;
                        if (disp == 4)
                            mrmv |= (0x2 << 6);
                        else
                            mrmv |= (0x1 << 6);

                    } else if (op.o[arg].reg.reg == SJA_X8664_RIP) {
                        /* for RIP-relative addressing, we must set the mode to
                         * 00 and the register to 101b */
                        /* 00 */
                        disp = 4;

                    } else if (((op.o[arg].reg.reg != SJA_X8664_RNONE) != !!op.o[arg].disp) ||
                               ((op.o[arg].reg.reg == SJA_X8664_RNONE) && !op.o[arg].disp)) {
                        /* 00 */
                        if (disp) disp = 4;

                    } else if (disp) {
                        /* base and displacement */
                        if (disp == 4) {
                            /* 32-bit displacement */
                            mrmv |= (0x2 << 6);
                        } else {
                            mrmv |= (0x1 << 6);
                        }

                    }

                }

                /* next three bits are a either the register (MRMR) or a fixed value */
                if (step == SJA_X8664_ES_MRMR) {
                    mrmv |= ((op.o[rarg].reg.reg & 0x7) << 3);
                    if (op.o[rarg].reg.reg >= SJA_X8664_R8)
                        /* extra bit goes in rex's R bit (bit 2) */
                        REXR;
                } else {
                    /* fixed value */
                    mrmv |= ((step - SJA_X8664_ES_MRM0) << 3);
                }

                /* last three bits are the actual register or memory address being requested */
                if (op.o[arg].type == SJA_X8664_OTYPE_REG) {
                    mrmv |= (op.o[arg].reg.reg & 0x7);
                    if (op.o[arg].reg.reg >= SJA_X8664_R8)
                        /* extra bit goes in rex's B bit (bit 0) */
                        REXB;

                } else if (op.o[arg].type == SJA_X8664_OTYPE_MEM) {
                    /* if we need a sib, we indicate that with 100b */
                    if (sib) {
                        mrmv |= 0x4;

                    /* if we wanted no register at all or RIP-relative
                     * addressing, we indicate that with 101b */
                    } else if (op.o[arg].reg.reg == SJA_X8664_RNONE ||
                               op.o[arg].reg.reg == SJA_X8664_RIP) {
                        mrmv |= 0x5;

                    } else {
                        /* no sib, just encode the register here */
                        mrmv |= (op.o[arg].reg.reg & 0x7);
                        if (op.o[arg].reg.reg >= SJA_X8664_R8)
                            REXB;

                    }

                }
                WRITE_ONE_BUFFER(*buf, mrmv);

                /* now comes the sib */
                if (sib) {
                    sibv = 0;

                    /* first two bytes are the scale */
                    switch (op.o[arg].imm) {
                        case 0:
                        case 1:
                            /* scale = 1 is 00b */
                            break;

                        case 2:
                            sibv |= (0x1 << 6);
                            break;

                        case 4:
                            sibv |= (0x2 << 6);
                            break;

                        case 8:
                            sibv |= (0x3 << 6);
                            break;
                    }

                    /* then comes the index */
                    if (op.o[arg].imm) {
                        /* it has an index */

                        /* RSP is used for encoding no index, and so cannot be
                         * used as an index */
                        if (op.o[arg].index.reg == SJA_X8664_SP) {
                            fprintf(stderr, "Invalid register use: SP cannot be an index.\n");
                            exit(1);
                        }

                        sibv |= ((op.o[arg].index.reg & 0x7) << 3);
                        if (op.o[arg].index.reg >= SJA_X8664_R8)
                            /* extra bit goes in rex's X bit (bit 0) */
                            REXX;

                    } else {
                        /* no index is represented by 100b */
                        sibv |= (0x4 << 3);

                    }

                    /* then the base */
                    if (op.o[arg].reg.reg == SJA_X8664_RNONE) {
                        /* "none" is represented by 101b */
                        sibv |= 0x5;

                    } else {
                        sibv |= (op.o[arg].reg.reg & 0x7);
                        if (op.o[arg].reg.reg >= SJA_X8664_R8)
                            REXB;

                    }

                    WRITE_ONE_BUFFER(*buf, sibv);
                }

                if (disp) {
                    /* now we write the displacement */
                    long dispv = op.o[arg].disp;
                    WRITE_ONE_BUFFER(*buf, dispv);
                    if (disp == 4) {
                        WRITE_ONE_BUFFER(*buf, dispv >> 8);
                        WRITE_ONE_BUFFER(*buf, dispv >> 16);
                        WRITE_ONE_BUFFER(*buf, dispv >> 24);
                    }
                }

                break;
            }
        }

    }
}

/* patch an frel entry to point to the next instruction */
void sja_patchFrel(struct Buffer_uchar *buf, size_t frel)
{
    long rel = buf->bufused - frel - 4;
    buf->buf[frel] = rel;
    buf->buf[frel+1] = rel>>8;
    buf->buf[frel+2] = rel>>16;
    buf->buf[frel+3] = rel>>24;
}
