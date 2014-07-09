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
Matrix* readMatrixFile(const char* filename);
int writeMatrixFile(const char* filename, const Matrix* matrix);
void displayMatrix(const Matrix* matrix);
void deleteMatrix(Matrix*& matrix);

// matrixMulOnDevice is defined in "matrix.cu"
extern void matrixMulOnDevice(const Matrix* A, const Matrix* B, Matrix* C);

/* globals */
char params[][256] = { "inputFileMatrixA.txt", "inputFileMatrixB.txt",
                       "OutputFileMatrixC.txt", "1" };
const int NUM_ARGS = sizeof params / sizeof params[0];
clock_t total_t = 0;
const char* PARAM_INFO = "inputFileMatrixA,inputFileMatrixB,OutputFileMatrixC,displayResult";

// TODO: delete main() after we are done testing this!
int main()
{
   // strcpy(params[0], "inputFileMatrixA.txt");
   // strcpy(params[1], "inputFileMatrixB.txt");
   // strcpy(params[3], "1");
   run();
   return 0;
}

///////////////////////////// MATRIX FUNCTIONS ////////////////////////////////

/******************************************************************************
* readMatrixFile()
* - opens the file for reading
* - allocates the matrix, read the elements, return the allocated matrix
******************************************************************************/
Matrix* readMatrixFile(const char* filename)
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
         rows = atoi(line.c_str() + 5);
      else if (boost::starts_with(line, "cols="))
         cols = atoi(line.c_str() + 5);
   }

   if (rows <= 0 || cols <= 0)
      return NULL;

   elements = new float[rows * cols](); // allocate the array of elements

   // loop, reading al the elements into the array
   int i = 0;
   while (fin >> elements[i++] && (i < (rows * cols)));

   fin.close();

   // allocate and assign the matrix items
   matrix = new Matrix;
   matrix->width = cols;
   matrix->height = rows;
   matrix->elements = elements;

   return matrix;
}

/******************************************************************************
* writeMatrix()
* - writes the matrix to the file
******************************************************************************/
int writeMatrixFile(const char* filename, const Matrix* matrix)
{
   // open the file
   std::ofstream fout(filename);
   if (!fout || !matrix)
   {
      return ERROR;
   }

   // write the 1st couple of header lines
   fout << "rows=" << matrix->height << std::endl
        << "cols=" << matrix->width << std::endl << std::endl;

   int padding = 4; // TODO: dynamically figure out the padding size

   for (int row = 0; row < matrix->height; ++row)
   {
      for (int col = 0; col < matrix->width; ++col)
         fout << std::setw(padding)
              << matrix->elements[row * matrix->width + col]
              << " ";
      fout << std::endl;
   }
   fout.close();
   return OK;
}

/******************************************************************************
* displays the Matrix elements
******************************************************************************/
void displayMatrix(const Matrix* matrix)
{
   if (matrix)
   {
      int row;
      int col;
      for (row = 0; row < matrix->height; ++row)
      {
         for (col = 0; col < matrix->width; ++col)
            std::cout << matrix->elements[row * matrix->width + col] << " ";
         std::cout << std::endl;
      }
   }
}

/******************************************************************************
* deallocates the matrix elements
******************************************************************************/
void deleteMatrix(Matrix*& matrix)
{
   if (matrix)
   {
      if (matrix->elements)
      {
         delete [] matrix->elements;
         matrix->elements = NULL;
      }
      matrix = NULL;
   }
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

   // open and allocate the input Matrix Files
   Matrix* A = readMatrixFile(params[0]);
   Matrix* B = readMatrixFile(params[1]);
   if (!A || !B)
   {
      std::cout << "index: " << params[!A ? 0 : 1] << std::endl;
      std::cout << "Failed to open Matrix file: \"" << params[!A ? 0 : 1]
                << "\"\n";
      return ERROR;
   }

   if (A->width != B->height) // C = A * B = (example): 3x4 * 4x5 = 3x5
   {
      std::cout << "Invalid matrix dimensions!\n"
                << "Can't multiply " << A->height << 'x' << A->width << " * "
                                     << B->height << 'x' << B->width
                << std::endl;
      return ERROR;
   }

   // allocate the output Matrix
   Matrix* C = NULL;
   C = new Matrix;
   C->height = A->height;
   C->width = B->width;
   C->elements = new float[C->height * C->width]();

   #ifdef DEBUG
   std::cout << "Multiplying C = A * B\n";
   std::cout << "A:\n";
   displayMatrix(A);
   std::cout << "B:\n";
   displayMatrix(B);
   #endif

   // compute the result using the GPU
   start_t = clock();
   matrixMulOnDevice(A, B, C); // using CUDA
   end_t = clock();
   total_t = (end_t - start_t) * 1000 / CLOCKS_PER_SEC;

   // write the result to the file
   if (writeMatrixFile(params[2], C) == ERROR)
   {
      std::cout << "Failed to write Matrix to: \"" << params[2] << "\"\n";
   }

   // check if the result should be displayed
   if (atoi(params[3])) // params[3] => "0" or "1"
   {
      std::cout << "result: \n";
      displayMatrix(C);
   }

   // no memory leaks
   deleteMatrix(A);
   deleteMatrix(B);
   deleteMatrix(C);

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
