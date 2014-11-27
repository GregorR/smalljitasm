#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "sja/sja.h"

/* turn an operation list into a program fragment */
void sja_compile(struct SJA_Operation op, struct Buffer_uchar *buf)
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
                        case 8:  if (op.o[si].imm >= 0x80   || op.o[si].imm < -0x80)   bad = 1; break;
                        case 16: if (op.o[si].imm >= 0x8000 || op.o[si].imm < -0x8000) bad = 1; break;
                        /* FIXME: 32 */
                    }
                    break;

                case SJA_X8664_OTYPE_REG:
                    /* make sure the register size is supported */
                    if (!(enc->osz[si] & op.o[si].reg.sz)) bad = 1;
                    break;
            }
            if (bad) break;
        }
        if (!bad) break;
    }

    if (ii == op.inst->encodingCt) {
        fprintf(stderr, "Invalid operation!\n");
        exit(1);
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
                if (op.o[si].index.reg >= SJA_X8664_R8 ||
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

    /* now write the opcode */
    WRITE_ONE_BUFFER(*buf, enc->opcode);

    /* then go step-by-step */
    for (si = 0;; si++) {
        unsigned char step = enc->steps[si];
        if (step == SJA_X8664_ES_END) break;
        switch (step) {
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
                if (step == SJA_X8664_ES_IMM32) break;
                WRITE_ONE_BUFFER(*buf, op.o[arg].imm >> 32);
                WRITE_ONE_BUFFER(*buf, op.o[arg].imm >> 40);
                WRITE_ONE_BUFFER(*buf, op.o[arg].imm >> 48);
                WRITE_ONE_BUFFER(*buf, op.o[arg].imm >> 56);
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

                    disp = op.o[arg].disp ? 1 : 0;

                    /* certainly we need a sib if there's an offset... */
                    if (op.o[arg].imm) sib = 1;

                    /* but we also need a sib if the register isn't otherwise supported */
                    else if (op.o[arg].reg.reg == SJA_X8664_SP ||
                             op.o[arg].reg.reg == SJA_X8664_R12 ||
                             op.o[arg].reg.reg > SJA_X8664_R15) sib = 1;
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
                     *     4. Base is RBP or R13 with or without displacement
                     * 01: 8-bit displacement (never used)
                     * 10: 32-bit displacement */
                    if (((op.o[arg].reg.reg != SJA_X8664_RNONE) != !!op.o[arg].disp) ||
                        ((op.o[arg].reg.reg == SJA_X8664_RNONE) && !op.o[arg].disp)) {
                        /* 00 */

                    } else if (op.o[arg].reg.reg == SJA_X8664_BP || op.o[arg].reg.reg == SJA_X8664_R13) {
                        /* 00 with forced displacement */
                        disp = 1;

                    } else if (op.o[arg].disp) {
                        /* index and displacement */
                        mrmv |= (0x2 << 6);

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
                        sibv |= (0x5 << 3);

                    } else {
                        sibv |= ((op.o[arg].reg.reg & 0x7) << 3);
                        if (op.o[arg].reg.reg >= SJA_X8664_R8)
                            REXB;

                    }

                    WRITE_ONE_BUFFER(*buf, sibv);
                }

                if (disp) {
                    /* now we write a 32-bit displacement */
                    long disp = op.o[arg].disp;
                    WRITE_ONE_BUFFER(*buf, disp);
                    WRITE_ONE_BUFFER(*buf, disp >> 8);
                    WRITE_ONE_BUFFER(*buf, disp >> 16);
                    WRITE_ONE_BUFFER(*buf, disp >> 24);
                }

                break;
            }
        }

    }
}
