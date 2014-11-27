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

/* ADD */
INST(ADD, ALU(
    0x80, MRM0,
    0x81, MRM0,
    0x81, MRM0,
    0x83, MRM0,
    0x00, 0x01, 0x02, 0x03
));

/* SUB */
INST(SUB, ALU(
    0x80, MRM5,
    0x81, MRM5,
    0x81, MRM5,
    0x83, MRM5,
    0x28, 0x29, 0x2A, 0x2B
));
