/*
 * SJA: x86_64 instruction mnemonics.
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

#include "sja/x8664.h"

#define INST(n, e) \
static struct SJA_X8664_Encoding n ## _encodings[] = e; \
struct SJA_X8664_Instruction sja_x8664_inst_ ## n = { \
    sizeof(n ## _encodings) / sizeof(struct SJA_X8664_Encoding), \
    n ## _encodings \
}
#define IEA (struct SJA_X8664_Encoding[])
#define ENC(ot1, os1, ot2, os2, ot3, os3, opc, es) \
{ {(ot1), (ot2), (ot3)}, {(os1), (os2), (os3)}, (opc), (es) }
#define OT(x) SJA_X8664_OTYPE_ ## x
#define OTRM (OT(REG)|OT(MEM))
#define W2Q (2|4|8)
#define D2Q (4|8)
#define ESA (unsigned char[])
#define ES(x) SJA_X8664_ES_ ## x

/* many of the ALU instructions have the same suite of formats, so they can be
 * abbreviated */
#define ALU(rm8i8o, rm8i8r, \
            rm16i16o, rm16i16r, \
            rm64i32o, rm64i32r, \
            rm64i8o, rm64i8r, \
            rm8r8, \
            rm64r64, \
            r8rm8, \
            r64rm64) \
    (IEA { \
        ENC(OTRM, 1, OT(IMM), 1, 0, 0, rm8i8o, \
            (ESA {ES(rm8i8r), 0, ES(IMM8), 1, ES(END)})), \
        ENC(OTRM, 2, OT(IMM), 2, 0, 0, rm16i16o, \
            (ESA {ES(rm16i16r), 0, ES(IMM16), 1, ES(END)})), \
        ENC(OTRM, D2Q, OT(IMM), 4, 0, 0, rm64i32o, \
            (ESA {ES(rm64i32r), 0, ES(IMM32), 1, ES(END)})), \
        ENC(OTRM, W2Q, OT(IMM), 1, 0, 0, rm64i8o, \
            (ESA {ES(rm64i8r), 0, ES(IMM8), 1, ES(END)})), \
        ENC(OTRM, 1, OT(REG), 1, 0, 0, rm8r8, \
            (ESA {ES(MRMR), 1, 0, ES(END)})), \
        ENC(OTRM, W2Q, OT(REG), W2Q, 0, 0, rm64r64, \
            (ESA {ES(MRMR), 1, 0, ES(END)})), \
        ENC(OT(REG), 1, OTRM, 1, 0, 0, r8rm8, \
            (ESA {ES(MRMR), 0, 1, ES(END)})), \
        ENC(OT(REG), W2Q, OTRM, W2Q, 0, 0, r64rm64, \
            (ESA {ES(MRMR), 0, 1, ES(END)})) \
    })

/* unary operations tend to have a familiar form too */
#define UNARY(rm8o, rm8r, rm64o, rm64r) \
    (IEA { \
        ENC(OTRM, 1, 0, 0, 0, 0, rm8o, \
            (ESA {ES(rm8r), 0, ES(END)})), \
        ENC(OTRM, W2Q, 0, 0, 0, 0, rm64o, \
            (ESA {ES(rm64r), 0, ES(END)})) \
    })

/* the MUL/DIV and family instructions also have a fixed format */
#define MULDIV(r8o, r8r, r64o, r64r, mul) \
    (IEA { \
        ENC(OTRM, 1, 0, 0, 0, 0, r8o, \
            (ESA {ES(r8r), 0, ES(END)})), \
        ENC(OTRM, W2Q, 0, 0, 0, 0, r64o, \
            (ESA {ES(r64r), 0, ES(END)})) \
        mul \
    })

/* the MUL instructions extend it */
#define MUL(r64rm64o1, r64rm64o2, \
            r64rm64i8, \
            r16rm16i16, \
            r64rm64i32) , \
        ENC(OT(REG), W2Q, OTRM, W2Q, 0, 0, r64rm64o1, \
            (ESA {ES(FIX), r64rm64o2, ES(MRMR), 0, 1, ES(END)})), \
        ENC(OT(REG), W2Q, OTRM, W2Q, OT(IMM), 1, r64rm64i8, \
            (ESA {ES(MRMR), 0, 1, ES(IMM8), 2, ES(END)})), \
        ENC(OT(REG), 2, OTRM, 2, OT(IMM), 2, r16rm16i16, \
            (ESA {ES(MRMR), 0, 1, ES(IMM16), 2, ES(END)})), \
        ENC(OT(REG), D2Q, OTRM, D2Q, OT(IMM), 4, r64rm64i32, \
            (ESA {ES(MRMR), 0, 1, ES(IMM32), 2, ES(END)}))

/* same for shifts */
#define SHIFT(r8i8o, r8i8r, r64i8o, r64i8r) \
    (IEA { \
        ENC(OTRM, 1, OT(IMM), 1, 0, 0, r8i8o, \
            (ESA {ES(r8i8r), 0, ES(IMM8), 1, ES(END)})), \
        ENC(OTRM, W2Q, OT(IMM), 1, 0, 0, r64i8o, \
            (ESA {ES(r64i8r), 0, ES(IMM8), 1, ES(END)})) \
    })

/* jumps are all simple */
#define JxxR(o8, o32) \
    (IEA { \
        ENC(OT(RREL), 1, 0, 0, 0, 0, o8, \
            (ESA {ES(RREL8), 0, ES(END)})), \
        ENC(OT(RREL), 2|4, 0, 0, 0, 0, 0x0F, \
            (ESA {ES(FIX), o32, ES(RREL32), 0, ES(END)})) \
    })
#define JxxF(o32) \
    (IEA { \
        ENC(0, 0, 0, 0, 0, 0, 0x0F, \
            (ESA {ES(FIX), o32, ES(FREL32), ES(END)})) \
    })

#define BLANK

/***************************************************************
 * ACTUAL INSTRUCTION FORMATS BELOW
 ***************************************************************/

/* ADD */
INST(ADD, ALU(
    0x80, MRM0,
    0x81, MRM0,
    0x81, MRM0,
    0x83, MRM0,
    0x00, 0x01, 0x02, 0x03
));

/* AND */
INST(AND, ALU(
    0x80, MRM4,
    0x81, MRM4,
    0x81, MRM4,
    0x83, MRM4,
    0x20 ,0x21, 0x22, 0x23
));

/* CALL */
INST(CALL, (IEA {
    ENC(OT(RREL), 4, 0, 0, 0, 0, 0xE8,
        (ESA {ES(RREL32), 0, ES(END)})),
    ENC(OT(FREL), 4, 0, 0, 0, 0, 0xE8,
        (ESA {ES(FREL32), ES(END)})),
    ENC(OTRM, D2Q, 0, 0, 0, 0, 0xFF,
        (ESA {ES(MRM2), 0, ES(END)}))
}));

/* CMP */
INST(CMP, ALU(
    0x80, MRM7,
    0x81, MRM7,
    0x81, MRM7,
    0x83, MRM7,
    0x38, 0x39, 0x3A, 0x3B
));

/* DIV */
INST(DIV, MULDIV(0xF6, MRM6, 0xF7, MRM6, BLANK));

/* ENTER */
INST(ENTER, (IEA {
    ENC(OT(IMM), 2, OT(IMM), 1, 0, 0, 0xC8,
        (ESA {ES(IMM16), 0, ES(IMM8), 1, ES(END)}))
}));

/* IDIV */
INST(IDIV, MULDIV(0xF6, MRM7, 0xF7, MRM7, BLANK));

/* IMUL */
INST(IMUL, MULDIV(0xF6, MRM5, 0xF7, MRM5, MUL(
    0x0F, 0xAF,
    0x6B, 0x69, 0x69)));

/* Jxx */
#define JRF(nm, o) \
    INST(nm ## R, JxxR((0x70+o), (0x80+o))); \
    INST(nm ## F, JxxF((0x80+o)))
JRF(JO,     0x0);
JRF(JNO,    0x1);
JRF(JB,     0x2);
JRF(JC,     0x2);
JRF(JNAE,   0x2);
JRF(JNB,    0x3);
JRF(JNC,    0x3);
JRF(JAE,    0x3);
JRF(JZ,     0x4);
JRF(JE,     0x4);
JRF(JNZ,    0x5);
JRF(JNE,    0x5);
JRF(JBE,    0x6);
JRF(JNA,    0x6);
JRF(JNBE,   0x7);
JRF(JA,     0x7);
JRF(JS,     0x8);
JRF(JNS,    0x9);
JRF(JP,     0xA);
JRF(JPE,    0xA);
JRF(JNP,    0xB);
JRF(JPO,    0xB);
JRF(JL,     0xC);
JRF(JNGE,   0xC);
JRF(JNL,    0xD);
JRF(JGE,    0xD);
JRF(JLE,    0xE);
JRF(JNG,    0xE);
JRF(JNLE,   0xF);
JRF(JG,     0xF);
#undef JRF

/* JMP */
INST(JMPR, (IEA {
    ENC(OT(RREL), 1, 0, 0, 0, 0, 0xEB,
        (ESA {ES(RREL8), 0, ES(END)})),
    ENC(OT(RREL), 4, 0, 0, 0, 0, 0xE9,
        (ESA {ES(RREL32), 0, ES(END)})),
    ENC(OTRM, W2Q, 0, 0, 0, 0, 0xFF,
        (ESA {ES(MRM4), 0, ES(END)}))
}));
INST(JMPF, (IEA {
    ENC(0, 0, 0, 0, 0, 0, 0xE9,
        (ESA {ES(FREL32), ES(END)}))
}));

/* LEA */
INST(LEA, (IEA {
    ENC(OT(REG), W2Q, OT(MEM), W2Q, 0, 0, 0x8D,
        (ESA {ES(MRMR), 0, 1, ES(END)}))
}));

/* LEAVE */
INST(LEAVE, (IEA {
    ENC(0, 0, 0, 0, 0, 0, 0xC9,
        (ESA {ES(END)}))
}));

/* MOV */
INST(MOV, (IEA {
    ENC(OTRM, 1, OT(IMM), 1, 0, 0, 0xC6,
        (ESA {ES(MRM0), 0, ES(IMM8), 1, ES(END)})),
    ENC(OTRM, 2, OT(IMM), 2, 0, 0, 0xC7,
        (ESA {ES(MRM0), 0, ES(IMM16), 1, ES(END)})),
    ENC(OTRM, D2Q, OT(IMM), 4, 0, 0, 0xC7,
        (ESA {ES(MRM0), 0, ES(IMM32), 1, ES(END)})),
    ENC(OTRM, 1, OT(REG), 1, 0, 0, 0x88,
        (ESA {ES(MRMR), 1, 0, ES(END)})),
    ENC(OTRM, W2Q, OT(REG), W2Q, 0, 0, 0x89,
        (ESA {ES(MRMR), 1, 0, ES(END)})),
    ENC(OT(REG), 1, OTRM, 1, 0, 0, 0x8A,
        (ESA {ES(MRMR), 0, 1, ES(END)})),
    ENC(OT(REG), W2Q, OTRM, W2Q, 0, 0, 0x8B,
        (ESA {ES(MRMR), 0, 1, ES(END)}))
}));

/* MUL */
INST(MUL, MULDIV(0xF6, MRM4, 0xF7, MRM4, BLANK));

/* NEG */
INST(NEG, UNARY(0xF6, MRM3, 0xF7, MRM3));

/* NOP */
INST(NOP, (IEA {
    ENC(0, 0, 0, 0, 0, 0, 0x90,
        (ESA {ES(END)})),
    ENC(OTRM, W2Q, 0, 0, 0, 0, 0x0F,
        (ESA {ES(FIX), 0x1F, ES(MRM0), 0}))
}));

/* NOT */
INST(NOT, UNARY(0xF6, MRM2, 0xF7, MRM2));

/* OR */
INST(OR, ALU(
    0x80, MRM1,
    0x81, MRM1,
    0x81, MRM1,
    0x83, MRM1,
    0x08, 0x09, 0x0A, 0x0B
));

/* POP */
INST(POP, (IEA {
    ENC(OT(REG), W2Q, 0, 0, 0, 0, 0x58,
        (ESA {ES(ADDREG), 0, ES(END)})),
    ENC(OTRM, W2Q, 0, 0, 0, 0, 0x8F,
        (ESA {ES(MRM0), 0, ES(END)}))
}));

/* PUSH */
INST(PUSH, (IEA {
    ENC(OT(REG), W2Q, 0, 0, 0, 0, 0x50,
        (ESA {ES(ADDREG), 0, ES(END)})),
    ENC(OTRM, W2Q, 0, 0, 0, 0, 0xFF,
        (ESA {ES(MRM0), 0, ES(END)}))
}));

/* REP (actually a prefix) */
INST(REP, (IEA {
    ENC(0, 0, 0, 0, 0, 0, 0xF3,
        (ESA {ES(END)}))
}));

/* RET */
INST(RET, (IEA {
    ENC(0, 0, 0, 0, 0, 0, 0xC3,
        (ESA {ES(END)})),
    ENC(OT(IMM), 2, 0, 0, 0, 0, 0xC2,
        (ESA {ES(IMM16), 0, ES(END)}))
}));

/* SAL */
INST(SAL, SHIFT(0xC0, MRM4, 0xC1, MRM4));

/* SAR */
INST(SAR, SHIFT(0xC0, MRM7, 0xC1, MRM7));

/* SBB */
INST(SBB, ALU(
    0x80, MRM3,
    0x81, MRM3,
    0x81, MRM3,
    0x83, MRM3,
    0x18, 0x19, 0x1A, 0x1B
));

/* SUB */
INST(SUB, ALU(
    0x80, MRM5,
    0x81, MRM5,
    0x81, MRM5,
    0x83, MRM5,
    0x28, 0x29, 0x2A, 0x2B
));

/* SHL (same as SAL) */
INST(SHL, SHIFT(0xC0, MRM4, 0xC1, MRM4));

/* SHLD */
INST(SHLD, (IEA {
    ENC(OTRM, W2Q, OT(REG), W2Q, OT(IMM), 1, 0x0F,
        (ESA {ES(FIX), 0xA4, ES(MRMR), 1, 0, ES(IMM8), 2, ES(END)}))
}));

/* SHR */
INST(SHR, SHIFT(0xC0, MRM5, 0xC1, MRM5));

/* SHRD */
INST(SHRD, (IEA {
    ENC(OTRM, W2Q, OT(REG), W2Q, OT(IMM), 1, 0x0F,
        (ESA {ES(FIX), 0xAC, ES(MRMR), 1, 0, ES(IMM8), 2, ES(END)}))
}));

/* STD */
INST(STD, (IEA {
    ENC(0, 0, 0, 0, 0, 0, 0xFD,
        (ESA {ES(END)}))
}));

/* STOS */
INST(STOS, (IEA {
    ENC(OTRM, 1, 0, 0, 0, 0, 0xAA,
        (ESA {ES(END)})),
    ENC(OTRM, W2Q, 0, 0, 0, 0, 0xAB,
        (ESA {ES(END)}))
}));

/* TEST */
INST(TEST, (IEA {
    ENC(OTRM, 1, OT(IMM), 1, 0, 0, 0xF6,
        (ESA {ES(MRM0), 0, ES(IMM8), 1, ES(END)})),
    ENC(OTRM, 2, OT(IMM), 2, 0, 0, 0xF7,
        (ESA {ES(MRM0), 0, ES(IMM16), 1, ES(END)})),
    ENC(OTRM, W2Q, OT(IMM), 4, 0, 0, 0xF7,
        (ESA {ES(MRM0), 0, ES(IMM32), 1, ES(END)})),
    ENC(OTRM, 1, OT(REG), 1, 0, 0, 0x84,
        (ESA {ES(MRMR), 0, 1, ES(END)})),
    ENC(OTRM, D2Q, OT(REG), D2Q, 0, 0, 0x85,
        (ESA {ES(MRMR), 0, 1, ES(END)}))
}));

/* XOR */
INST(XOR, ALU(
    0x80, MRM6,
    0x81, MRM6,
    0x81, MRM6,
    0x83, MRM6,
    0x30, 0x31, 0x32, 0x33
));
