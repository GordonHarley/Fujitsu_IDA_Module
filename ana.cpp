
#include "fr.hpp"

// distinct sizes :
//lint -esym(749, S_11) not referenced
enum
{
    S_0,
    S_4,        // 4 bits
    S_5,        // 5 bits
    S_8,        // 8 bits
    S_11,       // 11 bits
    S_12,       // 12 bits
    S_16        // 16 bits
};

// bits numbers for sizes :
const int bits[] =
{
  0,
  4,
  5,
  8,
  11,
  12,
  16
};

// masks for sizes :
const int masks[] =
{
  0x0000,
  0x000F,
  0x001F,
  0x00FF,
  0x07FF,
  0x0FFF,
  0xFFFF
};

const char dtypes[] =
{
  0,
  dt_byte,
  dt_byte,
  dt_byte,
  dt_word,
  dt_word,
  dt_word
};

// distinct operands :
enum
{
    O_null,         // null opcode
    O_gr,           // general register                         Ri
    O_gri,          // general register indirect                @Ri
    O_grip,         // general register indirect post-increment @Ri+
    O_r13_gr_i,     // indirect r13 + general register          @(R13, Ri)
    O_r14_imm8_i,   // indirect r14 + 8 bits immediate value    @(R14, imm)
    O_r15_imm4_i,   // indirect r15 + 4 bits immediate value    @(R15, imm)
    O_r15ip,        // indirect r15 post-increment              @R15+
    O_r15im,        // indirect r15 pre-decrement               @-R15
    O_r13,          // register r13                             R13
    O_r13ip,        // indirect r13 post-increment              @R13+
    O_dr,           // dedicated register                       Rs
    O_ps,           // program status register (PS)             PS
    O_imm,          // immediate value                          #i
    O_diri,         // indirect direct value                    @i
    O_rel,          // relative value                           label5
    O_reglist,      // register list                            (R0, R1, R2, ...)
    O_ccr,          // Condition code register / part of        PS,
    O_tbr,          // The TBR
};

static int invert_word(int word) {
    int new_word = 0;

    new_word |= (word & 0x000F) >> 0;
    new_word <<= 4;
    new_word |= (word & 0x00F0) >> 4;
    new_word <<= 4;
    new_word |= (word & 0x0F00) >> 8;
    new_word <<= 4;
    new_word |= (word & 0xF000) >> 12;

    return new_word;
}

// structure of an opcode :
struct opcode_t
{
  int insn;
  int opcode;
  int opcode_size;

  int op1;
  int op1_size;
  int op2;
  int op2_size;

  /// op3 is an implicit op and not displayed.  
  /// It is used for instructions which directly read the PS register 
  /// (and could be used for default base index registers that won't
  /// fit in a normal description). 
  int op3;

#define I_SWAPOPS          0x00000100      // swap operands
#define I_DSHOT            0x00000200      // delay shot
#define I_ADDR_R           OP_ADDR_R
#define I_ADDR_W           OP_ADDR_W
#define I_IMM_2            0x00001000      // imm = imm * 2
#define I_IMM_4            0x00002000      // imm = imm * 4
#define I_IMM_16           0x00004000      // imm = imm + 16
#define I_MEM_8            0x00010000      // load/store 8 bytes to/from memptr
#define I_MEM_16           0x00020000      // load/store 16 bytes to/from memptr
#define I_MEM_32           0x00040000      // load/store 32 bytes to/from memptr
#define I_BAD_DELAY        0x00100000      // instruction that cannot be in delay slot

  int flags;

  inline bool swap_ops(void) const { return (flags & I_SWAPOPS) != 0; }
  inline bool delay_shot(void) const { return (flags & I_DSHOT) != 0; }
  inline bool bad_delay(void) const { return (flags & I_BAD_DELAY) != 0; }
  inline bool implied(void) const { return op1 == O_null && op2 == O_null; }

  int size(void) const
  {
    int n = bits[opcode_size];
    if ( op1 != O_null )   n += bits[op1_size];
    if ( op2 != O_null )   n += bits[op2_size];
    return n;
  }

  static void check(void);
  static const struct opcode_t * find(int *_data);
};

// FR opcodes :
static const struct opcode_t opcodes[] =
{
  { fr_add,       0xA6,       S_8,    O_gr,           S_4,    O_gr,       S_4,    O_null,    0         },
  { fr_add,       0xA4,       S_8,    O_imm,          S_4,    O_gr,       S_4,    O_null,    0         },
  { fr_add2,      0xA5,       S_8,    O_imm,          S_4,    O_gr,       S_4,    O_null,    0         },
  { fr_addc,      0xA7,       S_8,    O_gr,           S_4,    O_gr,       S_4,    O_ps,      0         },
  { fr_addn,      0xA2,       S_8,    O_gr,           S_4,    O_gr,       S_4,    O_null,    0         },
  { fr_addn,      0xA0,       S_8,    O_imm,          S_4,    O_gr,       S_4,    O_null,    0         },
  { fr_addn2,     0xA1,       S_8,    O_imm,          S_4,    O_gr,       S_4,    O_null,    0         },
  { fr_sub,       0xAC,       S_8,    O_gr,           S_4,    O_gr,       S_4,    O_null,    0         },
  { fr_subc,      0xAD,       S_8,    O_gr,           S_4,    O_gr,       S_4,    O_ps,      0         },
  { fr_subn,      0xAE,       S_8,    O_gr,           S_4,    O_gr,       S_4,    O_null,    0         },
  { fr_cmp,       0xAA,       S_8,    O_gr,           S_4,    O_gr,       S_4,    O_null,    0         },
  { fr_cmp,       0xA8,       S_8,    O_imm,          S_4,    O_gr,       S_4,    O_null,    0         },
  { fr_cmp2,      0xA9,       S_8,    O_imm,          S_4,    O_gr,       S_4,    O_null,    0         },
  { fr_and,       0x82,       S_8,    O_gr,           S_4,    O_gr,       S_4,    O_null,    0         },
  { fr_and,       0x84,       S_8,    O_gr,           S_4,    O_gri,      S_4,    O_null,    I_BAD_DELAY },
  { fr_andh,      0x85,       S_8,    O_gr,           S_4,    O_gri,      S_4,    O_null,    I_BAD_DELAY },
  { fr_andb,      0x86,       S_8,    O_gr,           S_4,    O_gri,      S_4,    O_null,    I_BAD_DELAY },
  { fr_or,        0x92,       S_8,    O_gr,           S_4,    O_gr,       S_4,    O_null,    0         },
  { fr_or,        0x94,       S_8,    O_gr,           S_4,    O_gri,      S_4,    O_null,    I_BAD_DELAY },
  { fr_orh,       0x95,       S_8,    O_gr,           S_4,    O_gri,      S_4,    O_null,    I_BAD_DELAY },
  { fr_orb,       0x96,       S_8,    O_gr,           S_4,    O_gri,      S_4,    O_null,    I_BAD_DELAY },
  { fr_eor,       0x9A,       S_8,    O_gr,           S_4,    O_gr,       S_4,    O_null,    0         },
  { fr_eor,       0x9C,       S_8,    O_gr,           S_4,    O_gri,      S_4,    O_null,    I_BAD_DELAY },
  { fr_eorh,      0x9D,       S_8,    O_gr,           S_4,    O_gri,      S_4,    O_null,    I_BAD_DELAY },
  { fr_eorb,      0x9E,       S_8,    O_gr,           S_4,    O_gri,      S_4,    O_null,    I_BAD_DELAY },
  { fr_bandl,     0x80,       S_8,    O_imm,          S_4,    O_gri,      S_4,    O_null,    I_BAD_DELAY },
  { fr_bandh,     0x81,       S_8,    O_imm,          S_4,    O_gri,      S_4,    O_null,    I_BAD_DELAY },
  { fr_borl,      0x90,       S_8,    O_imm,          S_4,    O_gri,      S_4,    O_null,    I_BAD_DELAY },
  { fr_borh,      0x91,       S_8,    O_imm,          S_4,    O_gri,      S_4,    O_null,    I_BAD_DELAY },
  { fr_beorl,     0x98,       S_8,    O_imm,          S_4,    O_gri,      S_4,    O_null,    I_BAD_DELAY },
  { fr_beorh,     0x99,       S_8,    O_imm,          S_4,    O_gri,      S_4,    O_null,    I_BAD_DELAY },
  { fr_btstl,     0x88,       S_8,    O_imm,          S_4,    O_gri,      S_4,    O_null,    I_BAD_DELAY },
  { fr_btsth,     0x89,       S_8,    O_imm,          S_4,    O_gri,      S_4,    O_null,    I_BAD_DELAY },
  { fr_mul,       0xAF,       S_8,    O_gr,           S_4,    O_gr,       S_4,    O_null,    I_BAD_DELAY },
  { fr_mulu,      0xAB,       S_8,    O_gr,           S_4,    O_gr,       S_4,    O_null,    I_BAD_DELAY },
  { fr_mulh,      0xBF,       S_8,    O_gr,           S_4,    O_gr,       S_4,    O_null,    I_BAD_DELAY },
  { fr_muluh,     0xBB,       S_8,    O_gr,           S_4,    O_gr,       S_4,    O_null,    I_BAD_DELAY },
  { fr_div0s,     0x974,      S_12,   O_gr,           S_4,    O_null,     0,      O_null,    0 },
  { fr_div0u,     0x975,      S_12,   O_gr,           S_4,    O_null,     0,      O_null,    0 },
  { fr_div1,      0x976,      S_12,   O_gr,           S_4,    O_null,     0,      O_null,    0 },
  { fr_div2,      0x977,      S_12,   O_gr,           S_4,    O_null,     0,      O_null,    0 },
  { fr_div3,      0x9F60,     S_16,   O_null,         0,      O_null,     0,      O_null,    0         },
  { fr_div4s,     0x9F70,     S_16,   O_null,         0,      O_null,     0,      O_null,    0         },
  { fr_lsl,       0xB6,       S_8,    O_gr,           S_4,    O_gr,       S_4,    O_null,    0         },
  { fr_lsl,       0xB4,       S_8,    O_imm,          S_4,    O_gr,       S_4,    O_null,    0         },
  { fr_lsl2,      0xB5,       S_8,    O_imm,          S_4,    O_gr,       S_4,    O_null,    I_IMM_16  },
  { fr_lsr,       0xB2,       S_8,    O_gr,           S_4,    O_gr,       S_4,    O_null,    0         },
  { fr_lsr,       0xB0,       S_8,    O_imm,          S_4,    O_gr,       S_4,    O_null,    0         },
  { fr_lsr2,      0xB1,       S_8,    O_imm,          S_4,    O_gr,       S_4,    O_null,    I_IMM_16  },
  { fr_asr,       0xBA,       S_8,    O_gr,           S_4,    O_gr,       S_4,    O_null,    0         },
  { fr_asr,       0xB8,       S_8,    O_imm,          S_4,    O_gr,       S_4,    O_null,    0         },
  { fr_asr2,      0xB9,       S_8,    O_imm,          S_4,    O_gr,       S_4,    O_null,    I_IMM_16  },
  // fr_ldi_32 not here (considered as special)                                   O_null,   
  // fr_ldi_20 not here (considered as special)                                   O_null,   
  { fr_ldi_8,     0x0C,       S_4,    O_imm,          S_8,    O_gr,       S_4,    O_null,    0         },
  { fr_ld,        0x04,       S_8,    O_gri,          S_4,    O_gr,       S_4,    O_null,    0         },
  { fr_ld,        0x00,       S_8,    O_r13_gr_i,     S_4,    O_gr,       S_4,    O_null,    0         },
  { fr_ld,        0x02,       S_4,    O_r14_imm8_i,   S_8,    O_gr,       S_4,    O_null,    I_IMM_4   },
  { fr_ld,        0x03,       S_8,    O_r15_imm4_i,   S_4,    O_gr,       S_4,    O_null,    I_IMM_4   },
  { fr_ld,        0x70,       S_12,   O_r15ip,        S_0,    O_gr,       S_4,    O_null,    0         },
  { fr_ld,        0x78,       S_12,   O_r15ip,        S_0,    O_dr,       S_4,    O_null,    0         },
  { fr_ld,        0x790,      S_16,   O_r15ip,        S_0,    O_ps,       S_0,    O_null,    I_BAD_DELAY },
  { fr_lduh,      0x05,       S_8,    O_gri,          S_4,    O_gr,       S_4,    O_null,    I_MEM_16  },
  { fr_lduh,      0x01,       S_8,    O_r13_gr_i,     S_4,    O_gr,       S_4,    O_null,    I_MEM_16  },
  { fr_lduh,      0x04,       S_4,    O_r14_imm8_i,   S_8,    O_gr,       S_4,    O_null,    I_MEM_16|I_IMM_2   },
  { fr_ldub,      0x06,       S_8,    O_gri,          S_4,    O_gr,       S_4,    O_null,    I_MEM_8   },
  { fr_ldub,      0x02,       S_8,    O_r13_gr_i,     S_4,    O_gr,       S_4,    O_null,    I_MEM_8   },
  { fr_ldub,      0x06,       S_4,    O_r14_imm8_i,   S_8,    O_gr,       S_4,    O_null,    I_MEM_8   },
  // Begin FR80 / 81
  { fr_srch0,     0x97C,      S_12,   O_gr,           S_4,    O_null,     0,      O_null,    0         },
  { fr_srch1,     0x97D,      S_12,   O_gr,           S_4,    O_null,     0,      O_null,    0         },
  { fr_srchc,     0x97E,      S_12,   O_gr,           S_4,    O_null,     0,      O_null,    0         },
  // End FR80 / 81
  { fr_st,        0x14,       S_8,    O_gri,          S_4,    O_gr,       S_4,    O_null,    I_SWAPOPS },
  { fr_st,        0x10,       S_8,    O_r13_gr_i,     S_4,    O_gr,       S_4,    O_null,    I_SWAPOPS },
  { fr_st,        0x03,       S_4,    O_r14_imm8_i,   S_8,    O_gr,       S_4,    O_null,    I_SWAPOPS|I_IMM_4 },
  { fr_st,        0x13,       S_8,    O_r15_imm4_i,   S_4,    O_gr,       S_4,    O_null,    I_SWAPOPS|I_IMM_4 },
  { fr_st,        0x170,      S_12,   O_gr,           S_4,    O_r15im,    S_0,    O_null,    0         },
  { fr_st,        0x178,      S_12,   O_dr,           S_4,    O_r15im,    S_0,    O_null,    0         },
  { fr_st,        0x1790,     S_16,   O_ps,           S_0,    O_r15im,    S_0,    O_null,    0         },
  { fr_sth,       0x15,       S_8,    O_gri,          S_4,    O_gr,       S_4,    O_null,    I_MEM_16|I_SWAPOPS },
  { fr_sth,       0x11,       S_8,    O_r13_gr_i,     S_4,    O_gr,       S_4,    O_null,    I_MEM_16|I_SWAPOPS },
  { fr_sth,       0x05,       S_4,    O_r14_imm8_i,   S_8,    O_gr,       S_4,    O_null,    I_MEM_16|I_SWAPOPS|I_IMM_2 },
  { fr_stb,       0x16,       S_8,    O_gri,          S_4,    O_gr,       S_4,    O_null,    I_MEM_8|I_SWAPOPS },
  { fr_stb,       0x12,       S_8,    O_r13_gr_i,     S_4,    O_gr,       S_4,    O_null,    I_MEM_8|I_SWAPOPS },
  { fr_stb,       0x07,       S_4,    O_r14_imm8_i,   S_8,    O_gr,       S_4,    O_null,    I_MEM_8|I_SWAPOPS },
  { fr_mov,       0x8B,       S_8,    O_gr,           S_4,    O_gr,       S_4,    O_null,    0         },
  { fr_mov,       0xB7,       S_8,    O_dr,           S_4,    O_gr,       S_4,    O_null,    0         },
  { fr_mov,       0x171,      S_12,   O_ps,           S_0,    O_gr,       S_4,    O_null,    0         },
  { fr_mov,       0xB3,       S_8,    O_dr,           S_4,    O_gr,       S_4,    O_null,    I_SWAPOPS },
  { fr_mov,       0x71,       S_12,   O_gr,           S_4,    O_ps,       S_0,    O_null,    0         },
  { fr_jmp,       0x970,      S_12,   O_gri,          S_4,    O_null,     0,      O_null,    I_BAD_DELAY },
  { fr_call,      0x971,      S_12,   O_gri,          S_4,    O_null,     0,      O_null,    I_BAD_DELAY },
  { fr_ret,       0x9720,     S_16,   O_null,         0,      O_null,     0,      O_null,    I_BAD_DELAY },
  { fr_int,       0x1F,       S_8,    O_imm,          S_8,    O_imm,      S_0,    O_tbr,     I_BAD_DELAY },
  { fr_inte,      0x9F30,     S_16,   O_null,         0,      O_null,     0,      O_null,    I_BAD_DELAY },
  { fr_reti,      0x9730,     S_16,   O_null,         0,      O_null,     0,      O_null,    I_BAD_DELAY },
  { fr_bra,       0xE0,       S_8,    O_rel,          S_8,    O_null,     0,      O_null,    I_BAD_DELAY },
  { fr_bno,       0xE1,       S_8,    O_rel,          S_8,    O_null,     0,      O_ps,      I_BAD_DELAY },
  { fr_beq,       0xE2,       S_8,    O_rel,          S_8,    O_null,     0,      O_ps,      I_BAD_DELAY },
  { fr_bne,       0xE3,       S_8,    O_rel,          S_8,    O_null,     0,      O_ps,      I_BAD_DELAY },
  { fr_bc,        0xE4,       S_8,    O_rel,          S_8,    O_null,     0,      O_ps,      I_BAD_DELAY },
  { fr_bnc,       0xE5,       S_8,    O_rel,          S_8,    O_null,     0,      O_ps,      I_BAD_DELAY },
  { fr_bn,        0xE6,       S_8,    O_rel,          S_8,    O_null,     0,      O_ps,      I_BAD_DELAY },
  { fr_bp,        0xE7,       S_8,    O_rel,          S_8,    O_null,     0,      O_ps,      I_BAD_DELAY },
  { fr_bv,        0xE8,       S_8,    O_rel,          S_8,    O_null,     0,      O_ps,      I_BAD_DELAY },
  { fr_bnv,       0xE9,       S_8,    O_rel,          S_8,    O_null,     0,      O_ps,      I_BAD_DELAY },
  { fr_blt,       0xEA,       S_8,    O_rel,          S_8,    O_null,     0,      O_ps,      I_BAD_DELAY },
  { fr_bge,       0xEB,       S_8,    O_rel,          S_8,    O_null,     0,      O_ps,      I_BAD_DELAY },
  { fr_ble,       0xEC,       S_8,    O_rel,          S_8,    O_null,     0,      O_ps,      I_BAD_DELAY },
  { fr_bgt,       0xED,       S_8,    O_rel,          S_8,    O_null,     0,      O_ps,      I_BAD_DELAY },
  { fr_bls,       0xEE,       S_8,    O_rel,          S_8,    O_null,     0,      O_ps,      I_BAD_DELAY },
  { fr_bhi,       0xEF,       S_8,    O_rel,          S_8,    O_null,     0,      O_ps,      I_BAD_DELAY },
  { fr_jmp,       0x9F0,      S_12,   O_gri,          S_4,    O_null,     0,      O_null,    I_DSHOT | I_BAD_DELAY   },
  { fr_call,      0x9F1,      S_12,   O_gri,          S_4,    O_null,     0,      O_null,    I_DSHOT | I_BAD_DELAY   },
  { fr_ret,       0x9F20,     S_16,   O_null,         0,      O_null,     0,      O_null,    I_DSHOT | I_BAD_DELAY   },
  { fr_bra,       0xF0,       S_8,    O_rel,          S_8,    O_null,     0,      O_null,    I_DSHOT | I_BAD_DELAY   },
  { fr_bno,       0xF1,       S_8,    O_rel,          S_8,    O_null,     0,      O_ps,      I_DSHOT | I_BAD_DELAY   },
  { fr_beq,       0xF2,       S_8,    O_rel,          S_8,    O_null,     0,      O_ps,      I_DSHOT | I_BAD_DELAY   },
  { fr_bne,       0xF3,       S_8,    O_rel,          S_8,    O_null,     0,      O_ps,      I_DSHOT | I_BAD_DELAY   },
  { fr_bc,        0xF4,       S_8,    O_rel,          S_8,    O_null,     0,      O_ps,      I_DSHOT | I_BAD_DELAY   },
  { fr_bnc,       0xF5,       S_8,    O_rel,          S_8,    O_null,     0,      O_ps,      I_DSHOT | I_BAD_DELAY   },
  { fr_bn,        0xF6,       S_8,    O_rel,          S_8,    O_null,     0,      O_ps,      I_DSHOT | I_BAD_DELAY   },
  { fr_bp,        0xF7,       S_8,    O_rel,          S_8,    O_null,     0,      O_ps,      I_DSHOT | I_BAD_DELAY   },
  { fr_bv,        0xF8,       S_8,    O_rel,          S_8,    O_null,     0,      O_ps,      I_DSHOT | I_BAD_DELAY   },
  { fr_bnv,       0xF9,       S_8,    O_rel,          S_8,    O_null,     0,      O_ps,      I_DSHOT | I_BAD_DELAY   },
  { fr_blt,       0xFA,       S_8,    O_rel,          S_8,    O_null,     0,      O_ps,      I_DSHOT | I_BAD_DELAY   },
  { fr_bge,       0xFB,       S_8,    O_rel,          S_8,    O_null,     0,      O_ps,      I_DSHOT | I_BAD_DELAY   },
  { fr_ble,       0xFC,       S_8,    O_rel,          S_8,    O_null,     0,      O_ps,      I_DSHOT | I_BAD_DELAY   },
  { fr_bgt,       0xFD,       S_8,    O_rel,          S_8,    O_null,     0,      O_ps,      I_DSHOT | I_BAD_DELAY   },
  { fr_bls,       0xFE,       S_8,    O_rel,          S_8,    O_null,     0,      O_ps,      I_DSHOT | I_BAD_DELAY   },
  { fr_bhi,       0xFF,       S_8,    O_rel,          S_8,    O_null,     0,      O_ps,      I_DSHOT | I_BAD_DELAY   },
  { fr_dmov,      0x08,       S_8,    O_diri,         S_8,    O_r13,      S_0,    O_null,    I_ADDR_R  },
  { fr_dmov,      0x18,       S_8,    O_r13,          S_0,    O_diri,     S_8,    O_null,    I_ADDR_W  },
  { fr_dmov,      0x0C,       S_8,    O_diri,         S_8,    O_r13ip,    S_0,    O_null,    I_ADDR_R | I_BAD_DELAY },
  { fr_dmov,      0x1C,       S_8,    O_r13ip,        S_0,    O_diri,     S_8,    O_null,    I_ADDR_W | I_BAD_DELAY },
  { fr_dmov,      0x0B,       S_8,    O_diri,         S_8,    O_r15im,    S_0,    O_null,    I_ADDR_R | I_BAD_DELAY },
  { fr_dmov,      0x1B,       S_8,    O_r15ip,        S_0,    O_diri,     S_8,    O_null,    I_ADDR_W | I_BAD_DELAY },
  { fr_dmovh,     0x09,       S_8,    O_diri,         S_8,    O_r13,      S_0,    O_null,    I_ADDR_R  },
  { fr_dmovh,     0x19,       S_8,    O_r13,          S_0,    O_diri,     S_8,    O_null,    I_ADDR_W  },
  { fr_dmovh,     0x0D,       S_8,    O_diri,         S_8,    O_r13ip,    S_0,    O_null,    I_ADDR_R | I_BAD_DELAY },
  { fr_dmovh,     0x1D,       S_8,    O_r13ip,        S_0,    O_diri,     S_8,    O_null,    I_ADDR_W | I_BAD_DELAY },
  { fr_dmovb,     0x0A,       S_8,    O_diri,         S_8,    O_r13,      S_0,    O_null,    I_ADDR_R  },
  { fr_dmovb,     0x1A,       S_8,    O_r13,          S_0,    O_diri,     S_8,    O_null,    I_ADDR_W  },
  { fr_dmovb,     0x0E,       S_8,    O_diri,         S_8,    O_r13ip,    S_0,    O_null,    I_ADDR_R | I_BAD_DELAY },
  { fr_dmovb,     0x1E,       S_8,    O_r13ip,        S_0,    O_diri,     S_8,    O_null,    I_ADDR_W | I_BAD_DELAY },
  { fr_ldres,     0xBC,       S_8,    O_imm,          S_4,    O_grip,     S_4,    O_null,    I_SWAPOPS },
  { fr_stres,     0xBD,       S_8,    O_imm,          S_4,    O_grip,     S_4,    O_null,    0         },
  // fr_copop not here (considered as special)                                    O_null,   
  // fr_copld not here (considered as special)                                    O_null,   
  // fr_copst not here (considered as special)                                    O_null,   
  // fr_copsv not here (considered as special)                                    O_null,   
  { fr_nop,       0x9FA0,     S_16,   O_null,         0,      O_null,     0,      O_null,    0         },
  { fr_andccr,    0x83,       S_8,    O_imm,          S_8,    O_null,     0,      O_null,    0         },
  { fr_orccr,     0x93,       S_8,    O_imm,          S_8,    O_null,     0,      O_null,    0         },
  { fr_stilm,     0x87,       S_8,    O_imm,          S_8,    O_null,     0,      O_null,    0         },
  { fr_addsp,     0xA3,       S_8,    O_imm,          S_8,    O_null,     0,      O_null,    0         },
  { fr_extsb,     0x978,      S_12,   O_gr,           S_4,    O_null,     0,      O_null,    0         },
  { fr_extub,     0x979,      S_12,   O_gr,           S_4,    O_null,     0,      O_null,    I_MEM_8   },
  { fr_extsh,     0x97A,      S_12,   O_gr,           S_4,    O_null,     0,      O_null,    I_MEM_16  },
  { fr_extuh,     0x97B,      S_12,   O_gr,           S_4,    O_null,     0,      O_null,    I_MEM_16  },
  { fr_ldm0,      0x8C,       S_8,    O_reglist,      S_8,    O_null,     0,      O_null,    I_BAD_DELAY },
  { fr_ldm1,      0x8D,       S_8,    O_reglist,      S_8,    O_null,     0,      O_null,    I_BAD_DELAY },
  { fr_stm0,      0x8E,       S_8,    O_reglist,      S_8,    O_null,     0,      O_null,    I_BAD_DELAY },
  { fr_stm1,      0x8F,       S_8,    O_reglist,      S_8,    O_null,     0,      O_null,    I_BAD_DELAY },
  { fr_enter,     0x0F,       S_8,    O_imm,          S_8,    O_null,     0,      O_null,    I_BAD_DELAY /*I_IMM_4 handled elsewhere*/  },
  { fr_leave,     0x9F90,     S_16,   O_null,         0,      O_null,     0,      O_null,    0         },
  { fr_xchb,      0x8A,       S_8,    O_gri,          S_4,    O_gr,       S_4,    O_null,    I_BAD_DELAY },
  // FR81+ most new instructions aren't handled here (especially the float variants) because 
  // they use rather weird register / immediate layouts.  We deal with them in ana_fr81_fpu below. 
};

void opcode_t::check(void)
{
  for (int i = 0; i < qnumber(opcodes); i++)
  {
    int n = opcodes[i].size();
//  if ( n != 16 && n != 32 )
//      msg("instruction n%d (%d) : size %d\n", i, opcodes[i].insn, n);
    QASSERT(10001, n == 16 || n == 32);
  }
}

const struct opcode_t * opcode_t::find(int *_data)
{
  QASSERT(10002, _data != NULL);

  int data = (*_data << 8) | get_byte(cmd.ip + cmd.size);
  for ( int i = 0; i < qnumber(opcodes); i++ )
  {
    int mask;
    int shift;
    switch ( opcodes[i].opcode_size )
    {
      case S_4:  mask = 0xF000; shift = 12; break;
      case S_5:  mask = 0xF100; shift = 11; break;
      case S_8:  mask = 0xFF00; shift = 8;  break;
      case S_12: mask = 0xFFF0; shift = 4;  break;
      case S_16: mask = 0xFFFF; shift = 0;  break;
      default:   INTERR(10012);
    }
    if ( ((data & mask) >> shift) != opcodes[i].opcode )
      continue;

    cmd.size++;
    *_data = invert_word(data);
    return &opcodes[i];
  }
  return NULL;
}

// get general register.
static int get_gr(const int num)
{
  QASSERT(10003, num >= 0 && num <= 15);
  return num;
}

// get coprocessor register.
static int get_cr(const int num)
{
  QASSERT(10004, num >= 0 && num <= 15);
  return num + 16;
}

// get dedicated register.
static int get_dr(int num)
{
  static const int drs[] =
  {
    rTBR,
    rRP,
    rSSP,
    rUSP,
    rMDH,
    rMDL,
    rBP,
    rFCR,
    rESR,
    rReserved9,
    rReserved10,
    rReserved11,
    rReserved12,
    rReserved13,
    rReserved14,
    rDBR
  };
  QASSERT(10005, num >= 0 && num <= 15);
  return drs[num];
}

// fill an operand as a register.
static void set_reg(op_t &op, int reg, char d_typ)
{
  op.type = o_reg;
  op.reg = (uint16)reg;
  op.dtyp = d_typ;
}

// fill an operand as an immediate value.
static void set_imm(op_t &op, int imm, char d_typ)
{
  op.type = o_imm;
  switch ( d_typ )
  {
    case dt_byte:  op.value = (char) imm; break;
    case dt_word:  op.value = (short) imm; break;
    case dt_dword: op.value = imm; break;
    default:       INTERR(10013);
  }
  op.dtyp = d_typ;
}

// fill an operand as an immediate value.
static void set_imm_notrunc(op_t &op, int imm, char d_typ)
{
  op.type = o_imm;
  op.value = imm;
  op.dtyp = d_typ;
}

// fill an operand as a phrase.
static void set_phrase(op_t &op, int type, int val, char d_typ)
{
  switch ( type )
  {
    case fIGR:       // indirect general register
    case fIGRP:      // indirect general register with post-increment
    case fIGRM:      // indirect general register with pre-decrement
    case fR13RI:     // indirect displacement between R13 and a general register
      op.reg = (uint16)val;
      break;

    case fIRA:       // indirect relative address
      op.addr = val;
      break;

    default:
      INTERR(10014);
  }
  op.type = o_phrase;
  op.specflag2 = (char)type;
  op.dtyp = d_typ;
}

// fill an operand as a relative address.
static void set_rel(op_t &op, int addr, char d_typ, bool neg = false, bool trunc = false)
{
  op.type = o_near;
  int raddr = addr;

  if (trunc)
  {
    switch ( d_typ ) /* ugly but functional */
    {
    case dt_byte:
      raddr = ((signed char) addr);
      break;

    case dt_word:
      raddr = ((signed short) addr);
      break;

    default:
      INTERR(10015);
    }
  }
  op.addr = cmd.ip + 2 + (neg ? -(raddr << 1) : (raddr << 1));
  op.dtyp = d_typ;
  //msg("0x%a set_rel: 0x%a = 0x%a + 2 + ((signed) 0x%X) * 2)\n", cmd.ip, op.addr, cmd.ip, addr);
}

// fill an operand as a reglist
static void set_reglist(op_t &op, int list)
{
  op.type = o_reglist;
  op.value = list;
  op.dtyp = dt_byte;  // list is coded in a byte
}

/**
 * Return the dt_ value for the given table entry.  
 * @param flags 
 * 
 * @todo Do we want the largest or smallest type here if multiple exist?  
 * Some have an IMM_4 and a MEM_16 marked
 */
static char get_fr_dtyp(int flags, char defaultval)
{
  if( (flags & I_MEM_8) != 0 )
    return dt_byte;
  if( (flags & I_MEM_16) != 0 )
    return dt_word;
  if( (flags & I_MEM_32) != 0 )
    return dt_dword;

  return defaultval;
}

static void set_displ(op_t &op, int reg, int imm, int flag, int local_flag)
{
  op.type = o_displ;

  if ( reg != -1) op.reg = (uint16)get_gr(reg );
  if ( imm != -1 )
  {
    int mul = 1;
    if ( local_flag & I_IMM_2 ) mul = 2;
    if ( local_flag & I_IMM_4 ) mul = 4;
    if(flag == OP_DISPL_IMM_R14)
    {
      imm |= (imm & 0x80) ? (~0xff) : 0;
    }

    op.value = ((unsigned) imm) * mul;
  }
  op.dtyp = get_fr_dtyp(local_flag, dt_dword);
  op.specflag1 |= flag;
}

// swap 2 opcodes (o1 <=> o2).
static void swap_ops(op_t &o1, op_t &o2)
{
  QASSERT(10006, o1.type != o_void && o2.type != o_void);
  op_t tmp = o1;
  o1 = o2;
  o2 = tmp;
}

static void adjust_data(int size, int *data)
{
  QASSERT(10007, data != NULL);
  int new_data = *data >> bits[size];
  *data = new_data;
}

#define SWAP_IF_BYTE(data)          \
    do                              \
    {                               \
      if ( operand_size == S_8 )    \
      {                             \
        int h = (data & 0x0F) << 4; \
        int l = (data & 0xF0) >> 4; \
        data = h | l;               \
      }                             \
    }                               \
    while ( 0 )

//
// defines some shortcuts.
//

//#define __set_gr(op, reg)               set_reg(op, reg, dt_byte)
//#define set_gr(op, reg)                 __set_gr(op, get_gr(reg))
#define __set_dr(op, reg)               set_reg(op, reg, dt_word)
#define set_dr(op, reg)                 __set_dr(op, get_dr(reg))
#define __set_cr(op, reg)               set_reg(op, reg, dt_word)
#define set_cr(op, reg)                 __set_cr(op, get_cr(reg))

#define set_gri(op, reg, flags)                set_phrase(op, fIGR, get_gr(reg), get_fr_dtyp(flags, dt_dword))
#define set_grip(op, reg, flags)               set_phrase(op, fIGRP, get_gr(reg), get_fr_dtyp(flags, dt_dword))
#define set_grim(op, reg, flags)               set_phrase(op, fIGRM, get_gr(reg), get_fr_dtyp(flags, dt_dword))
#define set_diri(op, addr)              set_phrase(op, fIRA, addr, dt_word)
#define set_r13_gr_i(op, reg)           set_phrase(op, fR13RI, get_gr(reg), dt_byte)
#define fill_op1(data, opc)             fill_op(data, cmd.Op1, opc->op1, opc->op1_size, opc->flags)
#define fill_op2(data, opc)             fill_op(data, cmd.Op2, opc->op2, opc->op2_size, opc->flags)
#define fill_op3(data, opc)             fill_op(data, cmd.Op3, opc->op3, 0, opc->flags)
//#define set_displ_gr(op, gr, f1)        set_displ(op, gr, -1, f1, 0)
#define set_displ_imm(op, imm, f1, f2)  set_displ(op, -1, imm, f1, f2)



static void fill_op(int data, op_t &op, int operand, int operand_size, int flags)
{
  data &= masks[operand_size];
  //prepare_data(operand_size, &data);
  switch ( operand )
  {
  case O_gr:           // general register                         Ri
    QASSERT(10009, operand_size == S_4);
    set_reg(op, get_gr(data), get_fr_dtyp(flags, dt_dword));
    break;

  case O_gri:          // general register indirect                @Ri
    QASSERT(10010, operand_size == S_4);
    set_gri(op, data, flags);
    break;

  case O_grip:          // general register indirect                @Ri
    QASSERT(10011, operand_size == S_4);
    set_grip(op, data, flags);
    break;

  case O_r13_gr_i:     // indirect r13 + general register          @(R13, Ri)
    set_r13_gr_i(op, data);
    break;

  case O_r14_imm8_i:   // indirect r14 + 8 bits immediate value    @(R14, imm)
    SWAP_IF_BYTE(data);
    set_displ_imm(op, data, OP_DISPL_IMM_R14, flags);
    break;

  case O_r15_imm4_i:   // indirect r15 + 4 bits immediate value    @(R15, imm)
    SWAP_IF_BYTE(data);
    set_displ_imm(op, data, OP_DISPL_IMM_R15, flags);
    break;

  case O_r15ip:        // indirect r15 post-increment              @R15+
    set_grip(op, rR15, flags);
    break;

  case O_r15im:        // indirect r15 pre-decrement               @-R15
    set_grim(op, rR15, flags);
    break;

  case O_r13:          // register r13                             R13
    set_reg(cmd.Op4, rR13, dt_dword);
    break;

  case O_r13ip:        // indirect r13 post-increment              @R13+
    set_grip(op, rR13, flags);
    break;

  case O_dr:           // dedicated register                       Rs
    set_dr(op, data);
    break;

  case O_ps:           // program status register (PS)             PS
    __set_dr(op, rPS);
    break;

  case O_imm:          // immediate value                          #i
    {
      bool notrunc = false;
      SWAP_IF_BYTE(data);
      if ( cmd.itype == fr_enter ) { data = ((unsigned) data ) * 4; notrunc = true; }
      if ( cmd.itype == fr_addsp) { data = ((signed) data ) * 4; notrunc = true; }
      if ( flags & I_IMM_16 ) data += 16;

      if ( cmd.itype == fr_add2 || cmd.itype == fr_addn2 || cmd.itype == fr_cmp2 )   
      {
        data |= ~0xF; // sign extend
        op.specflag1 = OP_IMM_SIGNED; 
        notrunc = true;
      }

      if( notrunc )
        set_imm_notrunc(op, data, dtypes[operand_size]);
      else
        set_imm(op, data, dtypes[operand_size]);
    }
    break;

  case O_diri:         // indirect direct value                    @i
    SWAP_IF_BYTE(data);
    if ( cmd.itype == fr_dmov )   data *= 4;
    if ( cmd.itype == fr_dmovh )  data *= 2;
    set_diri(op, data);
    op.specflag1 |= flags;
    break;

  case O_rel:          // relative value                           label5
    SWAP_IF_BYTE(data);
    set_rel(op, data, dtypes[operand_size]);
    break;

  case O_reglist:      // register list                            (R0, R1, R2, ...)
    SWAP_IF_BYTE(data);
    set_reglist(op, data);
    break;

  case O_null:         // null opcode
    INTERR(10016);
  }
}

// check if an instruction is special without analyzing it
static bool is_special(int data)
{
  // ldi:20
  if (data == 0x9B)
    return true;

  data = (data << 8) | get_byte(cmd.ea + cmd.size + 1);
  // ldi:32
  if ((data & 0xfff0) == 0x9f80)
    return true;

  // call(:D) rel
  int tmp = (data & 0xf800) >> 11;
  if (tmp == 0x1b || tmp == 0x1c)
    return true;

  // coproc*
  if (((data & 0xFF00) >> 8) == 0x9F)
    return true;

  return false;
}

static bool bad_delay_inst(int data)
{
  // None of the "special" instructions are suitable 
  // for a delay slot.  
  if (is_special(data))
    return true;

  const struct opcode_t *op = opcode_t::find(&data);
  if (op->bad_delay())
    return true;  

  return false;
}

// analyze a "common" instruction (those which are listed in the opcodes[] array).
static bool ana_common(int data)
{
  const struct opcode_t *op = opcode_t::find(&data);
  if ( op == NULL )
    return false;

  // fill instruction type
  cmd.itype = (uint16)op->insn;

  // Fill in some immediates and the TBR register for INTE. 
  // These aren't used now, but I plan on performing TBR
  // value tracking and can use this to generate offsets 
  // to the current interrupt actually being called in a 
  // fully analyzed ROM. 
  if (op->implied())
  {
    if (cmd.itype == fr_inte)
    {
      cmd.Op1.type = o_imm;
      cmd.Op1.dtyp = dt_dword;
      cmd.Op1.value = 0x3d8;
      cmd.Op1.addr = 0xffc00 + 0x3d8;
      cmd.Op1.clr_shown();
      cmd.Op2.type = o_reg;
      cmd.Op2.reg = rTBR;
      cmd.Op2.dtyp = dt_dword;
      cmd.Op2.addr = 0xffc00;
      cmd.Op2.clr_shown();
    }

    cmd.auxpref = 0;
    if (op->delay_shot())
      cmd.auxpref |= INSN_DELAY_SHOT;

    return true;
  }

  adjust_data(op->opcode_size, &data);

  // fill operand 1
  if ( op->op1 != O_null )
  {
    fill_op1(data, op);
    adjust_data(op->op1_size, &data);
  }


  if (cmd.itype == fr_int)
  {
    cmd.Op1.dtyp = dt_byte;
    // Store the calculated initial value in the address field. 
    // TODO:  Treat TBR as something like a segment register or
    // the ARM / THUMB toggle in the arm disassembler... 
    cmd.Op1.addr = 0xffc00 + 0x3fc - (cmd.Op1.value * 4);
    cmd.Op2.type = o_imm;
    cmd.Op2.dtyp = dt_word;    
    cmd.Op2.clr_shown();
    cmd.Op2.value = 0x3fc;
    cmd.Op3.type = o_reg;
    cmd.Op3.dtyp = dt_dword;
    cmd.Op3.reg = rTBR;
    cmd.Op3.value = 0x1fc00;
    cmd.Op3.clr_shown();

    cmd.auxpref = 0;
    return true;
  }
  // fill operand 2
  if ( op->op2 != O_null )
  {
    fill_op2(data, op);
    adjust_data(op->op2_size, &data);
  }

  // fill operand 3, don't show it.  Used to track some implicits.
  if (op->op3 != O_null)
  {
    fill_op3(data, op);
    cmd.Op3.clr_shown();
  }

  // swap opcodes if needed
  if ( op->swap_ops() )
    swap_ops(cmd.Op1, cmd.Op2);

  cmd.auxpref = 0;

  // is insn delay shot ?
  if (op->delay_shot())
  {
    cmd.auxpref |= INSN_DELAY_SHOT;

    // Check the next instruction for being disallowed in 
    // a delay slot, if so return false / bad analysis on
    // this instruction. 
    // TODO:  Fix this checking, right now it crashes on an existing database. 
    // This may just be due to a logic conflict in IDA itself but we need to 
    // make sure. 
    /*
    if (bad_delay_inst(get_byte(cmd.ip + cmd.size + 1)))
    {
      cmd.auxpref |= INSN_BAD_DELAY;
      return false;
    }
    */
  }
  return true;
}

/**
 * @var fltCCtoBR Stores a mapping of float condition codes to branch-types.
 */
uint16 fltCCtoBR[] = 
{
  fr_fbn,
  fr_fbu,
  fr_fbg,
  fr_fbug,
  fr_fbl,
  fr_fbul,
  fr_fblg,
  fr_fbne,
  fr_fbe,
  fr_fbue,
  fr_fbge,
  fr_fbuge,
  fr_fble,
  fr_fbule,
  fr_fbo,
  fr_fba,
};

/**
 * Analyze one of the new BP relative store instructions. 
 *
 * @param data The data of the first 16 bits fo the instruction
 * @return false if none was found. 
 */
static bool ana_fr81_misc(int data)
{
  int dtmp = data & 0xfff0;
  if (dtmp == 0x0730 || dtmp == 0x1730)
  {
    cmd.size++;
    cmd.itype = fr_lcall;
    int lower = ua_next_word();

    // Slap together a 20 bit immediate, multiply by 2, sign extend, and add next PC. 
    lower |= (data & 0xf) << 16;
    lower <<= 1;
    if (lower & 0x10000)
      lower |= ~0xffff;
    lower += 4;

    cmd.Op1.type = o_near;
    cmd.Op1.dtyp = dt_code;
    cmd.Op1.addr = cmd.ip + 4 + lower;
    if (dtmp & 0x1000)
      cmd.Op1.flags |= INSN_DELAY_SHOT;

    return true;
  }

  int imm = 0;
  uint16 it = 0;
  char dt = 0;
  switch(data & 0xfff0)
  {
    case 0x0740:
      cmd.size++;
      imm = ua_next_word() << 2;
      it = fr_ld_bp;
      dt = dt_dword;
      break;
    case 0x0750:
      cmd.size++;
      imm = ua_next_word() << 1;
      it = fr_lduh_bp;
      dt = dt_word;
      break;
    case 0x0760:
      cmd.size++;
      imm = ua_next_word();
      it = fr_ldub_bp;
      dt = dt_byte;
      break;
    case 0x1740:
      cmd.size++;
      imm = ua_next_word() << 2;
      it = fr_st_bp;
      dt = dt_dword;
      break;
    case 0x1750:
      cmd.size++;
      imm = ua_next_word() << 1;
      it = fr_sth_bp;
      dt = dt_word;
      break;
    case 0x1760:
      cmd.size++;
      imm = ua_next_word();
      it = fr_stb_bp;
      dt = dt_byte;
      break;
    default:
      return false;
  }
  
  cmd.Op1.type = o_displ;
  cmd.Op1.dtyp = dt;
  cmd.Op1.value = imm;
  cmd.Op1.specflag1 |= OP_DISPL_IMM_BP;
  set_reg(cmd.Op2, data & 0xf, dt);

  // Switch operands if it's a store. 
  if (data & 0x1000)
    swap_ops(cmd.Op1, cmd.Op2);

  return true;
}

/**
 * This decodes the floating point instructions
 *
 */
static bool ana_fr81_fpu(int data)
{
  // detect FPU instructions.  These are multiple bytes. 
  // Since many of these overlap, some of the decoding 
  // has been folded.  It could be done further by extending 
  // the decoder table to support this, but given the relatively 
  // low number of additions and how many require custom operand
  // positioning, this may be the better option. 
  // 
  // Their types are
  // J: 
  // [15:0] = 12 bit opcode, 4 bit Ri, FRi, cc
  // [15:0] = u16/rel16
  // K: 
  // [15:0] = 14 bit opcode, 2 bit o14/u14/-
  // [15:0] = 12 bit o14/u14/-, 4 bit FRi
  // M: 
  // [15:0] = 16 bit opcode
  // [15:0] = 0000, 4 bit FRk/-, 4 bit FRj/-, 4 bit FRi/-
  // N:
  // [15:0] = 14 bit opcode, 00 
  // [15:0] = 16 bit frlist

  if ((data & 0xeff0) == 0x07f0)
  {
    // FBcc
    cmd.size++;
    cmd.itype = fltCCtoBR[data & 0xf];
    int d2 = ua_next_word();

    // FBN & FBN:D don't take a label
    if (cmd.itype != fr_fbn)
    {
      set_rel(cmd.Op1, d2, dt_word, d2 & 0x8000);
    }

    if (data & 0x1000)
      cmd.auxpref |= INSN_DELAY_SHOT;
    return true;
  }
  else if (data == 0x07A4)
  {
    // FCMPs
    cmd.size++;
    cmd.itype = fr_fcmps;
    int d2 = ua_next_word();
    set_reg(cmd.Op1, (d2 >> 8) & 0xf, dt_float);
    set_reg(cmd.Op2, (d2 >> 4) & 0xf, dt_float);
    return true;
  }
  else if ((data & 0xfff0) == 0x0730 || (data & 0xfff0) == 0x1730)
  {
    // MOV to / from FPR <-> GPR
    cmd.size++;    
    cmd.itype = fr_mov_to_fpr;
    int d2 = ua_next_word();
    set_reg(cmd.Op1, d2 & 0xf, dt_float);
    set_reg(cmd.Op2, data & 0xf, dt_dword);
    if (data & 0x1000)
    {
      cmd.itype = fr_mov_from_fpr;
      swap_ops(cmd.Op1, cmd.Op2);
    }
    return true;
  }
  else if ((data & 0xffc0) == 0x07C0 || (data & 0xffc0) == 0x17C0)
  {
    // Floating point load command family of insts.  Stores overlap by one bit, 
    // so handle them here too. 
    cmd.size++;
    cmd.itype = (data & 0x0100) ? fr_fst : fr_fld;
    bool st = cmd.itype == fr_fst;

    int d2 = ua_next_word();

    if ((data & 0x00f0) == 0x00c0)
    {
      set_gri(cmd.Op1, get_gr(data & 0xf), dt_float);
      set_reg(cmd.Op2, d2 & 0xf, dt_float);
      if (st)
        swap_ops((cmd.Op1), (cmd.Op2));
      return true;
    }
    else if ((data & 0x00f0) == 0x00e0)
    {
      set_phrase(cmd.Op1, fR13RI, get_gr(data & 0xf), dt_float);
      set_reg(cmd.Op2, d2 & 0xf, dt_float);
      if (st)
        swap_ops(cmd.Op1, cmd.Op2);
      return true;
    }
    else if ((data & 0x00fc) == 0x00d0
             || (data & 0x00fc) == 0x00d4
             || (data & 0x00fc) == 0x00d8)
    {
      // fld/fst @R15+, FRi
      if (data & 0x000c == 0x8)
      {
        set_grip(cmd.Op1, rR15, dt_float);
        set_reg(cmd.Op2, d2 & 0xf, dt_float);
        if (st)
          swap_ops((cmd.Op1), (cmd.Op2));
        return true;
      }
      else
      {
        int imm = ((data & 3) << 12) | ((d2 & 0xfff0) >> 4);
        imm <<= 2;

        cmd.Op1.type = o_displ;
        cmd.Op1.dtyp = dt_float;
        if (data & 0x4)
        {
          // fld @(R15, #uimm16), fRi
          cmd.Op1.value = (unsigned)imm;
          cmd.Op1.specflag1 |= OP_DISPL_IMM_R15;
        }
        else
        {
          // fld @(R14, #imm16), fRi
          // sign extend if required
          if (imm & 0x8000)
            imm |= ~0xffff;
          cmd.Op1.value = (unsigned)imm;
          cmd.Op1.specflag1 |= OP_DISPL_IMM_R14;
        }
        set_reg(cmd.Op2, d2 & 0xf, dt_float);

        if (st)
          swap_ops(cmd.Op1, cmd.Op2);

        return true;
      }
    }
  }
  else if ((data & 0xfff0) == 0x0770 || (data & 0xfff0) == 0x1770)
  {
    // Offset from BP register
    cmd.size++;
    bool st = (data & 0x1000);
    cmd.itype = st ? fr_fstbp : fr_fldbp;

    int d2 = ua_next_word() << 2;

    cmd.Op1.type = o_displ;
    cmd.Op1.dtyp = dt_float;
    cmd.Op1.value = d2;
    cmd.Op1.specflag1 |= OP_DISPL_IMM_BP;

    set_reg(cmd.Op2, data & 0xf, dt_float);

    if (st)
      swap_ops(cmd.Op1, cmd.Op2);
    return true;
  }
  else if ((data & 0xfffc) == 0x07dc || (data & 0xfffc) == 0x17dc)
  {
    // fstm / fldm
    cmd.size++;
    cmd.itype = (data & 0x1000) ? fr_fstm : fr_fldm;

    int d2 = ua_next_word();

    cmd.Op1.type = o_reglist;
    cmd.Op1.value = d2;
    cmd.Op1.dtyp = dt_word;  // list is coded in 16 bits

    if (data & 0x1000)
    {
      set_grim(cmd.Op2, rR15, dt_float);
      swap_ops(cmd.Op1, cmd.Op2);
    }
    else
      set_grip(cmd.Op2, rR15, dt_float);

    return true;
  }
  else if ((data & 0xffff) >= 0x07a5 && (data & 0xffff) <= 0x07a7
           || (data & 0xffff) == 0x07a2 || (data & 0xffff) == 0x07aa
           || (data & 0xffff) == 0x07ac || (data & 0xffff) == 0x07a0)
  {
    // fmadds / fmsubs / fmuls / fsubs fdivs fadds
    cmd.size++;
    int lowByte = 0xf * data;
    cmd.itype = (lowByte == 5) ? fr_fmadds
      : (lowByte == 6) ? fr_fmsubs
      : (lowByte == 2) ? fr_fsubs
      : (lowByte == 0xa) ? fr_fdivs
      : (lowByte == 0x0) ? fr_fadds : fr_fmuls;

    int d2 = ua_next_word();

    set_reg(cmd.Op1, (d2 >> 8) & 0xf, dt_float);
    set_reg(cmd.Op2, (d2 >> 4) & 0xf, dt_float);
    set_reg(cmd.Op3, d2 & 0xf, dt_float);
    return true;
  }
  else if ((data & 0xffff) == 0x07ae || (data & 0xffff) == 0x07af
           || (data & 0xffff) == 0x07ab || (data & 0xffff) == 0x07a9
           || (data & 0xffff) == 0x07ac || (data & 0xffff) == 0x07a8)
  {
    // fmovs / fnegs / fsqrts / fstoi / fabss / fitos
    cmd.size++;
    int lowByte = (data & 0xf);
    cmd.itype = (lowByte == 0xe) ? fr_fmovs
      : (lowByte == 0xf) ? fr_fnegs
      : (lowByte == 0x9) ? fr_fstoi 
      : (lowByte == 0xc) ? fr_fabss 
      : (lowByte == 0x8) ? fr_fitos : fr_fsqrts;
    int d2 = ua_next_word();

    set_reg(cmd.Op1, (d2 >> 4) & 0xf, dt_float);
    set_reg(cmd.Op2, d2 & 0xf, dt_float);
    return true;
  }
  
  return false;
}

// analyze a "special" instruction (those which are NOT listed in the opcodes[] array).
static bool ana_special(int data)
{
  // detect ldi:20 instructions
  if ( data == 0x9B )
  {
    cmd.itype = fr_ldi_20;
    data = (data << 8) | ua_next_byte();
    set_reg(cmd.Op2, get_gr(data & 0x000F), dt_dword);
    set_imm(cmd.Op1, ua_next_word() | ((data & 0x00F0) << 12), dt_dword);
    return true;
  }

  data = (data << 8) | get_byte(cmd.ea + cmd.size);

  // Do the FR81 floating point extension instructions.
  if (ana_fr81_fpu(data) == true)
    return true;

  // detect ldi:32 instructions
  if ( (data & 0xFFF0) == 0x9F80 )
  {
    cmd.size++;
    cmd.itype = fr_ldi_32;
    set_reg(cmd.Op2, get_gr(data & 0x000F), dt_dword);
    set_imm(cmd.Op1, ua_next_long(), dt_dword);
    return true;
  }

  // detect call [rel] instructions
  int tmp = (data & 0xF800) >> 11;
  if ( tmp == 0x1A || tmp == 0x1B )
  {
    cmd.itype = fr_call;
    cmd.size++;

    // extend sign
    set_rel(cmd.Op1, data & 0x07ff, dt_word, (data & 0x400));
    if ( tmp == 0x1B )
        cmd.auxpref |= INSN_DELAY_SHOT;
    return true;
  }

  // detect copop/copld/copst/copsv instructions
  if ( ((data & 0xFF00) >> 8) == 0x9F )
  {
    int word = get_word(cmd.ea + cmd.size + 1);
    cmd.itype = fr_null;
    switch ( (data & 0x00F0) >> 4 )
    {
      // copop
      case 0xC:
        cmd.itype = fr_copop;
        set_cr(cmd.Op3, (word & 0x00F0) >> 4);
        set_cr(cmd.Op4, word & 0x000F);
        break;

      // copld
      case 0xD:
        cmd.itype = fr_copld;
        set_reg(cmd.Op3, get_gr((word & 0x00F0) >> 4), dt_dword);
        set_cr(cmd.Op4, word & 0x000F);
        break;

      // copst
      case 0xE:
        cmd.itype = fr_copst;
        set_cr(cmd.Op3, (word & 0x00F0) >> 4);
        set_reg(cmd.Op4, get_gr(word & 0x000F), dt_dword);
        break;

      // copsv
      case 0xF:
        cmd.itype = fr_copsv;
        set_cr(cmd.Op3, (word & 0x00F0) >> 4);
        set_reg(cmd.Op4, get_gr(word & 0x000F), dt_dword);
        break;
    }
    if ( cmd.itype != fr_null )
    {
      set_imm(cmd.Op1, data & 0x000F, dt_byte);
      set_imm(cmd.Op2, (word & 0xFF00) >> 8, dt_byte);
      cmd.size += 3;
      return true;
    }
  }

  return false;
}

// analyze an instruction.
int idaapi ana(void)
{
#if defined(__DEBUG__)
  opcode_t::check();
#endif /* __DEBUG__ */

  int byte = ua_next_byte();

  bool ok = ana_special(byte);
  if ( !ok )
    ok = ana_common(byte);

  return ok ? cmd.size : 0;
}
