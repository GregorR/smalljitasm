/*
 * SJA: x86_64 assembler conventions.
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

#ifndef SJA_X8664_H
#define SJA_X8664_H 1

#include <stdlib.h>
#include <sys/types.h>

#ifdef USE_SJA_SHORT_NAMES
#ifndef USE_SJA_X8664_SHORT_NAMES
#define USE_SJA_X8664_SHORT_NAMES 1
#endif
#endif

/* our fundamental register list */
enum sja_x8664_register_name {
    SJA_X8664_AX,
    SJA_X8664_CX,
    SJA_X8664_DX,
    SJA_X8664_BX,
    SJA_X8664_SP,
    SJA_X8664_BP,
    SJA_X8664_SI,
    SJA_X8664_DI,
    SJA_X8664_R8,
    SJA_X8664_R9,
    SJA_X8664_R10,
    SJA_X8664_R11,
    SJA_X8664_R12,
    SJA_X8664_R13,
    SJA_X8664_R14,
    SJA_X8664_R15,

    /* unusual registers */
    SJA_X8664_RNONE,
    SJA_X8664_RIP
};

/* a register is one of the fundamental registers, plus a size */
struct SJA_X8664_Register {
    unsigned char sz;
    unsigned char reg;
};

/* above that, we build all the various argument types that are possible */
enum sja_x8664_operand_type {
    SJA_X8664_OTYPE_NONE = 0,
    SJA_X8664_OTYPE_IMM = 1,
    SJA_X8664_OTYPE_RREL = 2, /* reverse relative address */
    SJA_X8664_OTYPE_FREL = 4, /* forward relative address (to be patched) */
    SJA_X8664_OTYPE_REG = 8,
    SJA_X8664_OTYPE_MEM = 16 /* scale*index + base */
};

struct SJA_X8664_Operand {
    unsigned char type;
    /* the size, for memory operands (otherwise implied by registers) */
    unsigned char sz;
    /* represents:
     *  * the immediate value for IMM
     *  * relative offset for RREL (relative to the beginning of the code buffer)
     *  * scale for MEM, which may be 0, 1, 2, 4 or 8
     */
    long imm;
    /* the index for MEM, ignored otherwise */
    struct SJA_X8664_Register index;
    /* the register for REG, or the base for MEM */
    struct SJA_X8664_Register reg; 
    /* the displacement */
    long disp;
};

/* encoding for the "none" operand */
#define SJA_X8664_ONONE \
    ((struct SJA_X8664_Operand) { SJA_X8664_OTYPE_NONE, 0, 0, \
        (struct SJA_X8664_Register) {0, 0}, \
        (struct SJA_X8664_Register) {0, 0}, \
        0 \
    })

/* encoding for immediate operands */
#define SJA_X8664_OIMMISH(type, val) \
    ((struct SJA_X8664_Operand) { type, 0, val, \
        (struct SJA_X8664_Register) {0, 0}, \
        (struct SJA_X8664_Register) {0, 0}, \
        0 \
    })
#define SJA_X8664_OIMM(val) SJA_X8664_OIMMISH(SJA_X8664_OTYPE_IMM, val)
#define SJA_X8664_ORREL(val) SJA_X8664_OIMMISH(SJA_X8664_OTYPE_RREL, val)
#define SJA_X8664_OFREL SJA_X8664_OIMMISH(SJA_X8664_OTYPE_FREL, 0)
#ifdef USE_SJA_X8664_SHORT_NAMES
#define IMM(val) SJA_X8664_OIMM(val)
#define RREL(val) SJA_X8664_ORREL(val)
#define FREL SJA_X8664_OFREL
#endif

/* encodings for register operands */
#define SJA_X8664_OREG(sz, reg) \
    ((struct SJA_X8664_Operand) { SJA_X8664_OTYPE_REG, 0, 0, \
        (struct SJA_X8664_Register) {0, 0}, \
        (struct SJA_X8664_Register) {(sz), (reg)}, \
        0 \
    })
#ifdef USE_SJA_X8664_SHORT_NAMES
#define RNONE SJA_X8664_OREG(8, SJA_X8664_RNONE)
#define AL SJA_X8664_OREG(1, SJA_X8664_AX)
#define AX SJA_X8664_OREG(2, SJA_X8664_AX)
#define EAX SJA_X8664_OREG(4, SJA_X8664_AX)
#define RAX SJA_X8664_OREG(8, SJA_X8664_AX)
#define CL SJA_X8664_OREG(1, SJA_X8664_CX)
#define CX SJA_X8664_OREG(2, SJA_X8664_CX)
#define ECX SJA_X8664_OREG(4, SJA_X8664_CX)
#define RCX SJA_X8664_OREG(8, SJA_X8664_CX)
#define DL SJA_X8664_OREG(1, SJA_X8664_DX)
#define DX SJA_X8664_OREG(2, SJA_X8664_DX)
#define EDX SJA_X8664_OREG(4, SJA_X8664_DX)
#define RDX SJA_X8664_OREG(8, SJA_X8664_DX)
#define BL SJA_X8664_OREG(1, SJA_X8664_BX)
#define BX SJA_X8664_OREG(2, SJA_X8664_BX)
#define EBX SJA_X8664_OREG(4, SJA_X8664_BX)
#define RBX SJA_X8664_OREG(8, SJA_X8664_BX)
#define AH SJA_X8664_OREG(1, SJA_X8664_SP)
#define SP SJA_X8664_OREG(2, SJA_X8664_SP)
#define ESP SJA_X8664_OREG(4, SJA_X8664_SP)
#define RSP SJA_X8664_OREG(8, SJA_X8664_SP)
#define CH SJA_X8664_OREG(1, SJA_X8664_BP)
#define BP SJA_X8664_OREG(2, SJA_X8664_BP)
#define EBP SJA_X8664_OREG(4, SJA_X8664_BP)
#define RBP SJA_X8664_OREG(8, SJA_X8664_BP)
#define DH SJA_X8664_OREG(1, SJA_X8664_SI)
#define SI SJA_X8664_OREG(2, SJA_X8664_SI)
#define ESI SJA_X8664_OREG(4, SJA_X8664_SI)
#define RSI SJA_X8664_OREG(8, SJA_X8664_SI)
#define BH SJA_X8664_OREG(1, SJA_X8664_DI)
#define DI SJA_X8664_OREG(2, SJA_X8664_DI)
#define EDI SJA_X8664_OREG(4, SJA_X8664_DI)
#define RDI SJA_X8664_OREG(8, SJA_X8664_DI)

#define R8B SJA_X8664_OREG(1, SJA_X8664_R8)
#define R8W SJA_X8664_OREG(2, SJA_X8664_R8)
#define R8D SJA_X8664_OREG(4, SJA_X8664_R8)
#define R8 SJA_X8664_OREG(8, SJA_X8664_R8)
#define R9B SJA_X8664_OREG(1, SJA_X8664_R9)
#define R9W SJA_X8664_OREG(2, SJA_X8664_R9)
#define R9D SJA_X8664_OREG(4, SJA_X8664_R9)
#define R9 SJA_X8664_OREG(8, SJA_X8664_R9)
#define R10B SJA_X8664_OREG(1, SJA_X8664_R10)
#define R10W SJA_X8664_OREG(2, SJA_X8664_R10)
#define R10D SJA_X8664_OREG(4, SJA_X8664_R10)
#define R10 SJA_X8664_OREG(8, SJA_X8664_R10)
#define R11B SJA_X8664_OREG(1, SJA_X8664_R11)
#define R11W SJA_X8664_OREG(2, SJA_X8664_R11)
#define R11D SJA_X8664_OREG(4, SJA_X8664_R11)
#define R11 SJA_X8664_OREG(8, SJA_X8664_R11)
#define R12B SJA_X8664_OREG(1, SJA_X8664_R12)
#define R12W SJA_X8664_OREG(2, SJA_X8664_R12)
#define R12D SJA_X8664_OREG(4, SJA_X8664_R12)
#define R12 SJA_X8664_OREG(8, SJA_X8664_R12)
#define R13B SJA_X8664_OREG(1, SJA_X8664_R13)
#define R13W SJA_X8664_OREG(2, SJA_X8664_R13)
#define R13D SJA_X8664_OREG(4, SJA_X8664_R13)
#define R13 SJA_X8664_OREG(8, SJA_X8664_R13)
#define R14B SJA_X8664_OREG(1, SJA_X8664_R14)
#define R14W SJA_X8664_OREG(2, SJA_X8664_R14)
#define R14D SJA_X8664_OREG(4, SJA_X8664_R14)
#define R14 SJA_X8664_OREG(8, SJA_X8664_R14)
#define R15B SJA_X8664_OREG(1, SJA_X8664_R15)
#define R15W SJA_X8664_OREG(2, SJA_X8664_R15)
#define R15D SJA_X8664_OREG(4, SJA_X8664_R15)
#define R15 SJA_X8664_OREG(8, SJA_X8664_R15)

#define RIP SJA_X8664_OREG(8, SJA_X8664_RIP)

#endif /* USE_SJA_X8664_SHORT_NAMES */

/* encodings for MEM (SIB) operands */
#define SJA_X8664_OMEM(msz, basesz, base, scale, indexsz, index, disp) \
    ((struct SJA_X8664_Operand) { SJA_X8664_OTYPE_MEM, (msz), (scale), \
        (struct SJA_X8664_Register) {(indexsz), (index)}, \
        (struct SJA_X8664_Register) {(basesz), (base)}, \
        (disp) \
    })
#ifdef USE_SJA_X8664_SHORT_NAMES
#define MEM(msz, base, scale, index, disp) \
    ((struct SJA_X8664_Operand) { SJA_X8664_OTYPE_MEM, msz, (scale), \
        (index).reg, \
        (base).reg, \
        (disp) \
    })
#endif

/* now on to the actual instructions. For each instruction, we define all of
 * its possible encodings, in terms of <= 3 operands, which are described in
 * terms of type and size, then a sequence of steps which build the
 * instruction. Steps IMM-MRM7 must the be followed by the operand (0-2)
 * encoded by that step, and MRMR must be followed by both operands.
 * */
enum sja_x8664_encoding_step {
    SJA_X8664_ES_END,
    SJA_X8664_ES_FIX, /* fixed value */
    SJA_X8664_ES_ADDREG, /* add the register to the opcode */
    SJA_X8664_ES_IMM8,
    SJA_X8664_ES_IMM16,
    SJA_X8664_ES_IMM32,
    SJA_X8664_ES_RREL8,
    SJA_X8664_ES_RREL16,
    SJA_X8664_ES_RREL32,
    SJA_X8664_ES_FREL32,
    SJA_X8664_ES_MRM0,
    SJA_X8664_ES_MRM1,
    SJA_X8664_ES_MRM2,
    SJA_X8664_ES_MRM3,
    SJA_X8664_ES_MRM4,
    SJA_X8664_ES_MRM5,
    SJA_X8664_ES_MRM6,
    SJA_X8664_ES_MRM7,
    SJA_X8664_ES_MRMR
};

/* a single instruction encoding */
struct SJA_X8664_Encoding {
    unsigned char otypes[3];
    unsigned char osz[3];
    unsigned char opcode;
    unsigned char *steps;
};

/* an instruction */
struct SJA_X8664_Instruction {
    size_t encodingCt;
    struct SJA_X8664_Encoding *encodings;
};

/* the instructions supported by this system */
#define INST(x) extern struct SJA_X8664_Instruction sja_x8664_inst_ ## x;
#include "x8664-instructions.h"
#undef INST
#ifdef USE_SJA_X8664_SHORT_NAMES
#define INST(x) static struct SJA_X8664_Instruction * const x = &(sja_x8664_inst_ ## x);
#include "x8664-instructions.h"
#undef INST
#endif

/* and finally, an actual operation */
struct SJA_X8664_Operation {
    struct SJA_X8664_Instruction *inst;
    struct SJA_X8664_Operand o[3];
};

#define SJA_X8664_OP3(inst, o1, o2, o3) \
    ((struct SJA_X8664_Operation) { (inst), {(o1), (o2), (o3)} })
#define SJA_X8664_OP2(inst, o1, o2) \
    SJA_X8664_OP3(inst, o1, o2, SJA_X8664_ONONE)
#define SJA_X8664_OP1(inst, o1) \
    SJA_X8664_OP3(inst, o1, SJA_X8664_ONONE, SJA_X8664_ONONE)
#define SJA_X8664_OP0(inst) \
    SJA_X8664_OP3(inst, SJA_X8664_ONONE, SJA_X8664_ONONE, SJA_X8664_ONONE)
#ifdef USE_SJA_X8664_SHORT_NAMES
#define OP3 SJA_X8664_OP3
#define OP2 SJA_X8664_OP2
#define OP1 SJA_X8664_OP1
#define OP0 SJA_X8664_OP0
#endif

/* the generic form of operation */
#define SJA_Operation SJA_X8664_Operation

#endif
