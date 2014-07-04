#include <stdio.h>
#include <cuda.h>

typedef struct {
   int width;
   int height;
   float* elements;
} Matrix;

// Kernel that executes on the CUDA device
// http://stackoverflow.com/questions/18815489/cuda-tiled-matrix-matrix-multiplication-with-shared-memory-and-matrix-size-whic
template <int BLOCK_SIZE>
__global__ void matrixMulCUDA(Matrix* A, Matrix* B, Matrix* C)
{
   float CValue = 0;
   int row = blockIdx.y * BLOCK_SIZE + threadIdx.y;
   int col = blockIdx.x * BLOCK_SIZE + threadIdx.x;

   __shared__ float As[BLOCK_SIZE][BLOCK_SIZE];
   __shared__ float Bs[BLOCK_SIZE][BLOCK_SIZE];

   for (int k = 0; k < (BLOCK_SIZE + A->width - 1)/BLOCK_SIZE; k++)
   {
      if (k*BLOCK_SIZE + threadIdx.x < A->width && row < A->height)
         As[threadIdx.y][threadIdx.x] = A->elements[row*A->width + k*BLOCK_SIZE + threadIdx.x];
      else
         As[threadIdx.y][threadIdx.x] = 0.0;

      if (k*BLOCK_SIZE + threadIdx.y < B->width && col < B->height)
         Bs[threadIdx.y][threadIdx.x] = B->elements[(k*BLOCK_SIZE + threadIdx.y)*B->height + col];
      else
         Bs[threadIdx.y][threadIdx.x] = 0.0;

      __syncthreads();

      for (int n = 0; n < BLOCK_SIZE; ++n)
         CValue += As[threadIdx.y][n] * Bs[n][threadIdx.x];

      __syncthreads();
   }

   if (row < C->height && col < C->width)
      C->elements[((blockIdx.y * blockDim.y + threadIdx.y)*C->width)+(blockIdx.x*blockDim.x)+threadIdx.x] = CValue;
}


void allocateDeviceMatrix(Matrix* hostMatrix, Matrix* deviceMatrix)
{
   if (cudaMalloc(&deviceMatrix, sizeof(Matrix)) != cudaSuccess)
      printf("FAILED TO ALLOCATE MATRIX ON DEVICE!\n");
   deviceMatrix->width  = hostMatrix->width;
   deviceMatrix->height = hostMatrix->height;

   int size = hostMatrix->width * hostMatrix->height * sizeof(float);
   if (cudaMalloc(&(deviceMatrix->elements), size) != cudaSuccess)
      printf("FAILED TO ALLOCATE MATRIX ELEMENTS ON DEVICE!\n");

   if (cudaMemcpy(deviceMatrix, hostMatrix, size, cudaMemcpyHostToDevice) != cudaSuccess)
      printf("FAILED TO COPY MATRIX ELEMENTS TO DEVICE!\n");
}

// C = A * B
// this function allocates the matrices on the device, execute the kernel, and
// then store the result back in C
void matrixMulOnDevice(Matrix* A, Matrix* B, Matrix* C)
{
   // query the Device and decide on the block size
   int devID = 0; // the default device ID
   cudaError_t error;
   cudaDeviceProp deviceProp;
   error = cudaGetDevice(&devID);
   if (error != cudaSuccess)
   {
      printf("cudaGetDevice returned error code %d, line(%d)\n", error, __LINE__);
   }

   error = cudaGetDeviceProperties(&deviceProp, devID);

   if (deviceProp.computeMode == cudaComputeModeProhibited ||
       error != cudaSuccess)
   {
      // print error and return
   }

   // Use a larger block size for Fermi and above
   int block_size = (deviceProp.major < 2) ? 16 : 32;

   // TODO: veviry this dimsA, dimsB, threads, grid
   // not sure what 5*2 or 5*4 is coming from...
   dim3 dimsA(5*2*block_size, 5*2*block_size, 1);
   dim3 dimsB(5*4*block_size, 5*2*block_size, 1);
   dimsA.x = A->height;
   dimsA.y = A->width;
   dimsB.x = B->height;
   dimsB.y = B->width;

   Matrix* d_A = NULL;
   Matrix* d_B = NULL;
   Matrix* d_C = NULL;

   // allocate the matrices on the device
   allocateDeviceMatrix(A, d_A);
   allocateDeviceMatrix(B, d_B);
   allocateDeviceMatrix(C, d_C);

   // setup the execution configuration
   dim3 threads(block_size, block_size);
   dim3 grid(B->height / threads.x, A->width / threads.y);


   // compute the matrix multiplication by executing the kernel
   if (block_size == 16)
      matrixMulCUDA<16><<< grid, threads >>>(d_A, d_B, d_C);
   else
      matrixMulCUDA<32><<< grid, threads >>>(d_A, d_B, d_C);

   // copy the result from device to host
   int mem_size_C = C->height * C->width * sizeof(float);
   error = cudaMemcpy(C->elements, d_C, mem_size_C, cudaMemcpyDeviceToHost);
   if (error != cudaSuccess)
   {
      printf("cudaMemcpy (h_C,d_C) returned error code %d, line(%d)\n", error, __LINE__);
      exit(EXIT_FAILURE);
   }


}
