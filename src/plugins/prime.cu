#include <helper_cuda.h> // checkCudaErrors- NVIDIA_CUDA-6.0_Samples/common/inc
#include <stdio.h>

// typedef unsigned long long int uint64_t;

/******************************************************************************
 * kernel that initializes the 1st couple of values in the primes array.
 ******************************************************************************/
__global__ static void sieveInitCUDA(char* primes)
{
    primes[0] = 1; // value of 1 means the number is NOT prime
    primes[1] = 1; // numbers "0" and "1" are not prime numbers
}

/******************************************************************************
 * kernel for sieving the even numbers starting at 4.
 ******************************************************************************/
__global__ static void sieveEvenNumbersCUDA(char* primes, uint64_t max)
{
    uint64_t index = blockIdx.x * blockDim.x * 2 + threadIdx.x + threadIdx.x + 4;
    if (index < max)
        primes[index] = 1; // mark off the even numbers
}

/******************************************************************************
 * kernel for finding prime numbers using the sieve of eratosthenes
 * - primes: an array of bools. initially all numbers are set to "0".
 *           A "0" value means that the number at that index is prime.
 * - max: the max size of the primes array
 * - maxRoot: the sqrt of max (the other input). we don't wanna make all threads
 *   compute this over and over again, so it's being passed in
 ******************************************************************************/
__global__ static void sieveOfEratosthenesCUDA(char* primes, uint64_t max, const uint64_t maxRoot)
{
    // get the starting index, sieve only odds starting at 3
    // block 0: 3,   5,  7,  9, 11, 13, ...,  65
    // block 1: 67, 69, 71, 73, 75, 77, ..., 129
    uint64_t index = blockIdx.x * blockDim.x * 2 + threadIdx.x + threadIdx.x + 3;

    // make sure index won't go out of bounds, also don't start the execution
    // on numbers that are already composite
    if (index <= maxRoot && primes[index] == 0)
    {
        // mark off the composite numbers
        for (int j = index * index; j < max; j += index)
        {
            primes[j] = 1;
        }
    }
}

/******************************************************************************
 * checkDevice()
 ******************************************************************************/
__host__ int checkDevice()
{
    // query the Device and decide on the block size
    int devID = 0; // the default device ID
    cudaError_t error;
    cudaDeviceProp deviceProp;
    error = cudaGetDevice(&devID);
    if (error != cudaSuccess)
    {
        printf("CUDA Device not ready or not supported\n");
        printf("%s: cudaGetDevice returned error code %d, line(%d)\n", __FILE__, error, __LINE__);
        exit(EXIT_FAILURE);
    }

    error = cudaGetDeviceProperties(&deviceProp, devID);
    if (deviceProp.computeMode == cudaComputeModeProhibited || error != cudaSuccess)
    {
        printf("CUDA device ComputeMode is prohibited or failed to getDeviceProperties\n");
        return EXIT_FAILURE;
    }

    // Use a larger block size for Fermi and above (see compute capability)
    return (deviceProp.major < 2) ? 16 : 32;
}

/******************************************************************************
 * genPrimesOnDevice
 * - inputs: limit - the largest prime that should be computed
 *           primes - an array of size [limit], initialized to 0
 ******************************************************************************/
__host__ void genPrimesOnDevice(char* primes, uint64_t max)
{
    int blockSize = checkDevice();
    if (blockSize == EXIT_FAILURE)
        return;

    char* d_Primes = NULL;
    int sizePrimes = sizeof(char) * max;
    uint64_t maxRoot = sqrt(max);

    // allocate the primes on the device and set them to 0
    checkCudaErrors(cudaMalloc(&d_Primes, sizePrimes));
    checkCudaErrors(cudaMemset(d_Primes, 0, sizePrimes));

    // make sure that there are no errors...
    checkCudaErrors(cudaPeekAtLastError());

    // setup the execution configuration
    dim3 dimBlock(blockSize);
    // dim3 dimGrid((maxRoot + dimBlock.x) / dimBlock.x);
    dim3 dimGrid(ceil((maxRoot + dimBlock.x) / (double)dimBlock.x) / (double)2);
    dim3 dimGridEvens(ceil((max + dimBlock.x) / (double)dimBlock.x) / (double)2);

    // if for some reason, the user wants to find primes below 32 @.@ ...
    if (dimGridEvens.x < 1)
        dimGridEvens.x = 1; // make sure this won't be a '0'

//////// debug
#ifdef DEBUG
    printf("max: %llu\n", max);
    printf("maxRoot: %llu\n", maxRoot);
    printf("dimBlock(%d, %d, %d)\n", dimBlock.x, dimBlock.y, dimBlock.z);
    printf("dimGrid(%d, %d, %d)\n", dimGrid.x, dimGrid.y, dimGrid.z);
    printf("dimGridEvens(%d, %d, %d)\n", dimGridEvens.x, dimGridEvens.y, dimGridEvens.z);
#endif

    // call the kernel
    // NOTE: no need to synchronize after each kernel
    // http://stackoverflow.com/a/11889641/2261947

    // NOTE: not sure why the 1st memset won't work. That would replace the
    //       sieveInitCUDA kernel...
    // checkCudaErrors(cudaMemset(primes, 0, sizeof(char)));
    // checkCudaErrors(cudaMemset(primes + 1, 1, sizeof(char)));
    sieveInitCUDA<<<1, 1>>>(d_Primes); // launch a single thread to initialize
    sieveEvenNumbersCUDA<<<dimGridEvens, dimBlock>>>(d_Primes, max);
    sieveOfEratosthenesCUDA<<<dimGrid, dimBlock>>>(d_Primes, max, maxRoot);

    // check for kernel errors
    checkCudaErrors(cudaPeekAtLastError());
    checkCudaErrors(cudaDeviceSynchronize());

    // copy the results back
    checkCudaErrors(cudaMemcpy(primes, d_Primes, sizePrimes, cudaMemcpyDeviceToHost));

    // no memory leaks
    checkCudaErrors(cudaFree(d_Primes));
}
