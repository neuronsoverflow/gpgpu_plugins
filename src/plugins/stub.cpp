#include <stdio.h>  /* printf  */
#include <time.h>   /* clock_t */
#include <string.h> /* strcpy  */
#include "constants.h"

int setParams(const char* buffer);
int getParams(char* buffer, int bufferSize);
int run();
char* queryParamInfo();
clock_t getRunTime();

/* globals */
char params[][256] = { "input1", "input2", "output" };
const int NUM_ARGS = (sizeof params / sizeof params[0]);
clock_t total_t = 0;


/******************************************************************************
*
******************************************************************************/
int setParams(const char* buffer)
{
   return OK;
}

/******************************************************************************
*
******************************************************************************/int getParams(char*buffer, int bufferSize)
{
   return OK;
}

/******************************************************************************
*
******************************************************************************/int run()
{
   clock_t start_t;
   clock_t end_t;
   start_t = clock();

      end_t = clock();
   total_t = (end_t - start_t) * 1000 / CLOCKS_PER_SEC;
   return OK;
}

/******************************************************************************
* - does not use the DELIM to return, uses comma instead
* - if no params, may return NULL or ""
******************************************************************************/char* queryParamInfo()
{
   return "inputFile1,inputFile2,outputFile";
}

/******************************************************************************
* returns how many milliseconds it took to run the main program
******************************************************************************/clock_t getRunTime()
{
   return total_t;
}
