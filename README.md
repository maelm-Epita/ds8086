# DS8086 Disassembler

This is a disassembler for the original 8086 instruction set from 1978.
It supports ELF files and AOUT files for both 32 bit and 64 bit systems.
It can be used with later x86-64 extended instruction sets, however some output may be garbage
due to some instructions being undefined.

## Usage : ds {filepath}


Warning : ELF files are disassembled without symbols or relocations