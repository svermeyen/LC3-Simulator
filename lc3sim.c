/* LC3 simulator file.
 * Complete the functions!
 * This is a front-end to the code you wrote in lc3.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lc3.h"

void cmd_registers(lc3machine* mach);
void cmd_dump(lc3machine* mach, int start, int end);
void cmd_list(lc3machine* mach, int start, int end);
void cmd_setaddr(lc3machine* mach, int address, short value);
void cmd_setreg(lc3machine* mach, int reg, short value);

/* FIXME: change this to be a good prompt string */
#define PROMPT "Enter a valid command: "

// Don't touch I will use this during grading.
#ifndef NO_MAIN_PLZ
int main(int argc, char **argv) 
{

  const char* prog;

  /* We expect only one argument for the program... */
  if (argc != 2) 
  {
    fprintf(stderr, "Usage: %s file.obj\n", argv[0]);
    return 1;
  }
  /* We want to open the file and make sure that it exists. */
  prog = argv[1];
  if (NULL == fopen(prog, "r"))
  {
    fprintf(stderr, "File does not exist %s\n", argv[1]);
    return 1;
  }

  /* Create a variable of type lc3machine here */
  /* Make a call to lc3_init to initialize your lc3machine */
  lc3machine *state = malloc(sizeof(lc3machine));
  lc3_init(state);
  lc3_load(state, prog);

  /* FIXME: add your name here! */
  printf("LC-3 simulator and debugger\n");
  printf("Written by Scott Vermeyen\n");
  printf("File given %s\n", prog);

  /* Run this loop until we are told to stop going. */
  while (1) 
  {
    printf("%s", PROMPT);
    /* FIXME: Read in user commands and execute them! */

    char buffer[50]; /*buffer to hold user input*/ 
    char *dummy = fgets(buffer, 50, stdin); /*read the user input, last char is NULL char*/ 
    const char delim1[2] = "\n"; /* for remove new line char*/ 
    const char delim2[2] = " ";
    const char delim3[2] = "R"; /*for setreg*/ 
    const char delim4[2] = "x"; /*for delimiting hex values*/ 
    char *instruction;
    if (buffer != NULL) /*crash protection for any unexpected behavior*/ 
    {
      instruction = strtok(buffer, delim1);
    }
    int data = 0;
    int moreData = 0;
    int myBool = -1;

    if (instruction != NULL)
    {

      if (strncmp("step", instruction, 4) == 0)
      {
        dummy = strtok(instruction, delim2); /*dummy call to strtok on string to advance to number of steps*/
        dummy = strtok(NULL, delim2); /*now we have number of steps ready to manipulate*/ 
        if (dummy != NULL) /*if we actually have steps*/ 
        {
          data = atoi(dummy); /*number of steps*/ 
          lc3_run(state, data);
        }
        else /*no steps,  run once*/ 
        {
          lc3_step_one(state);
        }
      }

      else if (strncmp("quit", instruction, 4) == 0)
      {
        free(state);
        exit(0);
      }

      else if (strncmp("continue", instruction, 8) == 0)
      {
        lc3_run(state, -1);
      }

      else if (strncmp("registers", instruction, 9) == 0)
      {
        cmd_registers(state);
      }

      else if (strncmp("dump", instruction, 4) == 0)
      {
        dummy = strtok(instruction, delim4); /*dummy call*/ 
        dummy = strtok(NULL, delim4); /*string containing start value*/
        if (dummy != NULL) /*crash protection against improper dump commands*/ 
        {
          data = (int) strtol(dummy, NULL, 16); /*strtol conversion from hexadecimal string to int*/ 
          myBool = 0;
        }
        dummy = strtok(NULL, delim2); /*string containing end value if it exists*/ 
        if (dummy != NULL) /*if there is an end value*/ 
        {
          moreData = (int) strtol(dummy, NULL, 16); /*strtol conversion from hexadecimal string to int*/ 
          cmd_dump(state, data, moreData);
          myBool = 1; /*check to see if data has been set*/ 
        }
        if ((myBool != 1) && (myBool == 0))  /*condition where start is given but no end*/ 
        {
          cmd_dump(state, data, -1);
        }
      }

      else if (strncmp("list", instruction, 4) == 0)
      {
        dummy = strtok(instruction, delim4); /*dummy call*/ 
        dummy = strtok(NULL, delim4); /*string containing start value*/ 
        if (dummy != NULL) /*crash protection against improper list commands*/ 
        {
          data = (int) strtol(dummy, NULL, 16); /*strtol conversion from hexadecimal string to int*/ 
          myBool = 0; /*check to see if data has been set*/ 
        }
        dummy = strtok(NULL, delim2); /*string containing end value if it exists*/ 
        if (dummy != NULL) /*if there is an end value*/ 
        {
          moreData = (int) strtol(dummy, NULL, 16); /*strtol conversion from hexadecimal string to int*/ 
          cmd_list(state, data, moreData);
          myBool = 1;
        }
        if ((myBool != 1) && (myBool == 0))  /*condition where start is given but no end*/ 
        {
          cmd_list(state, data, -1);
        }

      }

      else if (strncmp("setaddr", instruction, 7) == 0)
      {
        dummy = strtok(instruction, delim4); /*dummy call*/ 
        dummy = strtok(NULL, delim2); /*string containing address*/ 
        if (dummy != NULL) /*crash protection against improper setaddr commands*/ 
        {
          data = (int) strtol(dummy, NULL, 16); /*strtol conversion from hexadecimal string to int*/ 
        }

        dummy = strtok(NULL, delim2); /*string containing value*/ 

        if (dummy != NULL)
        {
          moreData = atoi(dummy); /*retrieving value for use*/ 
          cmd_setaddr(state, data, moreData);
        }
      }

      else if (strncmp("setreg", instruction, 6) == 0)
      {
        dummy = strtok(instruction, delim3); /*dummy call*/ 
        dummy = strtok(NULL, delim2); /*string containing register*/ 
        if (dummy != NULL) /*crash protection against improper setreg commands*/ 
        {
          data = atoi(dummy); /*retrieving register for use*/ 
        }

        dummy = strtok(NULL, delim2); /*string containing value*/ 

        if (dummy != NULL)
        {
          moreData = atoi(dummy); /*retrieving value for use*/ 
          cmd_setreg(state, data, moreData);
        }
      }

    }
  }
  return 0;
}
#endif // IFNDEF NO_MAIN_PLZ

/* cmd_step and cmd_continue 's functionality are provided in lc3_run
   Therefore to execute the step and coninute commands you can just call lc3_run with the correct parameters*/

/* cmd_registers
   Should print out all of the registers and the PC and CC in both hex and signed decimal. The format should be as follows

   R0 dec|hex	R1 dec|hex	R2 dec|hex	R3 dec|hex
   R4 dec|hex	R5 dec|hex	R6 dec|hex	R7 dec|hex
   CC n/z/p
   PC hex

   hex is to be preceded with only an x and you should always print out 4 characters representing the hexadecimal representation the hex letters are CAPITALIZED.
   n/z/p will only be one of the characters n, z, or p
   between each register's information is a single tab.

   Example output 
   R0 0|x0000	R1 -1|xFFFF	R2 2|x0002	R3 32767|x7FFF
   R4 31|x001F	R5 -32768|x8000	R6 6|x0006	R7 11111|x2B67
   CC z
   PC x3000
 */
void cmd_registers(lc3machine* mach)
{
  for (int i =0; i<8; i++)
  {
    if ((i == 3) || (i == 7))
    {
      printf("R%d %hd|x%04hX\n", i, mach->regs[i], mach->regs[i]); /*awesome printf formatting*/ 
    }
    else
    {
      printf("R%d %hd|x%04hX\t", i, mach->regs[i], mach->regs[i]); 
    }
  }
  if (mach->cc == LC3_NEGATIVE) /*checks for CC since it can not be printed straight up*/ 
  {
    printf("CC: N\n");
  }
  else if (mach->cc == LC3_ZERO)
  {
    printf("CC: Z\n");
  }
  else if (mach->cc == LC3_POSITIVE)
  {
    printf("CC: P\n");
  }
  printf("PC: x%04hX\n", mach->pc);
}

/* cmd_dump
   Should print out the contents of memory from start to end
   If end is -1 then just print out memory[start]

   Output format
addr: dec|hex

Example format
x0000: 37|x0025
x0001: 25|x0019
 */
void cmd_dump(lc3machine* mach, int start, int end)
{
  if (end == -1)
  {
    printf("x%04hX: %hd|x%04hX\n", (unsigned short) start, mach->mem[(unsigned short)start], mach->mem[(unsigned short)start]); /*safe casts so we never index out of bounds*/ 
  }
  else
  {
    for (int i = 0; i <= (end - start); i++)
    {
      printf("x%04hX: %hd|x%04hX\n", (unsigned short) (start + i), mach->mem[(unsigned short)(start + i)], mach->mem[(unsigned short)(start + i)]); /*same thing, I never want to input a number that can possibly index out of array*/ 
    }
  }
}

/* cmd_list
   Should interpret the contents of memory from start to end as an assembled instruction
   and disassemble it. e.g. if the data was x1000 then the output will be x3000
   If end is -1 then just disassemble memory[start]

   You will be calling lc3_disassemble to do the actual disassembling!
 */
void cmd_list(lc3machine* mach, int start, int end)
{
  if (end == -1)
  {
    lc3_disassemble(mach->mem[(unsigned short)start]);
  }
  else
  {
    for (int i = 0; i <= (end - start); i++)
    {
      lc3_disassemble(mach->mem[(unsigned short)(start + i)]);
    }
  }
}

/* cmd_setaddr
   Should set a memory address to some value
 */
void cmd_setaddr(lc3machine* mach, int address, short value)
{
  mach->mem[(unsigned short)address] = value;
}

/* cmd_setreg
   Should set a register to some value passed in
 */
void cmd_setreg(lc3machine* mach, int reg, short value)
{
  mach->regs[reg] = value;
}

