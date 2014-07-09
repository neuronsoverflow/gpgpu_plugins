#include <stdio.h>
#include <helper_cuda.h> // checkCudaErrors()
#include <cuda.h>
#include <cuda_runtime_api.h>
#include <cuda_runtime.h>

typedef struct {
   int width;
   int height;
   float* elements;
} Matrix;

/******************************************************************************
* kernel for multiplying "C = A * B" on the Device
* kernel source: http://stackoverflow.com/a/18856054
******************************************************************************/
template <int BLOCK_SIZE>
__global__ void matrixMulCUDA(const Matrix A, const Matrix B, Matrix C)
{
   float cValue = 0;

   int row = blockIdx.y * BLOCK_SIZE + threadIdx.y;
   int col = blockIdx.x * BLOCK_SIZE + threadIdx.x;

   __shared__ float As[BLOCK_SIZE][BLOCK_SIZE];
   __shared__ float Bs[BLOCK_SIZE][BLOCK_SIZE];

   for (int k = 0; k < (BLOCK_SIZE + A.width - 1) / BLOCK_SIZE; k++)
   {
      if (k * BLOCK_SIZE + threadIdx.x < A.width && row < A.height)
         As[threadIdx.y][threadIdx.x] = A.elements[row * A.width + k * BLOCK_SIZE + threadIdx.x];
      else
         As[threadIdx.y][threadIdx.x] = 0.0;

      if (k * BLOCK_SIZE + threadIdx.y < B.height && col < B.width)
         Bs[threadIdx.y][threadIdx.x] = B.elements[(k * BLOCK_SIZE + threadIdx.y) * B.width + col];
      else
         Bs[threadIdx.y][threadIdx.x] = 0.0;

      __syncthreads();

      for (int n = 0; n < BLOCK_SIZE; ++n)
         cValue += As[threadIdx.y][n] * Bs[n][threadIdx.x];

      __syncthreads();
   }

   if (row < C.height && col < C.width)
      C.elements[((blockIdx.y * blockDim.y + threadIdx.y) * C.width) + (blockIdx.x * blockDim.x) + threadIdx.x] = cValue;
}

/******************************************************************************
* displayTheMatrix()
* - displays the (host) matrix elements
******************************************************************************/
__host__ void displayTheMatrix(Matrix* matrix)
{
   if (matrix)
   {
      int row;
      int col;
      for (row = 0; row < matrix->height; ++row)
      {
         for (col = 0; col < matrix->width; ++col)
            printf("%3.0f ", matrix->elements[row * matrix->width + col]);
         printf("\n");
      }
   }
}

/******************************************************************************
* allocateDeviceMatrix
* - allocates the deviceMatrix with a copy of the host elements
******************************************************************************/
__host__ void allocateDeviceMatrix(const Matrix* hostMatrix, Matrix& deviceMatrix)
{
   deviceMatrix.width  = hostMatrix->width;
   deviceMatrix.height = hostMatrix->height;

   // allocate deviceMatrix.elements on the device
   int size = hostMatrix->width * hostMatrix->height * sizeof(float);
   if (cudaMalloc(&deviceMatrix.elements, size) != cudaSuccess)
   {
      printf("FAILED TO ALLOCATE MATRIX ELEMENTS ON DEVICE!\n");
      exit(EXIT_FAILURE);
   }

   // copy the matrix elements to the device
   if (cudaMemcpy(deviceMatrix.elements, hostMatrix->elements, size, cudaMemcpyHostToDevice) != cudaSuccess)
   {
      printf("FAILED TO COPY MATRIX TO DEVICE!\n");
      exit(EXIT_FAILURE);
   }
}

/******************************************************************************
* freeDeviceMatrix()
* - deallocates the device matrix
******************************************************************************/
__host__ void freeDeviceMatrix(Matrix& matrix)
{
   if (matrix.elements)
   {
      cudaError_t error = cudaFree(matrix.elements);
      if (error != cudaSuccess)
      {
         printf("cudaFree returned error code %d, line(%d)\n", error, __LINE__);
         exit(EXIT_FAILURE);
      }
      matrix.elements = NULL;
   }
}

/******************************************************************************
* matrixMulOnDevice()
* - allocates the matrices on the device
* - call the kernel to compute C = A * B on the device
* - store the results back into C (host)
* - deallocate the device matrices
******************************************************************************/
__host__ void matrixMulOnDevice(const Matrix* A, const Matrix* B, Matrix* C)
{
   // query the Device and decide on the block size
   int devID = 0; // the default device ID
   cudaError_t error;
   cudaDeviceProp deviceProp;
   error = cudaGetDevice(&devID);
   if (error != cudaSuccess)
   {
      printf("cudaGetDevice returned error code %d, line(%d)\n", error, __LINE__);
      exit(EXIT_FAILURE);
   }

   error = cudaGetDeviceProperties(&deviceProp, devID);
   if (deviceProp.computeMode == cudaComputeModeProhibited || error != cudaSuccess)
   {
      printf("CUDA device ComputeMode is prohibited or failed to getDeviceProperties\n");
      return;
   }

   // Use a larger block size for Fermi and above (see compute capability)
   int block_size = (deviceProp.major < 2) ? 16 : 32;

   Matrix d_A;
   Matrix d_B;
   Matrix d_C;

   // allocate the matrices on the device
   allocateDeviceMatrix(A, d_A);
   allocateDeviceMatrix(B, d_B);
   allocateDeviceMatrix(C, d_C);

   // make sure that there are no errors...
   checkCudaErrors(cudaPeekAtLastError());

   // setup the execution configuration
   /* dim3 dimGrid(1, 1); */
   /* dim3 dimBlock(A->width, A->width); */
   // see http://stackoverflow.com/a/19007136 for choice of Block and Grid size
   dim3 dimBlock(block_size, block_size);
   dim3 dimGrid;
   dimGrid.x = (C->width  + dimBlock.x - 1) / dimBlock.x;
   dimGrid.y = (C->height + dimBlock.y - 1) / dimBlock.y;

   //////// debug
   #ifdef DEBUG
   printf("dimBlock(%d, %d, %d)\n", dimBlock.x, dimBlock.y, dimBlock.z);
   printf("dimGrid(%d, %d, %d)\n", dimGrid.x, dimGrid.y, dimGrid.z);
   #endif

   // run the kernel
   if (block_size == 16)
      matrixMulCUDA<16><<< dimGrid, dimBlock >>>(d_A, d_B, d_C);
   else
      matrixMulCUDA<32><<< dimGrid, dimBlock >>>(d_A, d_B, d_C);

   // some kernel error checking...
   checkCudaErrors(cudaPeekAtLastError());
   checkCudaErrors(cudaDeviceSynchronize());

   // copy the matrix result from device to host
   int mem_size_C = C->height * C->width * sizeof(float);
   error = cudaMemcpy(C->elements, d_C.elements, mem_size_C, cudaMemcpyDeviceToHost);
   if (error != cudaSuccess)
   {
      printf("cudaMemcpy (C->elements, d_C.elements) returned error code %d, line(%d)\n", error, __LINE__);
      exit(EXIT_FAILURE);
   }

   // cudaFree()
   freeDeviceMatrix(d_A);
   freeDeviceMatrix(d_B);
   freeDeviceMatrix(d_C);
}
