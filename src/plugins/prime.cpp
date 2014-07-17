#include <iostream>
#include <ctime>   /* clock_t */
#include <cstring> /* strcpy  */
#include <cstdlib> // atoi, strtoull
#include <fstream> // ifstream, ofstream
#include "constants.h"
#include "pluginHeader.h"
#include <boost/algorithm/string.hpp> // starts_with
#include <iomanip> // setw


// Prime Number functions
typedef unsigned long long int uint64;
int writePrimesFile(const char* filename, const char* primes, uint64 size);
uint64 getPrimesCount(const char* primes, uint64 size);
void displayPrimes(const char* primes, uint64 size);

// genPrimesOnDevice is defined in "primes.cu"
extern void genPrimesOnDevice(char* primes, uint64 limit);

/* globals */
char params[][256] = { "prime_numbers.txt" , "101" };
const int NUM_ARGS = sizeof params / sizeof params[0];
clock_t total_t = 0;
const char* PARAM_INFO = "outputFile,limit";

// TODO: delete main() after we are done testing this!
int main(int argc, char** argv)
{
   if (argc > 1)
      strcpy(params[1], argv[1]);

   return run();
}

////////////////////////////// PRIME FUNCTIONS ////////////////////////////////

/******************************************************************************
* writePrimesFile() - saves the generated prime numbers to the file
******************************************************************************/
int writePrimesFile(const char* filename, const char* primes, uint64 size)
{
   // open the file
   std::ofstream fout(filename);
   if (!fout || !primes || !size)
   {
      return ERROR;
   }

   /*
   // write a comma-separated list of primes
   uint64 count = getPrimesCount(primes, size);
   for (uint64 i = 0; i < size; i++)
   {
      if (primes[i] == 0)
      {
         count--;
         fout << i;
         if (((i + 1) != size) && count)
            fout << ", ";
      }
   }
   fout << "\n";
   */

   // write a newline separated list of primes
   for (uint64 i = 0; i < size; i++)
   {
      if (primes[i] == 0) // a value of '0' means the number is prime,
      {                   // a value of '1' means the number is composite
         fout << i << "\n";
      }
   }

   fout.close();
   return OK;

}

/******************************************************************************
* getPrimesCount() - counts the number of primes
******************************************************************************/
uint64 getPrimesCount(const char* primes, uint64 size)
{
   uint64 count = 0;
   for (uint64 i = 0; i < size; i++)
      if (primes[i] == 0)
         count++;
   return count;
}

/******************************************************************************
* displayPrimes() - displays a comma-separated list of prime numbers
******************************************************************************/
void displayPrimes(const char* primes, uint64 size)
{
   uint64 count = getPrimesCount(primes, size);
   for (uint64 i = 0; i < size; i++)
   {
      if (primes[i] == 0)
      {
         count--;
         std::cout << i;
         if (((i + 1) != size) && count)
            std::cout << ", ";
      }
   }
   std::cout << std::endl;
}

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

   uint64 maxPrime = strtoull(params[1], NULL, 10); // TODO: check for errors
   char* primes;
   maxPrime++; // allow the given parameter to be part of the search

   #ifdef DEBUG
   std::cout << "calling genPrimesOnDevice with maxPrime = " << maxPrime << std::endl;
   #endif

   primes = new (std::nothrow) char[maxPrime]();
   if (primes == NULL)
   {
      std::cout << "Failed to allocate memory for the prime numbers.\n"
                << "The limit parameter might be too large: " << maxPrime
                << std::endl;
      return ERROR;
   }

   start_t = clock();
   genPrimesOnDevice(primes, maxPrime);
   end_t = clock();
   total_t = end_t - start_t;

   // now save to the file
   std::cout << "Found " << getPrimesCount(primes, maxPrime) << " primes"
             << " in " << total_t / (double) CLOCKS_PER_SEC << " seconds.\n"
             << "Writing results to: \"" << params[0]  << "\"\n";
   int result = writePrimesFile(params[0], primes, maxPrime);
   if (result == ERROR)
   {
      std::cout << "Failed to write Primes to: \"" << params[0] << "\"\n";
      std::cout << "Displaying result instead:\n";
      displayPrimes(primes, maxPrime);
   }

   #ifdef DEBUG
   if (result != ERROR)
      displayPrimes(primes, maxPrime);
   #endif

   delete [] primes;

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
* returns info / help on how to use this plugin
******************************************************************************/
void* displayPluginInfo()
{
   printf("welcome! this is the matrix plugin...\nblah\nblah\nblah\n");
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
