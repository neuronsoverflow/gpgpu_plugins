//
// CUDA: nvcc --shared -o hello.so hello.c --compiler-options '-fPIC'
//
// Normal C file:
//    gcc -c -Wall -O0 -g -fPIC -I ../ -o hello.o hello.c
//    gcc -shared -o hello.so hello.o
//
#include <stdio.h>  /* printf  */
#include <time.h>   /* clock_t */
#include <string.h> /* strcpy  */
#include "constants.h"
#include "pluginHeader.h"

/* globals */
char params[][256] = { "input1", "input2", "output" };
const int NUM_ARGS = (sizeof params / sizeof params[0]);
clock_t total_t = 0;

/******************************************************************************
*
******************************************************************************/
int setParams(const char* buffer)
{
   int bufferSize = strlen(buffer) + 1; // +1 for '\0'

   // check if the passed in buffer will fit in our params
   if (bufferSize > NUM_ARGS * BUFFER_SIZE)
      return ERROR;

   // count the number of arguments by counting the number of delimiters
   int commas = 0;
   const char* p;
   for (p = buffer; *p; p++)
      if (*p == DELIM)
         commas++;

   if (commas + 1 == NUM_ARGS)
   {
      // make a copy of the input buffer (so strtok doesnt change the original)
      char buf[bufferSize];
      strcpy(buf, buffer);
      char* arg;
      int i;

      // copy the arguments to the params array
      arg = strtok(buf, DELIM_STR);
      for (i = 0; i < NUM_ARGS && arg; ++i)
      {
         strcpy(params[i], arg);
         arg = strtok(NULL, DELIM_STR);
      }

      return OK; // OK
   }
   return ERROR; // NOT_OK
}

/******************************************************************************
*
******************************************************************************/
int getParams(char*buffer, int bufferSize)
{
   int i;

   // count the size needed
   int size = 0;
   for (i = 0; i < NUM_ARGS; i++)
      size += strlen(params[i]);
   size += NUM_ARGS - 1;

   // the size of the array must be big enough
   if (bufferSize < size || bufferSize < 1)
      return ERROR; // ERROR

   // join the arguments into a DELIM separated list
   buffer[0] = '\0';
   for (i = 0; i < NUM_ARGS; i++)
   {
      strcat(buffer, params[i]);
      strcat(buffer, DELIM_STR); // NOTE: could use DC1 as delimeter
   }
   buffer[size] = '\0'; // delete the last inserted comma
   return OK; // OK
}

/******************************************************************************
*
******************************************************************************/int run()
{
   clock_t start_t;
   clock_t end_t;
   start_t = clock();

   printf("Hello World\n");
   end_t = clock();
   total_t = (end_t - start_t) * 1000 / CLOCKS_PER_SEC;
   return OK;
}

/******************************************************************************
* - does not use the DELIM to return, uses comma instead
* - if no params, may return NULL or ""
******************************************************************************/const char* queryParamInfo()
{
   return "inputFile1,inputFile2,outputFile";
}

/******************************************************************************
* returns how many milliseconds it took to run the main program
******************************************************************************/clock_t getRunTime()
{
   return total_t;
}
