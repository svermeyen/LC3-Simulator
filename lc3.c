#include "lc3.h"

void lc3_init(lc3machine* state) 
{
  // Initialize the lc3 state according to the assignment.
  state->cc = LC3_ZERO;
  state->pc = 0x3000;
  state->halted = 0;
}

void lc3_load(lc3machine* state, const char* program)
{
  /* FIXME: Add in the code to initialize the memory according to the
     contents of the program. */
  // Reread the section on the .obj file format.
  // Note you will have to load the file from disk and do File I/O.

  FILE *fp = fopen(program, "r"); /*Initialize FILE pointer fp, open program in read mode*/ 
  u16 address; /*LC-3 starting address of statement*/ 
  u16 count; /*Number of instructions to be executed*/
  u16 instruction; /*Specific instruction to be loaded into memory*/ 
  short leftByte; /*Moving from big-endian to little-endian*/ 
  short rightByte; /*Grab two bytes and pack them into a short for use instead of reading a u16 and swapping the bits*/ 

  leftByte = (short) fgetc(fp); /*Grab first byte*/ 

  while (leftByte != EOF) /*leftByte is returned as EOF meaning we have reached the End Of File*/ 
  {
    rightByte = (short) fgetc(fp);
    address = (leftByte<<8) | rightByte;  /*Build address short from first 2 bytes of statement*/ 

    leftByte = (short) fgetc(fp);  /*Grab new bytes to determine number of instructions*/ 
    rightByte = (short) fgetc(fp);
    count = (leftByte<<8) | rightByte; /*Build number of instructions*/ 

    for (int i = 0; i<count; i++)
    {
      leftByte = (short) fgetc(fp); /*Grab instruction*/ 
      rightByte = (short) fgetc(fp);
      instruction = (leftByte<<8) | rightByte;
      state->mem[(unsigned short)(address + (unsigned short)i)] = instruction; /*Load the instruction into memory*/ 
    }
    leftByte = (short) fgetc(fp); /*Start next statement or EOF will be returned and we stop*/ 
  }
  fclose(fp); /*All done*/ 
  printf("successful load");
}

void lc3_step_one(lc3machine* state)
{
  // If the machine is not halted
  // Fetch an instruction
  // And call lc3_execute 
  if (state->halted == 0)
  {
    lc3_execute(state, lc3_fetch(state));
  }

}

void lc3_run(lc3machine* state, int num_steps)
{
  if(num_steps == -1)
  {

    while(state->halted == 0)
    {
      lc3_step_one(state);
    }
    /*if (state->halted == 1)*/
    /*{*/
      /*while(1);*/
    /*}*/
  }
  else
  {
    while((state->halted == 0) && (num_steps > 0))
    {
      lc3_step_one(state);
      num_steps--;
    }
  }
}

unsigned short lc3_fetch(lc3machine* state)
{
  /* Think back to when we first started assembly to code this */
  /* What happens during the fetch stage? */
  short memcpy = state->mem[state->pc]; 
  state->pc = state->pc + 1; 
  return (memcpy); /*Return the instruction after fetch*/ 
}

void lc3_execute(lc3machine* state, unsigned short instruction)
{
  u16 DR = (instruction>>9) & 0x7; /*Grab destination register by bit shifting and masking out irrelevant bits*/ 
  u16 SR = (instruction>>9) & 0x7; /*Purely for readbility purposes, denotes Source Register for stores*/ 
  u16 SR1 = (instruction>>6) & 0x7; /*Grab source register 1*/ 
  u16 SR2 = (instruction) & 0x7; /*Grab source register 2*/ 
  u16 BR = (instruction>>6) & 0x7; /*Grab base register, same as SR1, just for readability purposes*/ 
  u16 N = (instruction>>11) & 1; /*Grab negative condition code*/ 
  u16 Z = (instruction>>10) & 1; /*Grab zero cc*/ 
  u16 P = (instruction>>9) & 1; /*Grab positive cc*/ 
  u16 BIT5 = (instruction>>5) & 1; /*Grab bit 5*/ 
  u16 BIT11 = (instruction>>11) & 1; /*Grab bit 11*/ 
  short OFF6 = ((short)instruction) & 0x3F;  /*Grab offset 6*/ 
  short PCOFF9 = (short)((instruction) & 0x1FF); /*Grab PC offset 9*/ 
  short PCOFF11 = ((short)instruction) & 0x7FF; /*Grab PC offset 11*/ 
  short IMM5 = ((short)instruction) & 0x1F; /*Grab first 5 bits*/ 
  u16 OPCODE = (instruction>>12); /*Grab opcode*/ 
  u16 V8 = (instruction) & 0xFF; /*Grab trap vector 8*/

  if (IMM5 > 15) /*IMM5 is negative so we must set it to its proper negative value*/ 
  {
    IMM5 = IMM5 - 32;
  }
  if (PCOFF9 > 255) /*Check for largest positive signed value possible, if larger, it's negative!*/ 
  {
    PCOFF9 = PCOFF9 - 512;
  }
  if (PCOFF11 > 1023)
  {
    PCOFF11 = PCOFF11 - 2048;
  }
  if (OFF6 > 6)
  {
    OFF6 = OFF6 - 64;
  }

  switch (OPCODE)
  {
    case(0): /*BR, Branch*/
      if ((N & (state->cc == LC3_NEGATIVE)) || (Z & (state->cc == LC3_ZERO)) || (P & (state->cc == LC3_POSITIVE)))
    {
      state-> pc = (unsigned short)((short)state->pc + PCOFF9); /*allow negative values to be added to pc*/ 
    }
      break;

    case(1): /*ADD*/ 
      if (BIT5 == 0) /*bit 5 is off,  add SR1 and SR2, store to DR*/ 
    {
      state->regs[DR] = state->regs[SR1] + state->regs[SR2];
      setcc(state, state->regs[DR]);
    }
      else /*bit 5 is on, add SR1 and IMM5, store to DR*/ 
      {
        state->regs[DR] = state->regs[SR1] + IMM5;
        setcc(state, state->regs[DR]);
      }
      break;

    case(2): /*LD, Load*/ 
      state->regs[DR] = state->mem[(unsigned short)(state->pc + PCOFF9)]; /*Access memory with only non-negative values*/ 
      setcc(state, state->regs[DR]);
      break;

    case(3): /*ST, Store*/
      state->mem[(unsigned short)((short)state->pc + PCOFF9)] = state->regs[SR];
      break;

    case(4): /*JSR, Jump to subroutine*/ 
      state->regs[7] = state->pc; /*store pc for return*/ 
      if (BIT11 == 0) /*JSRR, Jump to subroutine from register*/ 
      {
        state->pc = state->regs[BR];
      }
      else /*JSR*/ 
      {
        state-> pc = (unsigned short)((short)state->pc + PCOFF11);
      }
      break;

    case(5): /*AND*/ 
      if (BIT5 == 0)
    {
      state->regs[DR] = state->regs[SR1] & state->regs[SR2];
    }
      else
      {
        state->regs[DR] = state->regs[SR1] & IMM5;
      }
      setcc(state, state->regs[DR]);
      break;

    case(6): /*LDR, load base + offset*/ 
      state->regs[DR] = state->mem[(unsigned short)(state->regs[BR] + OFF6)];
      setcc(state, state->regs[DR]);
      break;

    case(7): /*STR, store base + offset*/ 
      state->mem[(unsigned short)(state->regs[BR] + OFF6)] = state->regs[SR];
      break;

    case(9): /*NOT, bit-wise complement*/
      state->regs[DR] = ~(state->regs[SR1]);
      setcc(state, state->regs[DR]);
      break;

    case(10): /*LDI, load indirect*/ 
      state->regs[DR] = state->mem[(unsigned short)state->mem[(unsigned short)(state->pc + PCOFF9)]];
      setcc(state, state->regs[DR]);
      break;

    case(11): /*STI, store indirect*/ 
      state->mem[(unsigned short)state->mem[(unsigned short)(state->pc + PCOFF9)]] = state->regs[SR];
      break;

    case(12): /*JMP, jump*/ 
      if (BR == 0x7)
    {
      state->pc = (unsigned short) state->regs[7]; /*RET*/ 
    }
      else
      {
        state->pc = (unsigned short) state->regs[BR];  
      }
      break;

    case(14): /*LEA, load effective address*/ 
      state->regs[DR] = ((short)state->pc) + PCOFF9;
      setcc(state, state->regs[DR]);
      break;

    case(15): /*TRAP*/ 
      if ((V8 == (0x20)) || (V8 == (0x21)) || (V8 ==(0x22)) || (V8 == (0x23)) || (V8 == (0x24)) || (V8 == (0x25)))
    {
      lc3_trap(state, V8);
    }
      else
      {
        state->regs[7] = state->pc;
        state->pc = state->mem[(unsigned short)V8];
      }
      break;


  }
}

void lc3_disassemble(unsigned short instruction)
{
  u16 DR = (instruction>>9) & 0x7; /*Grab destination register by bit shifting and masking out irrelevant bits*/ 
  u16 SR = (instruction>>9) & 0x7; /*Purely for readbility purposes, denotes Source Register for stores*/ 
  u16 SR1 = (instruction>>6) & 0x7; /*Grab source register 1*/ 
  u16 SR2 = (instruction) & 0x7; /*Grab source register 2*/ 
  u16 BR = (instruction>>6) & 0x7; /*Grab base register, same as SR1, just for readability purposes*/ 
  u16 N = (instruction>>11) & 1; /*Grab negative condition code*/ 
  u16 Z = (instruction>>10) & 1; /*Grab zero cc*/ 
  u16 P = (instruction>>9) & 1; /*Grab positive cc*/ 
  u16 BIT5 = (instruction>>5) & 1; /*Grab bit 5*/ 
  u16 BIT11 = (instruction>>11) & 1; /*Grab bit 11*/ 
  short OFF6 = ((short)instruction) & 0x3F;  /*Grab offset 6*/ 
  short PCOFF9 = (short)((instruction) & 0x01FF); /*Grab PC offset 9*/ 
  short PCOFF11 = ((short)instruction) & 0x07FF; /*Grab PC offset 11*/ 
  short IMM5 = ((short)instruction) & 0x001F; /*Grab first 5 bits*/ 
  u16 OPCODE = (instruction>>12); /*Grab opcode*/ 
  u16 V8 = (instruction) & 0xFF; /*Grab trap vector 8*/ 
  if (IMM5 > 15) /*IMM5 is negative so we must set it to its proper negative value*/ 
  {
    IMM5 = IMM5 - 32;
  }
  if (PCOFF9 > 255)
  {
    PCOFF9 = PCOFF9 - 512;
  }
  if (PCOFF11 > 1023)
  {
    PCOFF11 = PCOFF11 - 2048;
  }
  if (OFF6 > 6)
  {
    OFF6 = OFF6 - 64;
  }

  switch (OPCODE)
  {
    case(0): /*BR, Branch*/
      if ((N) && (Z) && (P)) /*One check for all condition codes because BR and BRNZP are the same*/ 
    {
      printf("BRNZP %hd\n", PCOFF9);
    }
      else if ((N) && (!Z) && (!P))
      {
        printf("BRN %hd\n", PCOFF9);
      }
      else if ((N) && (Z) && (!P))
      {
        printf("BRNZ %hd\n", PCOFF9);
      }
      else if ((N) && (!Z) && (P))
      {
        printf("BRNP %hd\n", PCOFF9);
      }
      else if ((!N) && (Z) && (!P))
      {
        printf("BRZ %hd\n", PCOFF9);
      }
      else if ((!N) && (Z) && (P))
      {
        printf("BRZP %hd\n", PCOFF9);
      }
      else if ((!N) && (!Z) && (P))
      {
        printf("BRP %hd\n", PCOFF9);
      }
      break;

    case(1): /*ADD*/ 
      if (BIT5 == 0)
    {
      printf("ADD R%hd, R%hd, R%hd\n", DR, SR1, SR2);

    }
      else
      {
        printf("ADD R%hd, R%hd, %hd\n", DR, SR1, IMM5);
      }
      break;

    case(2): /*LD, Load*/ 
      printf("LD R%hd, %hd\n", DR, PCOFF9);
      break;

    case(3): /*ST, Store*/
      printf("ST R%hd, %hd\n", SR, PCOFF9);
      break;

    case(4): /*JSR, Jump to subroutine*/ 
      if (BIT11 == 1)
    {
      printf("JSR %hd\n", PCOFF11);
    }
      else 
      {
        printf("JSRR R%hd\n", BR);
      }
      break;

    case(5): /*AND*/ 
      if (BIT5 == 0)
    {
      printf("AND R%hd, R%hd, R%hd\n", DR, SR1, SR2);
    }
      else
      {
        printf("AND R%hd, R%hd, %hd\n", DR, SR1, IMM5);
      }
      break;

    case(6): /*LDR, load base + offset*/ 
      printf("LDR R%hd, R%hd, %hd\n", DR, BR, OFF6);
      break;

    case(7): /*STR, store base + offset*/ 
      printf("STR R%hd, R%hd, %hd\n", SR, BR, OFF6);
      break;

    case(9): /*NOT, bit-wise complement*/
      printf("NOT R%hd, R%hd\n", DR, SR1);
      break;

    case(10): /*LDI, load indirect*/ 
      printf("LDI R%hd, %hd\n", DR, PCOFF9);
      break;

    case(11): /*STI, store indirect*/ 
      printf("STI R%hd, %hd\n", SR, PCOFF9);
      break;

    case(12): /*JMP, jump*/ 
      if (BR == 0x7)
    {
      printf("RET\n");
    }
      else
      {
        printf("JMP R%d\n", BR);
      }
      break;

    case(14): /*LEA, load effective address*/ 
      printf("LEA R%hd, %hd\n", DR, PCOFF9);
      break;

    case(15): /*TRAP*/ 
      printf("TRAP x%hx\n", V8);
      break;


  }
}

void lc3_trap(lc3machine* state, unsigned char vector8)
{
  short shrt;
  char temp1;
  char temp2;
  short count = 0x0;
  switch (vector8)
  {
    case(0x20): /*GETC*/ 
      shrt = (short) fgetc(stdin);
      state->regs[0] = shrt;
      break;

    case(0x21): /*OUT*/ 
      printf("%c\n", state->regs[0]);
      break;

    case(0x22): /*PUTS*/ 
      shrt = state->mem[(unsigned short)state->regs[0]];
      while (shrt != 0x0000)
      {
        printf("%c\n", shrt);
        count++;
        shrt = state->mem[(unsigned short)(state->regs[0] + count)];
      }
      break;

    case(0x23): /*IN*/ 
      printf("Type: ");
      shrt = (short) fgetc(stdin);
      state->regs[0] = shrt;
      printf("%c\n", shrt);
      break;

    case(0x24): /*PUTSP*/ 
      shrt = state->mem[(unsigned short)state->regs[0]];
      while (shrt != 0x0000)
      {
        temp1 = shrt>>8; /*Store 8 most significant bits from short*/ 
        temp2 = shrt; /*Upper 8 bits fall out and we have 8 least significant bits from short*/ 
        if (temp1 != '\0')
        {
          printf("%c\n", temp1);
        }
        if (temp2 != '\0')
        {
          printf("%c\n", temp2);
        }
        count++;
        shrt = state->mem[(unsigned short)(state->regs[0] + count)];
      }
      break;

    case(0x25): /*HALT*/ 
      state->halted = 1;
      state->pc = state->pc - 1;
      break;
  }

}

void setcc(lc3machine* state, short reg) /*Helper function to setcc*/ 
{
  if (reg < 0)
  {
    state->cc = LC3_NEGATIVE;
  }
  else if (reg == 0)
  {
    state->cc = LC3_ZERO;
  }
  else
  {
    state->cc = LC3_POSITIVE;
  }

}
