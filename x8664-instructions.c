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

/* ADD */
INST(ADD, (IEA {
    ENC(OTRM, 1, OT(IMM), 1, 0, 0,
        0x80,
        (ESA {ES(MRM0), 0, ES(IMM8), 1, ES(END)})),
    ENC(OTRM, 2, OT(IMM), 2, 0, 0,
        0x81,
        (ESA {ES(MRM0), 0, ES(IMM16), 1, ES(END)})),
    ENC(OTRM, D2Q, OT(IMM), 4, 0, 0,
        0x81,
        (ESA {ES(MRM0), 0, ES(IMM32), 1, ES(END)})),
    ENC(OTRM, W2Q, OT(IMM), 1, 0, 0,
        0x83,
        (ESA {ES(MRM0), 0, ES(IMM8), 1, ES(END)})),
    ENC(OTRM, 1, OT(REG), 1, 0, 0,
        0x00,
        (ESA {ES(MRMR), 1, 0, ES(END)})),
    ENC(OTRM, W2Q, OT(REG), W2Q, 0, 0,
        0x01,
        (ESA {ES(MRMR), 1, 0, ES(END)})),
    ENC(OT(REG), 1, OTRM, 1, 0, 0,
        0x02,
        (ESA {ES(MRMR), 0, 1, ES(END)})),
    ENC(OT(REG), W2Q, OTRM, W2Q, 0, 0,
        0x03,
        (ESA {ES(MRMR), 0, 1, ES(END)}))
}));

/* SUB */
INST(SUB, (IEA {
    ENC(OTRM, 1, OT(IMM), 1, 0, 0,
        0x80,
        (ESA {ES(MRM5), 0, ES(IMM8), 1, ES(END)})),
    ENC(OTRM, 2, OT(IMM), 2, 0, 0,
        0x81,
        (ESA {ES(MRM5), 0, ES(IMM16), 1, ES(END)})),
    ENC(OTRM, D2Q, OT(IMM), 4, 0, 0,
        0x81,
        (ESA {ES(MRM5), 0, ES(IMM32), 1, ES(END)})),
    ENC(OTRM, W2Q, OT(IMM), 1, 0, 0,
        0x83,
        (ESA {ES(MRM5), 0, ES(IMM8), 1, ES(END)})),
    ENC(OTRM, 1, OT(REG), 1, 0, 0,
        0x28,
        (ESA {ES(MRMR), 1, 0, ES(END)})),
    ENC(OTRM, W2Q, OT(REG), W2Q, 0, 0,
        0x29,
        (ESA {ES(MRMR), 1, 0, ES(END)})),
    ENC(OT(REG), 1, OTRM, 1, 0, 0,
        0x2A,
        (ESA {ES(MRMR), 0, 1, ES(END)})),
    ENC(OT(REG), W2Q, OTRM, W2Q, 0, 0,
        0x2B,
        (ESA {ES(MRMR), 0, 1, ES(END)}))
}));
