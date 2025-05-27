#pragma once

#include <stddef.h>
enum E_OPCODE{
  // - TRANSFER
  // MOV
  MOV_RM_R,
  MOV_I_RM, 
  MOV_I_R,
  MOV_M_A,
  MOV_A_M,
  MOV_RM_SR,
  MOV_SR_RM,
  // PUSH
  PUSH_RM, 
  PUSH_R,
  PUSH_SR, 
  // POP
  POP_RM,
  POP_R,
  POP_SR, 
  // XCHG
  XCHG_RM_R,
  XCHG_R_A,
  // IN
  IN_FIXED,
  IN_VARIABLE,
  // OUT
  OUT_FIXED,
  OUT_VARIABLE,
  // TRANSFER - OTHER
  XLAT,
  LEA,
  LDS,
  LES,
  LAHF,
  SAHF,
  PUSHF,
  POPF,

  // - ARITHMETIC
  // ADD
  ADD_RM_R,
  ADD_I_RM, 
  ADD_I_A,
  // ADC
  ADC_RM_R,
  ADC_I_RM, 
  ADC_I_A,
  // SUB
  SUB_RM_R,
  SUB_I_RM,
  SUB_I_A,
  // SBB
  SBB_RM_R,
  SBB_I_RM,
  SBB_I_A,
  // INC
  INC_RM,  
  INC_R,
  // DEC
  DEC_RM,
  DEC_R,
  // CMP
  CMP_RM_R,
  CMP_I_RM,
  CMP_I_A,
  // ARITHMETIC - OTHER
  NEG,
  AAA,
  BAA,
  AAS,
  DAS,
  MUL,
  IMUL,
  DIV,
  IDIV,
  AAM,
  AAD,
  CBW,
  CWD,
  // - LOGIC
  // AND
  AND_RM_R,
  AND_I_RM,
  AND_I_A,
  // TEST
  TEST_RM_R,
  TEST_I_RM,
  TEST_I_A,
  // OR
  OR_RM_R,
  OR_I_RM,
  OR_I_A,
  // XOR
  XOR_RM_R,
  XOR_I_RM,
  XOR_I_A,
  // LOGIC - OTHER
  NOT,
  SHL_SAL,
  SHR,
  SAR,
  ROL,
  ROR,
  RCL,
  RCR,
  // - STRINGS
  REP,
  MOVSB,
  MOVSW,
  CMPSB,
  CMPSW,
  SCASB,
  SCASW,
  LODSB,
  LODSW,
  STOSB,
  STOSW,
  // - CONTROL
  // CALL
  CALL_SEG_DIR,
  CALL_SEG_IDIR,
  CALL_ISEG_DIR,
  CALL_ISEG_IDIR,
  // JMP
  JMP_SEG_DIR,
  JMP_SSEG_DIR,
  JMP_SEG_IDIR,
  JMP_ISEG_DIR,
  JMP_ISEG_IDIR,
  // RET
  RET_SEG,
  RET_SEG_I_SP,
  RET_ISEG,
  RET_ISEG_I_SP,
  // INT
  INT_TYPE_SPEC,
  INT_TYPE_THREE,
  // CONTROL - OTHER
  JE_JZ,
  JL_JNGE,
  JLE_JNG,
  JB_JNAE,
  JBE_JNA,
  JP_JPE,
  JO,
  JS,
  JNE_JNZ,
  JNL_JGE,
  JNLE_JG,
  JNB_JAE,
  JNBE_JA,
  JNP_JPO,
  JNO,
  JNS,
  LOOP,
  LOOPZ_LOOPE,
  LOOPNZ_LOOPNE,
  JCXZ,
  INTO,
  IRET,
  // - PROCESSOR 
  CLC,
  CMC,
  STC,
  CLD,
  STD,
  CLI,
  STI,
  HLT,
  WAIT,
  ESC,
  LOCK,
  // - SEGMENT OVERRIDE
  SEG_OVR
};

enum SEGOVR_POS{
  SEGOVR_POS_NONE,
  SEGOVR_POS_DST,
  SEGOVR_POS_SRC
};

// Returns the size of the first instruction found in bytes
// If no valid mnemonic is found (opcode wrong or instruction length greater than remaining number of bytes)
// then "undefined" is returned as a mnemonic, and 1 is returned as length (to skip a byte)
// The function outputs the correct strings in each of the respective string* parameters
// The max length of "bytes" is 8 (max length of an instruction is 8)
// 3 prefix + 1 mnemonic + 2 disp + 2 data
// Instruction index is the index (since program text start) in bytes of the instruction
// Bytes count is the remaining number of bytes in the program text (to prevent going beyond the program text)
size_t get_first_instruction(unsigned char* bytes, 
                             char** prefix_lock, char** prefix_rep, char** prefix_segovr, int* prefix_segovr_pos,
                             char** mnemonic, char** size_spec, char* destination, char* source,
                             int instruction_index, size_t bytes_count);

void format_instruction(size_t program_index, size_t instruction_size, unsigned char* bytes,
                        char* prefix_lock, char* prefix_rep, char* prefix_segovr, int prefix_segovr_pos,
                        char* mnemonic, char* size_spec, char* destination, char* source, 
                        char* buf, size_t buf_size);
