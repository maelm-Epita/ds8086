#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <string.h>
#include "../include/instructions.h"
#include "../include/hex.h"


// these 2 functions return the offset in bits from file start to text start and the size of the text
void get_offset_length_aout(FILE* f, long* offset, long* size){
  *offset = 32;
  // SKIP FIRST 8 bytes of header to go to text size
  fseek(f, 8, SEEK_SET);
  // READ 4 bytes (long) 
  fread(size, 4, 1, f);
}

void get_offset_length_elf(FILE* f, long* offset, long* size){
  char bit_fmt = 0;
  long e_shoff = 0;
  short e_shentsize = 0;
  short e_shnum = 0;
  short e_shstrndx = 0;
  long str_offset = 0;
  long str_size = 0;
  fread(&bit_fmt, 1, 1, f);
  if (bit_fmt != 1 && bit_fmt != 2){
    errx(1, "Unknown bit format in header (not 32-bit or 64-bit)\n");
  }
  // seek to e_shoff and read
  // 19 + (8 / 16) bytes
  if (bit_fmt == 1){
    fseek(f, 27, SEEK_CUR);
    fread(&e_shoff, 4, 1, f);
  }
  else{
    fseek(f, 35, SEEK_CUR);
    fread(&e_shoff, 8, 1, f);
  }
  fseek(f, 10, SEEK_CUR);
  fread(&e_shentsize, 2, 1, f);
  fread(&e_shnum, 2, 1, f);
  fread(&e_shstrndx, 2, 1, f);
  // go to str section header
  fseek(f, e_shoff+e_shentsize*e_shstrndx, SEEK_SET);
  // seek to and read size and offset
  if (bit_fmt == 1){
    fseek(f, 16, SEEK_CUR);
    fread(&str_offset, 4, 1, f);
    fread(&str_size, 4, 1, f);
  }
  else{
    fseek(f, 24, SEEK_CUR);
    fread(&str_offset, 8, 1, f);
    fread(&str_size, 8, 1, f);
  }
  // allocate buffer to hold section name strings
  char* names = malloc(str_size);
  // seek to str section and read str_size bytes
  fseek(f, str_offset, SEEK_SET);
  fread(names, 1, str_size, f);
  // start reading headers until the right text is found
  for (int i = 0; i < e_shnum; i++) {
    fseek(f, e_shoff+e_shentsize*i, SEEK_SET);
    long sh_name = 0;
    fread(&sh_name, 4, 1, f);
    char* section_name = names+sh_name;
    // if we found the .txt section we can get size and offset and end the function
    if (strcmp(section_name, ".text") == 0){
      // seek to and read size and offset
      if (bit_fmt == 1){
        fseek(f, 12, SEEK_CUR);
        fread(offset, 4, 1, f);
        fread(size, 4, 1, f);
      }
      else{
        fseek(f, 20, SEEK_CUR);
        fread(offset, 8, 1, f);
        fread(size, 8, 1, f);
      }
      break;
    }
  }
  free(names);
}

int main(int argc, char* argv[]){
  if (argc != 2){
    errx(EXIT_FAILURE, "Usage: ./ds {file path}\n");
  }
  FILE *f;
  f = fopen(argv[1], "rb");
  if (f == NULL){
    errx(EXIT_FAILURE, "Could not open file \"%s\"", argv[1]);
  }
  long txt_size = 0;
  long txt_offset = 0;
  char magic[4] = "";
  fread(magic, 1, 4, f);
  if (magic[0] == 0x7F && magic[1] == 'E' &&
      magic[2] == 'L'  && magic[3] == 'F'){
    get_offset_length_elf(f, &txt_offset, &txt_size);
  }
  else{
    get_offset_length_aout(f, &txt_offset, &txt_size);
  }
  // SKIP FIRST _offset_ bytes of the file to go to text
  fseek(f, txt_offset, SEEK_SET);
  int pg_text_start = ftell(f);
  // instruction by instruction disassemble the file 
  char* mne;
  char* size_spec;
  char *prefix_lock, *prefix_rep, *prefix_segovr;
  // read 8 bytes into the buffer
  unsigned char bytes[8];
  int i=0;
  // repeat until end of text 
  while (i<txt_size){
    // theorizing that the longest an operand can be is [bx+si+xxxx] , just in case ill give my buffers twice that many characters 
    // reset all optional buffers
    char dst[24] = {0};
    char src[24] = {0};
    mne = "";
    size_spec = "";
    prefix_lock = "";
    prefix_rep = "";
    prefix_segovr = "";
    int prefix_segovr_pos = SEGOVR_POS_NONE;
    // read 8 bytes into bytes array (max instruction length)
    fread(&bytes, 1, 8, f);
    size_t remaining_bytes_count = txt_size-i;
    // get first instruction
    size_t instruction_len = get_first_instruction(bytes, &prefix_lock, &prefix_rep, &prefix_segovr, &prefix_segovr_pos,
                                                   &mne, &size_spec, dst, src, i, remaining_bytes_count);
    // the size is usually around 50 characters at most
    char buf[100];
    // format it and print it
    format_instruction(i, instruction_len, bytes, 
                       prefix_lock, prefix_rep, prefix_segovr, prefix_segovr_pos,
                       mne, size_spec, dst, src, buf, 100);
    printf("%s\n", buf);
    // add the length of the instruction to the position in bytes in the file, 
    i+=instruction_len;
    // put the cursor at that offset from the beginning of file
    fseek(f, pg_text_start+i, SEEK_SET);
  }
  fclose(f);
  return 0;
}

