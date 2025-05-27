#include "../include/instructions.h"
#include "../include/hex.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <string.h>

#define maskn 0b11111111
#define maskm 0b11100111
#define maska 0b11111110
#define maskb 0b11111100
#define maskc 0b11111000
#define maskd 0b11110000
#define maskspe 0b00111000
#define maskovr 0b11100111

#define NB_OPCODE_TABLE 107
#define NB_P_TABLE 7
#define NB_A_TABLE 8
#define NB_M_TABLE 7
#define NB_S_TABLE 7
#define NB_PREF_TABLE 3

unsigned char opcode_table[NB_OPCODE_TABLE][4] = {
  {MOV_RM_R, maskb, 0b10001000, 2},
  {MOV_I_RM, maska, 0b11000110, 2},
  {MOV_I_R, maskd, 0b10110000, 1},
  {MOV_M_A, maska, 0b10100000, 3},  // address is always 16bit
  {MOV_A_M, maska, 0b10100010, 3},  // same
  {MOV_RM_SR, maskn, 0b10001110, 2},
  {MOV_SR_RM, maskn, 0b10001100, 2},
  {PUSH_R, maskc, 0b01010000, 1},
  {PUSH_SR, maskm, 0b00000110, 1},
  {POP_RM, maskn, 0b10001111, 2},
  {POP_R, maskc, 0b01011000, 1},
  {POP_SR, maskm, 0b00000111, 1},
  {XCHG_RM_R, maska, 0b10000110, 2},
  {XCHG_R_A, maskc, 0b10010000, 1},
  {IN_FIXED, maska, 0b11100100, 1},
  {IN_VARIABLE, maska, 0b11101100, 1},
  {OUT_FIXED, maska, 0b11100110, 1},
  {OUT_VARIABLE, maska, 0b11101110, 1},
  {XLAT, maskn, 0b11010111, 1},
  {LEA, maskn, 0b10001101, 2},
  {LDS, maskn, 0b11000101, 2},
  {LES, maskn, 0b11000100, 2},
  {LAHF, maskn, 0b10011111, 1},
  {SAHF, maskn, 0b10011110, 1},
  {PUSHF, maskn, 0b10011100, 1},
  {POPF, maskn, 0b10011101, 1},
  {ADD_RM_R, maskb, 0b00000000, 2},
  {ADD_I_A, maska, 0b00000100, 1},
  {ADC_RM_R, maskb, 0b00010000, 2},
  {ADC_I_A, maska, 0b00010100, 1},
  {SUB_RM_R, maskb, 0b00101000, 2},
  {SUB_I_A, maska, 0b00101100, 1},
  {SBB_RM_R, maskb, 0b00011000, 2},
  {SBB_I_A, maska, 0b00001110, 1}, // this one weird only 7 bits
  {INC_R, maskc, 0b01000000, 1},
  {DEC_R, maskc, 0b01001000, 1},
  {CMP_RM_R, maskb, 0b00111000, 2},
  {CMP_I_A, maska, 0b00111100, 1},
  {AAA, maskn, 0b00110111, 1},
  {BAA, maskn, 0b00100111, 1},
  {AAS, maskn, 0b00111111, 1},
  {DAS, maskn, 0b00101111, 1},
  {AAM, maskn, 0b11010100, 2},
  {AAD, maskn, 0b11010101, 2},
  {CBW, maskn, 0b10011000, 1},
  {CWD, maskn, 0b10011001, 1},
  {AND_RM_R, maskb, 0b00100000, 2},
  {AND_I_A, maska, 0b00100100, 1},
  {TEST_RM_R, maska,0b10000100, 2},
  {TEST_I_A, maska, 0b10101000, 1},
  {OR_RM_R, maskb, 0b00001000, 2},
  {OR_I_A, maska, 0b00001100, 1},
  {XOR_RM_R, maskb, 0b00110000, 2},
  {XOR_I_A, maska, 0b00110100, 1},
  {MOVSB, maskn, 0b10100100, 1}, // These appear as MOVS with a w byte to decide the size in the doc, however 
  {MOVSW, maskn, 0b10100101, 1}, // it is easier to treat them as 2 separate opcodes because the operand text is different
  {CMPSB, maskn, 0b10100110, 1},
  {CMPSW, maskn, 0b10100111, 1},
  {SCASB, maskn, 0b10101110, 1},
  {SCASW, maskn, 0b10101111, 1},
  {LODSB, maskn, 0b10101100, 1},
  {LODSW, maskn, 0b10101101, 1},
  {STOSB, maskn, 0b10101010, 1},
  {STOSW, maskn, 0b10101011, 1},
  {CALL_SEG_DIR, maskn, 0b11101000, 3},
  {CALL_ISEG_DIR, maskn, 0b10011010, 3}, // wtf is this
  {JMP_SEG_DIR, maskn, 0b11101001, 3},
  {JMP_SSEG_DIR, maskn, 0b11101011, 2},
  {JMP_ISEG_DIR, maskn, 0b11101010, 3},  // wtf is this
  {RET_SEG, maskn, 0b11000011, 1},
  {RET_SEG_I_SP, maskn, 0b11000010, 3},
  {RET_ISEG, maskn, 0b11001011, 1},
  {RET_ISEG_I_SP, maskn, 0b11001010, 3},
  {INT_TYPE_SPEC, maskn, 0b11001101, 1},
  {INT_TYPE_THREE, maskn, 0b11001100, 1},
  {JE_JZ, maskn, 0b01110100, 2},
  {JL_JNGE, maskn, 0b01111100, 2},
  {JLE_JNG, maskn, 0b01111110, 2},
  {JB_JNAE, maskn, 0b01110010, 2},
  {JBE_JNA, maskn, 0b01110110, 2},
  {JP_JPE, maskn, 0b01111010, 2},
  {JO, maskn, 0b01110000, 2},
  {JS, maskn, 0b01111000, 2},
  {JNE_JNZ, maskn, 0b01110101, 2},
  {JNL_JGE, maskn, 0b01111101, 2},
  {JNLE_JG, maskn, 0b01111111, 2},
  {JNB_JAE, maskn, 0b01110011, 2},
  {JNBE_JA, maskn, 0b01110111, 2},
  {JNP_JPO, maskn, 0b01111011, 2},
  {JNO, maskn, 0b01110001, 2},
  {JNS, maskn, 0b01111001, 2},
  {LOOP, maskn, 0b11100010, 2},
  {LOOPZ_LOOPE, maskn, 0b11100001, 2},
  {LOOPNZ_LOOPNE, maskn, 0b11100000, 2},
  {JCXZ, maskn, 0b11100011, 2},
  {INTO, maskn, 0b11001110, 1},
  {IRET, maskn, 0b11001111, 1},
  {CLC, maskn, 0b11111000, 1},
  {CMC, maskn, 0b11110101, 1},
  {STC, maskn, 0b11111001, 1},
  {CLD, maskn, 0b11111100, 1},
  {STD, maskn, 0b11111101, 1},
  {CLI, maskn, 0b11111010, 1},
  {STI, maskn, 0b11111011, 1},
  {HLT, maskn, 0b11110100, 1},
  {WAIT, maskn, 0b10011011, 1},
  {ESC, maskc, 0b11011000, 2},
};

unsigned char p_table[NB_P_TABLE][4] = {
  {PUSH_RM, maskspe, 0b00110000, 2},
  {INC_RM, maskspe, 0b00000000, 2},
  {DEC_RM, maskspe, 0b00001000, 2},
  {CALL_SEG_IDIR, maskspe, 0b00010000, 2},
  {CALL_ISEG_IDIR, maskspe, 0b00011000, 2},
  {JMP_SEG_IDIR, maskspe, 0b00100000, 2},
  {JMP_ISEG_IDIR, maskspe, 0b00101000, 2},
};
unsigned char a_table[NB_A_TABLE][4] = {
  {ADD_I_RM, maskspe, 0b00000000, 2},
  {ADC_I_RM, maskspe, 0b00010000, 2},
  {SUB_I_RM, maskspe, 0b00101000, 2},
  {SBB_I_RM, maskspe, 0b00011000, 2},
  {CMP_I_RM, maskspe, 0b00111000, 2},
  {AND_I_RM, maskspe, 0b00100000, 2},
  {OR_I_RM, maskspe, 0b00001000, 2},
  {XOR_I_RM, maskspe, 0b00110000, 2},
};
unsigned char m_table[NB_M_TABLE][4] = {
  {MUL, maskspe, 0b00100000, 2},
  {IMUL, maskspe, 0b00101000, 2},
  {DIV, maskspe, 0b00110000, 2},
  {DIV, maskspe, 0b00111000, 2},
  {NEG, maskspe, 0b00011000, 2},
  {NOT, maskspe, 0b00010000, 2},
  {TEST_I_RM, maskspe, 0b00000000, 2}
};
unsigned char s_table[NB_S_TABLE][4] = {
  {SHL_SAL, maskspe, 0b00100000, 2},
  {SHR, maskspe, 0b00101000, 2},
  {SAR, maskspe, 0b00111000, 2},
  {ROL, maskspe, 0b00000000, 2},
  {ROR, maskspe, 0b00001000, 2},
  {RCL, maskspe, 0b00010000, 2},
  {RCR, maskspe, 0b00011000, 2},
};
unsigned char pref_table[NB_PREF_TABLE][3] = {
  {LOCK, maskn, 0b11110000},
  {REP, maska, 0b11110010},
  {SEG_OVR, maskovr, 0b00100110}
};

char* get_mnemonic(enum E_OPCODE opcode){
  switch(opcode){
    case MOV_RM_R:
    case MOV_I_RM:
    case MOV_I_R:
    case MOV_M_A:
    case MOV_A_M:
    case MOV_RM_SR:
    case MOV_SR_RM:
      return "mov";
    case PUSH_RM:
    case PUSH_R:
    case PUSH_SR:
      return "push";
    case POP_RM:
    case POP_R:
    case POP_SR:
      return "pop";
    case XCHG_RM_R:
    case XCHG_R_A:
      return "xchg";
    case IN_FIXED:
    case IN_VARIABLE:
      return "in";
    case OUT_FIXED:
    case OUT_VARIABLE:
      return "out";
    case XLAT:
      return "xlat";
    case LEA:
      return "lea";
    case LDS:
      return "lds";
    case LES:
      return "les";
    case LAHF:
      return "lahf";
    case SAHF:
      return "sahf";
    case PUSHF:
      return "pushf";
    case POPF:
      return "popf";
    case ADD_RM_R:
    case ADD_I_RM:
    case ADD_I_A:
      return "add";
    case ADC_RM_R:
    case ADC_I_RM:
    case ADC_I_A:
      return "adc";
    case SUB_RM_R:
    case SUB_I_RM:
    case SUB_I_A:
      return "sub";
    case SBB_RM_R:
    case SBB_I_RM:
    case SBB_I_A:
      return "sbb";
    case INC_RM:
    case INC_R:
      return "inc";
    case DEC_RM:
    case DEC_R:
      return "dec";
    case CMP_RM_R:
    case CMP_I_RM:
    case CMP_I_A:
      return "cmp";
    case NEG:
      return "neg";
    case AAA:
      return "aaa";
    case BAA:
      return "baa";
    case AAS:
      return "aas";
    case DAS:
      return "das";
    case MUL:
      return "mul";
    case IMUL:
      return "imul";
    case DIV:
      return "div";
    case IDIV:
      return "idiv";
    case AAM:
      return "aam";
    case AAD:
      return "aad";
    case CBW:
      return "cbw";
    case CWD:
      return "cwd";
    case AND_RM_R:
    case AND_I_RM:
    case AND_I_A:
      return "and";
    case TEST_RM_R:
    case TEST_I_RM:
    case TEST_I_A:
      return "test";
    case OR_RM_R:
    case OR_I_RM:
    case OR_I_A:
      return "or";
    case XOR_RM_R:
    case XOR_I_RM:
    case XOR_I_A:
      return "xor";
    case NOT:
      return "not";
    case SHL_SAL:
      return "shl";
    case SHR:
      return "shr";
    case SAR:
      return "sar";
    case ROL:
      return "rol";
    case ROR:
      return "ror";
    case RCL:
      return "rcl";
    case RCR:
      return "rcr";
    case REP:
      return "rep";
    case MOVSB:
      return "movsb";
    case MOVSW:
      return "movsw";
    case CMPSB:
      return "cmpsb";
    case CMPSW:
      return "cmpsw";
    case SCASB:
      return "scasb";
    case SCASW:
      return "scasw";
    case LODSB:
      return "lodsb";
    case LODSW:
      return "lodsw";
    case STOSB:
      return "stosb";
    case STOSW:
      return "stosw";
    case CALL_SEG_DIR:
    case CALL_SEG_IDIR:
      return "call";
    case CALL_ISEG_DIR:
    case CALL_ISEG_IDIR:
      return "call far";
    case JMP_SEG_DIR:
    case JMP_SEG_IDIR:
      return "jmp";
    case JMP_ISEG_DIR:
    case JMP_ISEG_IDIR:
      return "jmp far";
    case JMP_SSEG_DIR:
      return "jmp short";
    case RET_SEG:
    case RET_SEG_I_SP:
      return "ret";
    case RET_ISEG:
    case RET_ISEG_I_SP:
      return "retf";
    case INT_TYPE_SPEC:
    case INT_TYPE_THREE:
      return "int";
    case JE_JZ:
      return "je";
    case JL_JNGE:
      return "jl";
    case JLE_JNG:
      return "jle";
    case JB_JNAE:
      return "jb";
    case JBE_JNA:
      return "jbe";
    case JP_JPE:
      return "jp";
    case JO:
      return "jo";
    case JS:
      return "js";
    case JNE_JNZ:
      return "jne";
    case JNL_JGE:
      return "jnl";
    case JNLE_JG:
      return "jnle";
    case JNB_JAE:
      return "jnb";
    case JNBE_JA:
      return "jnbe";
    case JNP_JPO:
      return "jnp";
    case JNO:
      return "jno";
    case JNS:
      return "jns";
    case LOOP:
      return "loop";
    case LOOPZ_LOOPE:
      return "loope";
    case LOOPNZ_LOOPNE:
      return "loopne";
    case JCXZ:
      return "jcxz";
    case INTO:
      return "into";
    case IRET:
      return "iret";
    case CLC:
      return "clc";
    case CMC:
      return "cmc";
    case STC:
      return "stc";
    case CLD:
      return "cld";
    case STD:
      return "std";
    case CLI:
      return "cli";
    case STI:
      return "sti";
    case HLT:
      return "hlt";
    case WAIT:
      return "wait";
    case ESC:
      return "esc";
    case LOCK:
      return "lock";
    default:
      return "did not find mnemonic";
  }
}

char* get_reg16(char reg){
  switch(reg){
    case 0b00000000:
      return "ax";
    case 0b00000001:
      return "cx";
    case 0b00000010:
      return "dx";
    case 0b00000011:
      return "bx";
    case 0b00000100:
      return "sp";
    case 0b00000101:
      return "bp";
    case 0b00000110:
      return "si";
    case 0b00000111:
      return "di";
    default:
      return "did not find register";
  }
}
char* get_reg8(char reg){
  switch(reg){
    case 0b00000000:
      return "al";
    case 0b00000001:
      return "cl";
    case 0b00000010:
      return "dl";
    case 0b00000011:
      return "bl";
    case 0b00000100:
      return "ah";
    case 0b00000101:
      return "ch";
    case 0b00000110:
      return "dh";
    case 0b00000111:
      return "bh";
    default:
      return "did not find register";
  }
}
char* get_reg(char w, char reg){
  if (w==0){
    return get_reg8(reg);
  }
  else{
    return get_reg16(reg);
  }
}
char* get_seg(char seg){
  switch(seg){
    case 0b00000000:
      return "es";
    case 0b00000001:
      return "cs";
    case 0b00000010:
      return "ss";
    case 0b00000011:
      return "ds";
    default:
      return "did not find register";
  }
}


void get_formatted_immediate8(char* str, unsigned char im){
  byte_to_hex(str, im);
  str[2] = 0;
}
void get_formatted_immediate16(char* str, unsigned char im1, unsigned char im2){
  // reverse order for little endian
  byte_to_hex(str, im2);
  byte_to_hex(str+2, im1);
  str[4] = 0;
}
size_t get_formatted_immediate_w(char* str, unsigned char im1, unsigned char im2, unsigned char w){
  if (w == 1){
    get_formatted_immediate16(str, im1, im2);
    return 2;
  }
  else{
    get_formatted_immediate8(str, im1);
    return 1;
  }
}

void get_formatted_signed_immediate8(char* str, unsigned char im){
  if ((im & 0b10000000) == 0b10000000){
    str[0] = '-';
    byte_to_hex(str+1, two_complement8(im));
  }
  else{
    byte_to_hex(str, im);
  }
}

void get_formatted_signed_immediate16(char* str, unsigned char im1, unsigned char im2){
  if ((im2 & 0b10000000) == 0b10000000){
    str[0] = '-';
    short bytes = im1;
    bytes += im2 << 8;
    twobyte_to_hex(str+1, two_complement16(bytes));
  }
  else{
    byte_to_hex(str, im2);
    byte_to_hex(str+2, im1);
  }
}

void get_formatted_label_addr16(char* str, unsigned char im1, unsigned char im2, int i){
  short bytes = im1;
  bytes += im2 << 8;
  if ((bytes & 0b1000000000000000) == 0b1000000000000000){
    twobyte_to_hex(str, i-two_complement16(bytes));
  }
  else{
    twobyte_to_hex(str, i+bytes);
  }
}

void get_formatted_label_addr8(char* str, unsigned char im, int i){
  if ((im & 0b10000000) == 0b10000000){
    twobyte_to_hex(str, i-two_complement8(im));
  }
  else{
    twobyte_to_hex(str, i+im);
  }
}

size_t get_formatted_immediate_sw(char* str, unsigned char im1, unsigned char im2,
                                  unsigned char s, unsigned char w){
  if (w == 1 && s == 1){
    get_formatted_signed_immediate8(str, im1);
    return 1;
  }
  else if (w == 1){
    get_formatted_immediate16(str, im1, im2);
    return 2;
  }
  else{
    get_formatted_immediate8(str, im1);
    return 1;
  }
}

void remove_leading_zeros(char* nb, size_t len){
  int seen_nb = 0;
  for (size_t i=0; i<len && !seen_nb; i++){
    if ((nb[i] > '0' && nb[i] <= '9') || (nb[i] >= 'a' && nb[i] <= 'f')){
      seen_nb = 1;
    }
    else if (nb[i] == '0'){
      for (size_t j=i+1; j<len; j++){
        nb[j-1] = nb[j];
      }
      i--;
    }
  }
  if (nb[0] == 0 && len != 0){
    nb[0] = '0';
  }
}

char* get_size_specifier(unsigned char w){
  if (w){
    return "";
  }
  else{
    return "byte";
  }
}

// IF mem = 1 then modrm returned a memory adress, if mem = 0 then it returned a register
size_t get_modrm_mem(unsigned char mod, unsigned char rm, unsigned char* bytes, char w, char* str, char* mem){
  // we calculate the additional length of our instruction
  size_t len = 0;
  // if mod is 11, treated as reg
  if (mod == 0b00000011){
    *mem = 0;
    snprintf(str, 24, "%s", get_reg(w, rm));
  }
  else{
    char disp[6] = {0};
    // if we get this special case, we can already format and all
    if (mod == 0b00000000 && rm == 0b00000110){
      unsigned char disp_high = bytes[3];
      unsigned char disp_low = bytes[2];
      byte_to_hex(disp, disp_high);
      byte_to_hex(disp+2, disp_low);
      snprintf(str, 24, "[%s]", disp);
      len+=2;
    }
    // otherwise, we actually need a proper signed disp (+ or -)
    else{
      if (mod == 0b00000001){
        // here, we have a signed disp which is sign extended to disp_high
        // however, we do not need the data from disp_high since that's all going to be padded with the msb of disp_low
        // little endian means disp_high comes second as always
        // ex 
        // disp low  disp high
        // 01110101  000000000
        unsigned char disp_low = bytes[2];
        if ((disp_low & 0b10000000) == 0b10000000){
          disp[0] = '-';
          byte_to_hex(disp+1, two_complement8(disp_low));
        }
        else{
          disp[0] = '+';
          byte_to_hex(disp+1, disp_low);
        }
        len+=1;
      }
      else if (mod == 0b00000010){
        unsigned char disp_high = bytes[3];
        unsigned char disp_low = bytes[2];
        if ((disp_high & 0b10000000) == 0b10000000){
          disp[0] = '-';
          short bytes = disp_low;
          bytes += disp_high << 8;
          twobyte_to_hex(disp+1, two_complement16(bytes));
        }
        else{
          disp[0] = '+';
          byte_to_hex(disp+1, disp_high);
          byte_to_hex(disp+3, disp_low);
        }
        len+=2;
      }
      remove_leading_zeros(disp, 6);
      // once we have the disp, we consider all cases for different formats
      if ((rm & 0b00000100) == 0b00000100){
        char* reg;
        if (rm == 0b00000100){
          reg = "si";
        }
        else if (rm == 0b00000101){
          reg = "di";
        }
        else if (rm == 0b00000110){
          reg = "bp";
        }
        else if (rm == 0b00000111){
          reg = "bx";
        }
        if (mod == 0b00000000){
          snprintf(str, 24, "[%s]", reg);
        }
        else{
          snprintf(str, 24, "[%s%s]", reg, disp);
        }
      }
      else{
        char* reg1;
        char* reg2;
        if ((rm & 0b00000010) == 0b00000010){
          reg1 = "bp";
        }
        else{
          reg1 = "bx";
        }
        if ((rm & 0b00000001) == 0b00000001){
          reg2 = "di";
        }
        else{
          reg2 = "si";
        }
        if (mod == 0b00000000){
          snprintf(str, 24, "[%s+%s]", reg1, reg2);
        }
        else{
          snprintf(str, 24, "[%s+%s%s]", reg1, reg2, disp);
        }
      }
    }
    *mem = 1;
  }
  return len;
}

size_t get_modrm(unsigned char mod, unsigned char rm, unsigned char* bytes, char w, char* str){
  char m;
  return get_modrm_mem(mod, rm, bytes, w, str, &m);
}

char* get_ax_al(unsigned char w){
  if (w){
    return "ax";
  }
  else{
    return "al";
  }
}

size_t get_first_instruction(unsigned char* bytes, 
                             char** prefix_lock, char** prefix_rep, char** prefix_segovr, int* prefix_segovr_pos,
                             char** mnemonic, char** size_spec, char* destination, char* source,
                             int i, size_t byte_count){
  int opcode = -1;
  int has_rep = -1;
  size_t len = 0;
  size_t opcode_index = 0;
  //
  unsigned char op = bytes[opcode_index];
  unsigned char modrm;
  unsigned char z;

  // First, we check if there is a prefix
  // If no prefix is found, it will instantly break out of the loop
  // If lock is found, the text is simply set for lock
  // If rep is found, the default rep text is set so that in case of an illegal instruction, rep will still show (desired behavior)
  // z is also set as well as a boolean indicating the rep prefix was present, so we can figure out the final text later
  // If segovr is found, we get the text and the default position to SEGOVR_POS_NONE, that way it will not display in case 
  // of an illegal instruction (desired behavior)
  // Any prefix also increases the length of the instruction by one and increases the index of the opcode by one
  // need to loop 3 times (cus 3 prefixes at most (1 of each type at most))
  // lock doesnt need any fancy checks and will always go first in the final instruction
  // rep goes after lock and needs to check the opcode to figure out what it's text will be
  // segoverride goes in front of whichever operand is a memory address so it needs an additional variable to tell
  // the formatter which operand to go in front of
  while (opcode_index <= 3){
    if ((op & pref_table[0][1])  == pref_table[0][2]){
      *prefix_lock = "lock";
      len+=1;
      opcode_index+=1;
      op = bytes[opcode_index];
    }
    else if ((op & pref_table[1][1])  == pref_table[1][2]){
      has_rep = 1;
      *prefix_rep = "rep";
      z = op & 0b00000001;
      len+=1;
      opcode_index+=1;
      op = bytes[opcode_index];
    }
    else if ((op & pref_table[2][1])  == pref_table[2][2]){
      unsigned char seg = (op & 0b00011000) >> 3;
      *prefix_segovr = get_seg(seg);
      *prefix_segovr_pos = SEGOVR_POS_NONE;
      len+=1;
      opcode_index+=1;
      op = bytes[opcode_index];
    }
    else{
      break;
    }
  }

  // Then, we get the opcode
  // This first loop is the global opcode table
  for (size_t i=0; i<NB_OPCODE_TABLE; i++){
    if ((op & opcode_table[i][1])  == opcode_table[i][2]){
      opcode = opcode_table[i][0];
      len += opcode_table[i][3];
      break;
    }
  }
  // These subsequent loops wrapped in if statements are specific opcode tables for when the first byte is the same
  // And we have to check the second byte
  // PUSH_RM, INC_RM, DEC_RM, CALL_SEG_IDIR, CALL_ISEG_IDIR, JMP_SEG_IDIR, JMP_ISEG_IDIR 
  if ((op&maska) == 0b11111110){
    for (size_t i=0; i<NB_P_TABLE; i++){
      if ((bytes[1] & p_table[i][1])  == p_table[i][2]){
        opcode = p_table[i][0];
        len += p_table[i][3];
        break;
      }
    }
  }
  // ADD, ADC, SUB, SSB, CMP, AND, OR, XOR _I_RM 
  else if ((op&maskb) == 0b10000000){
    for (size_t i=0; i<NB_A_TABLE; i++){
      if ((bytes[1] & a_table[i][1])  == a_table[i][2]){
        opcode = a_table[i][0];
        len += a_table[i][3];
        break;
      }
    }
  }
  // NEG, MUL, IMUL, DIV, IDIV, NOT, TEST_I_RM
  else if ((op&maska) == 0b11110110){
    for (size_t i=0; i<NB_M_TABLE; i++){
      if ((bytes[1] & m_table[i][1])  == m_table[i][2]){
        opcode = m_table[i][0];
        len += m_table[i][3];
        break;
      }
    }
  }
  // SHL_SAL, SHR, SAR, ROL, ROR, RCL, RCR 
  else if ((op&maskb) == 0b11010000){
    for (size_t i=0; i<NB_S_TABLE; i++){
      if ((bytes[1] & s_table[i][1])  == s_table[i][2]){
        opcode = s_table[i][0];
        len += s_table[i][3];
        break;
      }
    }
  }


  if (opcode == -1){
      *mnemonic = "(undefined)";
      return 1;
  }
  *mnemonic = get_mnemonic(opcode);

  // Once we found the opcode, we can find the length and dst/src with some analysis, and also which bytes the fields correspond to
  unsigned char w;
  unsigned char d;
  unsigned char mod;
  unsigned char reg;
  unsigned char rm;
  unsigned char s;
  unsigned char v;

  // mem is used to indicate if modrm returned a memory address 
  // it is useful to determine the operand of the segment prefix
  // also used for modrm instructions that have no register operand, if mem = 1 then the size is ambiguous and we need a 
  // size specifier
  char mem = 0;

  // this is by default, if the opcode contains modrm it will always be at byte 1
  // if the opcode does not contain modrm this line is inconsequential
  modrm = bytes[opcode_index+1];

  switch(opcode){
    case ESC:
      mod = (modrm & 0b11000000) >> 6;
      rm = (modrm & 0b00000111);
      snprintf(destination, 24, "Not Implemented");
      break;
    case MOV_I_R:
      w = (op & 0b00001000) >> 3;
      reg = (op & 0b00000111);
      snprintf(destination, 24, "%s", get_reg(w, reg));
      len+=get_formatted_immediate_w(source, bytes[len], bytes[len+1], w);
      break;
    case MOV_RM_SR:
    case MOV_SR_RM:
      mod = (modrm & 0b11000000) >> 6;
      reg = (modrm & 0b00011000) >> 3;
      rm = (modrm & 0b00000111);
      if (opcode == MOV_RM_SR){
        snprintf(destination, 24, "%s", get_seg(reg));
        len += get_modrm_mem(mod, rm, bytes, 1, source, &mem);
        if (mem){
          *prefix_segovr_pos = SEGOVR_POS_SRC;
        }
      }
      else{
        len += get_modrm_mem(mod, rm, bytes, 1, destination, &mem);
        snprintf(source, 24, "%s", get_seg(reg));
        if (mem){
          *prefix_segovr_pos = SEGOVR_POS_DST;
        }
      }
      break;
    case PUSH_SR:
    case POP_SR:
      reg = (op & 0b00111000) >> 3;
      snprintf(destination, 24, "%s", get_seg(reg));
      break;
    case XCHG_RM_R:
    case TEST_RM_R:
      w = (op & 0b00000001);
      mod = (modrm & 0b11000000) >> 6;
      reg = (modrm & 0b00111000) >> 3;
      rm = (modrm & 0b00000111);
      len += get_modrm_mem(mod, rm, bytes, w, destination, &mem);
      snprintf(source, 24, "%s", get_reg(w, reg)); 
      if (mem){
        *prefix_segovr_pos = SEGOVR_POS_DST;
      }
      break;
    case LEA:
    case LDS:
    case LES:
      mod = (modrm & 0b11000000) >> 6;
      reg = (modrm & 0b00111000) >> 3;
      rm = (modrm & 0b00000111);
      snprintf(destination, 24, "%s", get_reg16(reg)); 
      len += get_modrm_mem(mod, rm, bytes, 1, source, &mem);
      if (mem){
        *prefix_segovr_pos = SEGOVR_POS_SRC;
      }
      break;
    case PUSH_R:
    case POP_R:
    case XCHG_R_A:
    case INC_R:
    case DEC_R:
      reg = (op & 0b00000111);
      if (opcode == XCHG_R_A){
        snprintf(source, 24, "ax");
      }
      snprintf(destination, 24, "%s", get_reg16(reg)); 
      break;
    case SHL_SAL:
    case SHR:
    case SAR:
    case ROL:
    case ROR:
    case RCL:
    case RCR:
      // NEED SIZE SPECIFIER
      w = (op & 0b00000001);
      v = (op & 0b00000010) >> 1;
      mod = (modrm & 0b11000000) >> 6;
      rm = (modrm & 0b00000111);
      len+=get_modrm_mem(mod, rm, bytes, w, destination, &mem);
      if (v){
        snprintf(source, 24, "cl");
      }
      else{
        snprintf(source, 24, "1");
      }
      // needs size specifier if the destination is an address because we can't know the size implicitely
      if(mem){
        *size_spec = get_size_specifier(w);
        *prefix_segovr_pos = SEGOVR_POS_DST;
      }
      break;
    case ADD_I_RM:
    case ADC_I_RM:
    case SUB_I_RM:
    case SBB_I_RM:
    case CMP_I_RM:
      // NEED SPECIFIER
      w = (op & 0b00000001);
      s = (op & 0b00000010) >> 1;
      mod = (modrm & 0b11000000) >> 6;
      rm = (modrm & 0b00000111);
      char mem = 0;
      len+=get_modrm_mem(mod, rm, bytes, w, destination, &mem);
      len+=get_formatted_immediate_sw(source, bytes[len], bytes[len+1], s, w); 
      if (w ==0 || s == 1){
        remove_leading_zeros(source, 24);
      }
      if(mem){
        *size_spec = get_size_specifier(w);
        *prefix_segovr_pos = SEGOVR_POS_DST;
      }
      break;
    case PUSH_RM:
    case POP_RM:
    case CALL_SEG_IDIR:
    case CALL_ISEG_IDIR:
    case JMP_SEG_IDIR:
    case JMP_ISEG_IDIR:
      mod = (modrm & 0b11000000) >> 6;
      rm = (modrm & 0b00000111);
      len+=get_modrm_mem(mod, rm, bytes, 1, destination, &mem);
      if(mem){
        *prefix_segovr_pos = SEGOVR_POS_DST;
      }
      break;
    case MOV_RM_R:
    case ADD_RM_R:
    case ADC_RM_R:
    case SUB_RM_R:
    case SBB_RM_R:
    case CMP_RM_R:
    case AND_RM_R:
    case OR_RM_R:
    case XOR_RM_R:
      d = (op & 0b00000010) >> 1;
      w = (op & 0b00000001);
      mod = (modrm & 0b11000000) >> 6;
      reg = (modrm & 0b00111000) >> 3;
      rm = (modrm & 0b00000111);
      if (d){
        snprintf(destination, 24, "%s", get_reg(w, reg));
        len+=get_modrm_mem(mod, rm, bytes, w, source, &mem);
        if(mem){
          *prefix_segovr_pos = SEGOVR_POS_SRC;
        }
      }
      else{
        len+=get_modrm_mem(mod, rm, bytes, w, destination, &mem);
        snprintf(source, 24, "%s", get_reg(w, reg));
        if(mem){
          *prefix_segovr_pos = SEGOVR_POS_DST;
        }
      }
      break;
    case NOT:
    case NEG:
    case INC_RM:
    case DEC_RM:
    case MUL:
    case IMUL:
    case DIV:
    case IDIV:
    case MOV_I_RM:
    case AND_I_RM:
    case TEST_I_RM:
    case OR_I_RM:
    case XOR_I_RM:
      // NEED SPECIFIER
      w = (op & 0b00000001);
      mod = (modrm & 0b11000000) >> 6;
      rm = (modrm & 0b00000111);
      len+=get_modrm_mem(mod, rm, bytes, w, destination, &mem);
      if (opcode == MOV_I_RM || opcode == AND_I_RM || opcode == TEST_I_RM || 
        opcode == OR_I_RM || opcode == XOR_I_RM){
        len+=get_formatted_immediate_w(source, bytes[len], bytes[len+1], w); 
        if (w == 0){
          remove_leading_zeros(source, 24);
        }
      }
      if(mem){
        *prefix_segovr_pos = SEGOVR_POS_DST;
        *size_spec = get_size_specifier(w);
      }
      break;
    case MOV_A_M:
      w = (op & 0b00000001);
      get_formatted_immediate16(destination, bytes[len], bytes[len+1]);
      snprintf(source, 24, "%s", get_ax_al(w));
      break;
    case MOV_M_A: 
      w = (op & 0b00000001);
      get_formatted_immediate16(source, bytes[len], bytes[len+1]);
      snprintf(destination, 24, "%s", get_ax_al(w));
      break;
    case ADD_I_A:
    case ADC_I_A:
    case SUB_I_A:
    case SBB_I_A:
    case CMP_I_A:
    case AND_I_A:
    case TEST_I_A:
    case OR_I_A:
    case XOR_I_A:
      w = (op & 0b00000001);
      len+=get_formatted_immediate_w(source, bytes[len], bytes[len+1], w);
      snprintf(destination, 24, "%s", get_ax_al(w));
      if (w == 0){
        remove_leading_zeros(source, 24);
      }
      break;
    case IN_VARIABLE:
      w = (op & 0b00000001);
      snprintf(destination, 24, "%s", get_ax_al(w));
      snprintf(source, 24, "dx");
      break;
    case OUT_VARIABLE:
      w = (op & 0b00000001);
      snprintf(source, 24, "%s", get_ax_al(w));
      snprintf(destination, 24, "dx");
      break;
    case OUT_FIXED:
      w = (op & 0b00000001);
      snprintf(source, 24, "%s", get_ax_al(w));
      get_formatted_immediate8(destination, bytes[len]);
      len+=1;
      break;
    case IN_FIXED:
      w = (op & 0b00000001);
      snprintf(destination, 24, "%s", get_ax_al(w));
      get_formatted_immediate8(source, bytes[len]);
      len+=1;
      break;
    case MOVSB:
    case MOVSW:
    case CMPSB:
    case CMPSW:
    case SCASB:
    case SCASW:
    case LODSB:
    case LODSW:
    case STOSB:
    case STOSW:
      w = (op & 0b00000001);
      if (has_rep == 1 && opcode == CMPSB || opcode == CMPSW || opcode == SCASB || opcode == SCASW){
        if (z){
          *prefix_rep = "repe";
        }
        else{
          *prefix_rep = "repne";
        }
      }
      // Nothing to do here, no operand
      break;
    case RET_SEG:
    case RET_ISEG:
    case CLC:
    case CMC:
    case STC:
    case CLD:
    case STD:
    case CLI:
    case STI:
    case HLT:
    case WAIT:
    case LOCK:
    case AAA:
    case BAA:
    case AAS:
    case DAS:
    case AAM:
    case AAD:
    case CBW:
    case CWD:
    case INTO:
    case IRET:
    case XLAT:
    case LAHF:
    case SAHF:
    case PUSHF:
    case POPF:
      // Nothing to do here, no operand
      break;
    case CALL_SEG_DIR:
    case JMP_SEG_DIR:
      // 16bit signed destination
      // the disp is added onto the instruction's end byte (not start byte) so it's len+i
      get_formatted_label_addr16(destination, bytes[opcode_index+1], bytes[opcode_index+2], len+i);
      break;
    case JMP_SSEG_DIR:
      get_formatted_label_addr8(destination, bytes[opcode_index+1], len+i);
      break;
    case CALL_ISEG_DIR:
    case JMP_ISEG_DIR:
      break;
    case RET_SEG_I_SP:
    case RET_ISEG_I_SP:
      get_formatted_immediate16(destination, bytes[opcode_index+1], bytes[opcode_index+2]);
      break;
    case INT_TYPE_SPEC:
      get_formatted_immediate8(destination, bytes[len]);
      len+=1;
      break;
    case INT_TYPE_THREE:
      break;
    case JE_JZ:
    case JNL_JGE:
    case JL_JNGE:
    case JLE_JNG:
    case JB_JNAE:
    case JBE_JNA:
    case JP_JPE:
    case JO:
    case JS:
    case JNE_JNZ:
    case JNLE_JG:
    case JNB_JAE:
    case JNBE_JA:
    case JNP_JPO:
    case JNO:
    case JNS:
    case JCXZ:
    case LOOP:
    case LOOPZ_LOOPE:
    case LOOPNZ_LOOPNE:
      get_formatted_label_addr8(destination, bytes[opcode_index+1], len+i);
      break;
  }
  
  // if the length (number of bytes of the instruction) was more than the bytes array should have contained (last instruction for example)
  // then mnemonic is undefined (because that means we got it wrong either way) and dst and src are brought back to 0
  if (len > byte_count){
    *mnemonic = "(undefined)";
    *prefix_lock = "";
    *prefix_rep = "";
    *prefix_segovr = "";
    destination[0] = 0;
    source[0] = 0;
    return byte_count;
  }

  // Finally, we can return the length in bytes of the instruction
  return len;
}

void format_instruction(size_t program_index, size_t instruction_size, unsigned char* bytes,
                        char* prefix_lock, char* prefix_rep, char* prefix_segovr, int prefix_segovr_pos,
                        char* mnemonic, char* size_spec, char* destination, char* source, 
                        char* buf, size_t buf_size){
  // the most an instruction can be is 7 bytes; prefix;opcode;modrm;disp(2);data(2) + 1 char to terminate
  char hex_instruction[15];
  for (size_t i=0; i<instruction_size; i++){
    byte_to_hex(hex_instruction+i*2, *(bytes+i));
  }
  hex_instruction[instruction_size*2] = 0;
  snprintf(buf, buf_size, "%04zx: %-14s", program_index, hex_instruction);
  if (prefix_lock[0]){
    strcat(buf, prefix_lock);
    strcat(buf, " ");
  }
  if (prefix_rep[0]){
    strcat(buf, prefix_rep);
    strcat(buf, " ");
  }
  strcat(buf, mnemonic);
  if (size_spec[0]){
    strcat(buf, " ");
    strcat(buf, size_spec);
  }
  if (destination[0]){
    strcat(buf, " ");
    if (prefix_segovr_pos == SEGOVR_POS_DST){
      strcat(buf, prefix_segovr);
      strcat(buf, ":");
    }
    strcat(buf, destination);
    if (source[0]){
      strcat(buf, ",");
    }
  }
  if (source[0]){
    strcat(buf, " ");
    if (prefix_segovr_pos == SEGOVR_POS_SRC){
      strcat(buf, prefix_segovr);
    }
    strcat(buf, source);
  }

  /*
  if (strcmp(destination, "") == 0 && strcmp(source, "") == 0){
    snprintf(buf, buf_size, "%04zx: %-13s %s%s", program_index, hex_instruction, prefix, mnemonic);
  }
  else if (strcmp(source, "") == 0){
    snprintf(buf, buf_size, "%04zx: %-13s %s%s%s %s", program_index, hex_instruction, prefix, mnemonic, size_spec, destination);
  }
  else if (strcmp(destination, "") == 0){
    snprintf(buf, buf_size, "%04zx: %-13s %s%s%s %s", program_index, hex_instruction, prefix, mnemonic, size_spec, source);
  }
  else{
    snprintf(buf, buf_size, "%04zx: %-13s %s%s%s %s, %s", program_index, hex_instruction, prefix, mnemonic, size_spec, destination, source);
  }
  */
}
