#include <iostream>
#include <ctime>   /* clock_t */
#include <cstring> /* strcpy  */
#include <cstdlib> // atoi
#include <fstream> // ifstream, ofstream
#include "constants.h"
#include "pluginHeader.h"
#include <boost/algorithm/string.hpp> // starts_with
#include <iomanip> // setw

typedef struct {
   int width;
   int height;
   float* elements;
} Matrix;

// Matrix functions
Matrix* readMatrixFile(char* filename);
int writeMatrixFile(char* filename, Matrix* matrix);
void displayMatrix(Matrix* matrix);
void freeMatrix(Matrix* matrix);
extern void matrixMulOnDevice(Matrix* A, Matrix* B, Matrix* C); // in matrix.cu

/* globals */
char params[][256] = { "inputFileMatrixA.txt", "inputFileMatrixB.txt",
                       "OutputFileMatrixC.txt", "0" };
const int NUM_ARGS = (sizeof params / sizeof params[0]);
clock_t total_t = 0;

/**************************** PLUGIN FUNCTIONS *******************************/


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
*   // this will compute C = A * B
******************************************************************************/
int run()
{
   clock_t start_t;
   clock_t end_t;

   // open the Matrix Files
   Matrix* A = readMatrixFile(params[0]);
   Matrix* B = readMatrixFile(params[1]);
   if (!A || !B)
   {
      std::cout << "Failed to open Matrix file: \"" << params[!A ? 0 : 1]
                << "\"\n";
      return ERROR;
   }
   Matrix* C = NULL;

   if (A->width != B->height) // C = A * B = (example): 3x4 * 4x5 = 3x5
      return ERROR;

   C = new Matrix;
   C->height = A->height;
   C->width = B->width;
   C->elements = new float[C->height * C->width];

   start_t = clock();
   matrixMulOnDevice(A, B, C); // run it on the GPU using CUDA
   end_t = clock();
   total_t = (end_t - start_t) * 1000 / CLOCKS_PER_SEC;

   if (writeMatrixFile(params[2], C) == ERROR)
   {
      std::cout << "Failed to write Matrix to: \"" << params[2] << "\"\n";
   }

   if (atoi(params[3])) // display?
      displayMatrix(C);

   // no memory leaks
   freeMatrix(A);
   freeMatrix(B);
   freeMatrix(C);

   return OK;
}

/******************************************************************************
* - does not use the DELIM to return, uses comma instead
* - if no params, may return NULL or ""
******************************************************************************/
const char* queryParamInfo()
{
   return "inputFileMatrixA,inputFileMatrixB,OutputFileMatrixC,displayResult";
}

/******************************************************************************
* returns how many milliseconds it took to run the main program
******************************************************************************/
clock_t getRunTime()
{
   return total_t;
}

/**************************** MATRIX FUNCTIONS *******************************/

/******************************************************************************
*
******************************************************************************/
Matrix* readMatrixFile(char* filename)
{
   // open the file
   std::ifstream fin(filename);
   if (!fin)
      return NULL;

   int rows = 0;
   int cols = 0;
   float* elements = NULL;
   Matrix* matrix = NULL;
   std::string line;

   // read the 1st 2 lines ~ should contain "rows=" and "cols="
   for (int i = 0; i < 2; i++)
   {
      getline(fin, line);
      if (boost::starts_with(line, "rows="))
         rows = atof(line.c_str() + 5);
      else if (boost::starts_with(line, "cols="))
         cols = atof(line.c_str() + 5);
   }

   if (rows <= 0 || cols <= 0)
      return NULL;

   elements = new float[rows * cols];

   // loop, reading al the elements into the array
   int i = 0;
   while (fin >> elements[i++] && (i < (rows * cols)));

   fin.close();

   // assign the matrix items
   matrix = new Matrix;
   matrix->width = cols;
   matrix->height = rows;
   matrix->elements = elements;

   return matrix;
}

/******************************************************************************
*
******************************************************************************/
int writeMatrixFile(char* filename, Matrix* matrix)
{
   // open the file
   std::ofstream fout(filename);
   if (!fout || !matrix)
   {
      return ERROR;
   }

   fout << "rows=" << matrix->height << std::endl
        << "cols=" << matrix->width << std::endl << std::endl;

   int padding = 4;

   for (int row = 0; row < matrix->height; ++row)
   {
      for (int col = 0; col < matrix->width; ++col)
         fout << std::setw(padding) << matrix->elements[row * matrix->width + col]
              << " ";
      fout << std::endl;
   }
   fout.close();
   return OK;
}

/******************************************************************************
*
******************************************************************************/
void displayMatrix(Matrix* matrix)
{
   if (matrix)
   {
      int row;
      int col;
      std::cout << "display:\n";
      for (row = 0; row < matrix->height; ++row)
      {
         for (col = 0; col < matrix->width; ++col)
            std::cout << matrix->elements[row * matrix->width + col] << " ";
         std::cout << std::endl;
      }
   }
}

/******************************************************************************
*
******************************************************************************/
void freeMatrix(Matrix* matrix)
{
   if (matrix)
   {
      delete [] matrix->elements;
      matrix->elements = NULL;
      matrix = NULL;
   }
}
