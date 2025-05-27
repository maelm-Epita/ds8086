#include "../include/hex.h"
#include <stddef.h>

char fourbit_to_hex(int b){
  if (b<10){
    return b+'0';
  }
  else{
    return b-10+'a';
  }
}

void byte_to_hex(char* hex, unsigned char byte){
  char h0 = 1; // 4 bits
  char h1 = 1; // 4 bits
  size_t b1 = 0; // decimal 
  size_t b2 = 0; // decimal
  // extract first 4 bits into decimal
  for(size_t i=0; i<4; i++){
    b1 += (byte & (128 >> i)) >> 4;
  }
  // convert
  h0 = fourbit_to_hex(b1);
  // extract last 4 bits into decimal
  for(size_t i=0; i<4; i++){
    b2 += (byte & (8 >> i));
  }
  // convert
  h1 = fourbit_to_hex(b2);
  hex[0] = h0;
  hex[1] = h1;
}
void twobyte_to_hex(char* hex, short byte){
  size_t b1 = 0; // decimal 
  size_t b2 = 0; // decimal
  size_t b3 = 0; // decimal 
  size_t b4 = 0; // decimal
  for(size_t i=0; i<4; i++){
    b1 += (byte & (32768 >> i)) >> 12;
  }
  for(size_t i=0; i<4; i++){
    b2 += (byte & (2048 >> i)) >> 8;
  }
  for(size_t i=0; i<4; i++){
    b3 += (byte & (128 >> i)) >> 4;
  }
  for(size_t i=0; i<4; i++){
    b4 += (byte & (8 >> i));
  }
  hex[0] = fourbit_to_hex(b1);
  hex[1] = fourbit_to_hex(b2);
  hex[2] = fourbit_to_hex(b3);
  hex[3] = fourbit_to_hex(b4);
}

unsigned char two_complement8(unsigned char byte){
  unsigned char cmp = 0;
  for (int i=7; i>=0; i--){
    unsigned char ith_byte = (byte & (1<<i)) >> i;
    unsigned char inverse = 1 - ith_byte;
    cmp += inverse << i;
  }
  return cmp+1;
}

short two_complement16(short bytes){
  short cmp = 0;
  for (int i=15; i>=0; i--){
    unsigned char ith_byte = (bytes & (1<<i)) >> i;
    unsigned char inverse = 1 - ith_byte;
    cmp += inverse << i;
  }
  return cmp+1;
}
