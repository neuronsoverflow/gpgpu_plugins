//
// CUDA: nvcc --shared -o hello.so hello.c --compiler-options '-fPIC'
//
// Normal C file:
//    gcc -c -Wall -O0 -g -fPIC -I ../ -o hello.o hello.c
//    gcc -shared -o hello.so hello.o
//
#include "constants.h"
#include "pluginHeader.h"
#include <stdio.h>  /* printf  */
#include <string.h> /* strcpy  */
#include <time.h>   /* clock_t */

/* globals */
char params[][256] = {};
const int NUM_ARGS = sizeof params / sizeof params[0];
clock_t total_t = 0;
const char* PARAM_INFO = "";

///////////////////////////// PLUGIN FUNCTIONS ////////////////////////////////

/******************************************************************************
 * run()
 * - computes the matrix multiplication: C = A * B
 * - reads the input files from the parameters: A = params[0], B = params[1]
 * - writes the output result C to the file set in params[2]
 * - if params[3] is set, then display the result to the screen
 ******************************************************************************/
int run()
{
    clock_t start_t;
    clock_t end_t;
    total_t = 0; // reset the clock counter

    start_t = clock();
    printf("Hello World\n");
    end_t = clock();
    total_t = (end_t - start_t) * 1000 / CLOCKS_PER_SEC;

    return OK;
}

/******************************************************************************
 * setParams()
 * - input: DELIM separated buffer
 * - splits the buffer and sets the "params" to the new passed in parameters
 * - this function only works if all the parameters are passed in to be set
 ******************************************************************************/
int setParams(const char* buffer)
{
    int bufferSize = strlen(buffer) + 1; // +1 for '\0'

    // if the buffer is empty, then just return
    if (bufferSize == 1 && NUM_ARGS == 0)
        return OK;

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
 * getParams()
 * - builds a DELIM delimited string of the current parameter values
 * - sets the "buffer" input to the newly built cstring
 ******************************************************************************/
int getParams(char* buffer, int bufferSize)
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
    return OK;           // OK
}

/******************************************************************************
 * returns info / help on how to use this plugin
 ******************************************************************************/
void* displayPluginInfo()
{
    printf("The hello world plugin takes no parameters!\nEnjoy!\n");
    return NULL;
}

/******************************************************************************
 * returns a comma-separated list of the parameter names
 ******************************************************************************/
const char* getParamInfo()
{
    return PARAM_INFO;
}

/******************************************************************************
 * returns the number of arguments this plugin contains
 ******************************************************************************/
const int getNumArgs()
{
    return NUM_ARGS;
}

/******************************************************************************
 * returns how many milliseconds it took to run the main program
 ******************************************************************************/
clock_t getRunTime()
{
    return total_t;
}
