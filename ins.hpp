
#ifndef __ins_hpp
#define __ins_hpp

extern instruc_t Instructions[];

enum nameNum ENUM_SIZE(uint16)
{
    fr_null = 0,            // null instruction

    fr_add,                 // add word data of source register / 4-bit immediate data to destination register
    fr_add2,                // add 4-bit immediate data to destination register
    fr_addc,                // add word data of source register and carry bit to destination register
    fr_addn,                // add word data of source register / immediate data to destination register
    fr_addn2,               // add immediate data to destination register
    fr_sub,                 // subtract word data in source register from destination register
    fr_subc,                // subtract word data in source register and carry bit from destination register
    fr_subn,                // subtract word data in source register from destination register
    fr_cmp,                 // compare word / immediate data in source register and destination register
    fr_cmp2,                // compare immediate data and destination register
    fr_and,                 // and word data of source register to destination register / data in memory
    fr_andh,                // and half-word data of source register to data in memory
    fr_andb,                // and byte data of source register to data in memory
    fr_or,                  // or word data of source register to destination register / data in memory
    fr_orh,                 // or half-word data of source register to data in memory
    fr_orb,                 // or byte data of source register to data in memory
    fr_eor,                 // exclusive or word data of source register to destination register / data in memory
    fr_eorh,                // exclusive or half-word data of source register to data in memory
    fr_eorb,                // exclusive or byte data of source register to data in memory
    fr_bandl,               // and 4-bit immediate data to lower 4 bits of byte data in memory
    fr_bandh,               // and 4-bit immediate data to higher 4 bits of byte data in memory
    fr_borl,                // or 4-bit immediate data to lower 4 bits of byte data in memory
    fr_borh,                // or 4-bit immediate data to higher 4 bits of byte data in memory
    fr_beorl,               // eor 4-bit immediate data to lower 4 bits of byte data in memory
    fr_beorh,               // eor 4-bit immediate data to higher 4 bits of byte data in memory
    fr_btstl,               // test lower 4 bits of byte data in memory
    fr_btsth,               // test higher 4 bits of byte data in memory
    fr_mul,                 // multiply word data
    fr_mulu,                // multiply unsigned word data
    fr_mulh,                // multiply half-word data
    fr_muluh,               // multiply unsigned half-word data
    fr_div0s,               // initial setting up for signed division
    fr_div0u,               // initial setting up for unsigned division
    fr_div1,                // main process of division
    fr_div2,                // correction when remainder is 0
    fr_div3,                // correction when remainder is 0
    fr_div4s,               // correction answer for signed division
    fr_lsl,                 // logical shift to the left direction
    fr_lsl2,                // logical shift to the left direction
    fr_lsr,                 // logical shift to the right direction
    fr_lsr2,                // logical shift to the right direction
    fr_asr,                 // arithmetic shift to the right direction
    fr_asr2,                // arithmetic shift to the right direction
    fr_ldi_32,              // load immediate 32-bit data to destination register
    fr_ldi_20,              // load immediate 20-bit data to destination register
    fr_ldi_8,               // load immediate 8-bit data to destination register
    fr_ld,                  // load word data in memory to register / program status register
    fr_lduh,                // load half-word data in memory to register
    fr_ldub,                // load byte data in memory to register
    fr_st,                  // store word data in register / program status register to memory
    fr_sth,                 // store half-word data in register to memory
    fr_stb,                 // store byte data in register to memory
    fr_mov,                 // move word data in source register / program status register to destination register / program status register
    fr_jmp,                 // jump
    fr_call,                // call subroutine
    fr_ret,                 // return from subroutine
    fr_int,                 // software interrupt
    fr_inte,                // software interrupt for emulator
    fr_reti,                // return from interrupt
    fr_bra,                 // branch relative if condition satisfied
    fr_bno,                 // branch relative if condition satisfied
    fr_beq,                 // branch relative if condition satisfied
    fr_bne,                 // branch relative if condition satisfied
    fr_bc,                  // branch relative if condition satisfied
    fr_bnc,                 // branch relative if condition satisfied
    fr_bn,                  // branch relative if condition satisfied
    fr_bp,                  // branch relative if condition satisfied
    fr_bv,                  // branch relative if condition satisfied
    fr_bnv,                 // branch relative if condition satisfied
    fr_blt,                 // branch relative if condition satisfied
    fr_bge,                 // branch relative if condition satisfied
    fr_ble,                 // branch relative if condition satisfied
    fr_bgt,                 // branch relative if condition satisfied
    fr_bls,                 // branch relative if condition satisfied
    fr_bhi,                 // branch relative if condition satisfied
    fr_dmov,                // move word data from register / address to register / address
    fr_dmovh,               // move half-word data from register / address to register / address
    fr_dmovb,               // move byte data from register / address to register / address
    fr_ldres,               // load word data in memory to resource
    fr_stres,               // store word data in resource to memory
    fr_copop,               // coprocessor operation
    fr_copld,               // load 32-bit data from register to coprocessor register
    fr_copst,               // store 32-bit data from coprocessor register to register
    fr_copsv,               // save 32-bit data from coprocessor register to register
    fr_nop,                 // no operation
    fr_andccr,              // and condition code register and immediate data
    fr_orccr,               // or condition code register and immediate data
    fr_stilm,               // set immediate data to interrupt level mask register
    fr_addsp,               // add stack pointer and immediate data
    fr_extsb,               // sign extend from byte data to word data
    fr_extub,               // unsign extend from byte data to word data
    fr_extsh,               // sign extend from byte data to word data
    fr_extuh,               // unsigned extend from byte data to word data
    fr_ldm0,                // load multiple registers
    fr_ldm1,                // load multiple registers
    fr_stm0,                // store multiple registers
    fr_stm1,                // store multiple registers
    fr_enter,               // enter function
    fr_leave,               // leave function
    fr_xchb,                // exchange byte data
    // FR65+
    fr_srch0,               // search first zero bit position distance from MSB
    fr_srch1,               // search first one bit position distance from MSB
    fr_srchc,               // search first bit value change position distance from MSB
    // FR80+

    // FR81+
    fr_fabss,
    fr_fadds,
    fr_fbn,                 // float branch never
    fr_fba,                 // float branch always
    fr_fbne,                // float branch not equal
    fr_fbe,                 // float branch equal
    fr_fblg,                // float branch less or greater
    fr_fbue,                // float branch unordered or equal
    fr_fbul,                // float branch unordered or less
    fr_fbge,                // float branch greater or equal
    fr_fbl,                 // float branch less
    fr_fbuge,               // float branch unordered or greater or equal
    fr_fbug,                // float branch unordered or greater
    fr_fble,                // float branch less or equal
    fr_fbg,                 // float branch greater
    fr_fbule,               // float branch unordered or less or equal
    fr_fbu,                 // float branch unordered
    fr_fbo,                 // float branch ordered
    fr_fcmps,               // floating point compare
    fr_fdivs,               // floating point divide
    fr_fitos,               // 32 bit integer to float
    fr_fld,                 // float load (Various indexing methods as ld)
    fr_fldbp,               // load from @(BP, #udisp18)
    fr_fldm,                // Load multiple single precision floats from @R15+
    fr_fmadds,              // Fused multiply-add
    fr_fmovs,               // Mov between fpu regs
    fr_fmsubs,              // Fused multiply subtract
    fr_fmuls,               // Multiply floats
    fr_fnegs,               // Negative of float
    fr_fsqrts,              // Square root
    fr_fst,                 // Store a float (Various indexing  methods as st)
    fr_fstbp,               // Store a float @(BP, #udisp18)
    fr_fstm,                // Store multiple floats.  Can be anywhere from 1 to all 16
    fr_fstoi,               // Convert single to integer in reg
    fr_fsubs,               // Subtract
    fr_mov_to_fpr,          // mov from Rj to FRi
    fr_mov_from_fpr,        // mov from FRj to Ri
    fr_lcall,               // long call (21 bit sign extended relative) and :D version
    fr_ld_bp,               // ld @(BP, #udisp18 (#udisp16 * 4)), Ri
    fr_ldub_bp,             // ld BYTE @(BP, #udisp16), Ri    
    fr_lduh_bp,             // ld WORD @(BP, #udisp17 (#udisp16 * 2)), Ri
    fr_st_bp,
    fr_stb_bp,
    fr_sth_bp,

    fr_last                 // last instruction
};

#endif /* __ins_hpp */
