/******************************************************************************
 *  The Graph Search algorithms follows the ones found in the paper:
 *  "Accelerating Large Graph Algorithms on the GPU using CUDA" (HiPC 2007)
 *  by Pawan Harish and P.J. Narayanan
 *
 *  This file implements:
 *    - BFS
 *    - SSSP
 *    - APSP
 ******************************************************************************/
#include "graph.h"
// #include <float.h> // FLT_MAX
#include <helper_cuda.h> // checkCudaErrors- NVIDIA_CUDA-6.0_Samples/common/inc
#include <limits.h>
#include <stdio.h>

////////////////////////// HELPER FUNCTIONS
// getNumEdges() - returns the number of edges for the vertex found in the
//                 given index
__host__ __device__ int getNumEdges(int index, int* vertices, int* edges, int numVertices, int numEdges)
{
    int count = vertices[(index + 1) % numVertices] - vertices[index];
    if (count < 0)
        count = numEdges - vertices[index];
    return count;
}

// getEdgeEndIndex() - given the starting index of the vertex, return the
//                     ending edge index for the given vertex
__host__ __device__ int getEdgeEndIndex(int index, int* vertices, int numVertices, int numEdges)
{
    return (index + 1 < (numVertices)) ? vertices[index + 1] : numEdges;
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

////////////////////////// KERNEL FUNCTIONS

/******************************************************************************
 *  CUDA_BFS_KERNEL (Va, Ea, Fa, Xa, Ca)
 *
 *  This algorithm follows the one found in the paper
 *    "Accelerating Large Graph Algorithms on the GPU using CUDA"
 *    by Pawan Harish and P.J. Narayanan
 *
 *  Algortithm:
 *  1: tid ← getThreadID
 *  2: if Fa [tid] then
 *  3:   Fa [tid] ← false, Xa [tid] ← true
 *  4:   for all neighbors nid of tid do
 *  5:     if NOT Xa [nid] then
 *  6:       Ca [nid] ← Ca [tid]+1
 *  7:       Fa [nid] ← true
 *  8:     end if
 *  9:   end for
 *  10:end if
 ******************************************************************************/
__global__ void bfsCUDA(int* vertices, int* edges, bool* frontier, bool* visited, int* cost, bool* keepRunning,
                        int numVertices, int numEdges)
{
    int tid = blockIdx.x * blockDim.x + threadIdx.x; // 1
    if (tid < numVertices && frontier[tid])          // 2
    {
        // 3
        frontier[tid] = false;
        visited[tid] = true;
        int edgeStart = vertices[tid];
        int edgeEnd = getEdgeEndIndex(tid, vertices, numVertices, numEdges);

        // 4
        // "If true, it fetches its cost from the cost array C and updates all
        // the costs of its neighbors if more than its own cost plus one using
        // the edge list E."

        // "The vertex removes its own entry from the frontier array F and adds
        // to the visited array Xa . It also adds its neighbors to the frontier
        // array if the neighbor is not already visited."
        for (int edge = edgeStart; edge < edgeEnd; edge++)
        {
            int nid = edges[edge];
            if (!visited[nid])
            {
                cost[nid] = cost[tid] + 1;
                frontier[nid] = true;
                *keepRunning = true;
            }
        }
    }
}

/******************************************************************************
 * CUDA_SSSP_KERNEL1 (Va, Ea, Wa, Ma, Ca, Ua)
 *
 * 1: tid ← getThreadID
 * 2: if Ma [tid] then
 * 3:   Ma [tid] ← false
 * 4:   for all neighbors nid of tid do
 * 5:     if Ua [nid]> Ca [tid]+Wa [edge] then
 * 6:       Ua [nid] ← Ca [tid]+Wa [edge]
 * 7:     end if
 * 8:   end for
 * 9: end if
 ******************************************************************************/
__global__ void ssspCUDA1(int* vertices, int* edges, int* weights, int* mask, int* cost, int* updatingCost,
                          // bool* mask, float* cost, float* updatingCost,
                          int numVertices, int numEdges)
{
    unsigned int tid = blockIdx.x * blockDim.x + threadIdx.x;
    if (tid < numVertices && mask[tid])
    {
        mask[tid] = 0;

        int edgeStart = vertices[tid];
        int edgeEnd = getEdgeEndIndex(tid, vertices, numVertices, numEdges);

        // edge = index of the neighbor in the edges array and in the weights arr
        for (int edge = edgeStart; edge < edgeEnd; edge++)
        {
            int nid = edges[edge]; // neighbor vertex index
            if (updatingCost[nid] > (cost[tid] + weights[edge]))
            {
                // updatingCost[nid] = (cost[tid] + weights[edge]);
                atomicMin(&updatingCost[nid], cost[tid] + weights[edge]);
            }
        }
    }
}

/******************************************************************************
 * CUDA_SSSP_KERNEL2 (Va, Ea, Wa, Ma, Ca, Ua)
 *
 * 1: tid ← getThreadID
 * 2: if Ca [tid] > Ua [tid] then
 * 3:   Ca [tid] ← Ua [tid]
 * 4:   Ma [tid] ← true
 * 5: end if
 * 6: Ua [tid] ← Ca [tid]
 ******************************************************************************/
__global__ void ssspCUDA2(int* vertices, int* edges, int* weights, int* mask, int* cost, int* updatingCost,
                          int numVertices, int numEdges, bool* keepRunning)
{
    int tid = blockIdx.x * blockDim.x + threadIdx.x;
    if (tid < numVertices)
    {
        if (cost[tid] > updatingCost[tid])
        {
            // cost[tid] = updatingCost[tid];
            atomicMin(&cost[tid], updatingCost[tid]);
            // atomicAdd(&mask[tid], 1); // mask[tid] = true;
            atomicMax(&mask[tid], 1); // mask[tid] = true;
            *keepRunning = true;
        }
        updatingCost[tid] = cost[tid];
    }
}

/******************************************************************************
 *  CUDA_BFS (Graph G(V, E), Source Vertex S)
 *
 *  1. Create vertex array Va from all vertices and edge Array Ea from all
 *     edges in G(V, E),
 *  2. Create frontier array Fa , visited array X a and cost array Ca of size V.
 *  3. Initialize Fa , Xa to false and Ca to ∞
 *  4. Fa [S] ← true, Ca [S] ← 0
 *  5. while Fa not Empty do
 *  6.   for each vertex V in parallel do
 *  7.     Invoke CUDA BFS KERNEL(Va, Ea, Fa, Xa, Ca) on the grid.
 *  8.   end for
 *  9. end while
 *
 *  "The BFS problem is, given an undirected, unweighted graph G(V, E) and a
 *     source vertex S, find the minimum number of edges needed to reach every
 *     vertex V in G from source vertex S."
 *
 * PARAMETERS:
 * graph - The Graph Data - Graph G(V,E)
 * sourceIndex - the index for the vertex/node to perform the BFS on.
 * h_Cost - an array of the size of the number of vertices.
 *          The final BFS result will be saved here, with the BFS costs to each
 *          node.
 ******************************************************************************/
__host__ void bfsOnDevice(GraphData* graph, int sourceIndex, int* h_Cost)
{
    if (!graph || sourceIndex < 0 || sourceIndex >= graph->vertexCount || !h_Cost)
    {
        printf("Failed to run BFS on the Device\n");
        return;
    }

    // HOST graph data
    bool* h_Frontier; // Fa (frontier array)
    bool* h_Visited;  // Xa (visited array)
                      // h_Cost = Ca (cost array)
    int numVertices = graph->vertexCount;
    int sizeVBool = sizeof(bool) * numVertices;
    int sizeVInt = sizeof(int) * numVertices;

    // 2, 3
    h_Frontier = (bool*)calloc(numVertices, sizeof(bool));
    h_Visited = (bool*)calloc(numVertices, sizeof(bool));
    h_Frontier[sourceIndex] = true;
    memset(h_Cost, -1, sizeVInt);

    // 4
    h_Visited[sourceIndex] = 1;
    h_Cost[sourceIndex] = 0;

    // DEVICE graph data
    int blockSize = checkDevice();
    int* d_Vertices;  // vertices / nodes
    int* d_Edges;     // edges
    bool* d_Frontier; // frontier / mask
    bool* d_Visited;  // visited
    int* d_Cost;      // cost
    bool* d_keepRunning;

    // 1 - copy the vertices and edges: HOST -> DEVICE (GPU)
    checkCudaErrors(cudaMalloc(&d_Vertices, sizeVInt));
    checkCudaErrors(cudaMalloc(&d_Edges, sizeof(int) * graph->edgeCount));
    checkCudaErrors(cudaMemcpy(d_Vertices, graph->vertexArray, sizeVInt, cudaMemcpyHostToDevice));
    checkCudaErrors(cudaMemcpy(d_Edges, graph->edgeArray, sizeof(int) * graph->edgeCount, cudaMemcpyHostToDevice));

    // 2 - copy the frontier, visited and cost arrays: HOST -> DEVICE
    checkCudaErrors(cudaMalloc(&d_Frontier, sizeVBool));
    checkCudaErrors(cudaMalloc(&d_Visited, sizeVBool));
    checkCudaErrors(cudaMalloc(&d_Cost, sizeof(int) * numVertices));
    checkCudaErrors(cudaMalloc(&d_keepRunning, sizeof(bool)));
    checkCudaErrors(cudaMemcpy(d_Frontier, h_Frontier, sizeVBool, cudaMemcpyHostToDevice));
    checkCudaErrors(cudaMemcpy(d_Visited, h_Visited, sizeVBool, cudaMemcpyHostToDevice));
    checkCudaErrors(cudaMemcpy(d_Cost, h_Cost, sizeVInt, cudaMemcpyHostToDevice));

    // set up the execution parameters
    // 1 thread for each vertex
    dim3 dimBlock(blockSize);
    dim3 dimGrid(ceil(numVertices / (double)dimBlock.x));

#ifdef DEBUG
    printf("dimBlock(%d, %d, %d)\n", dimBlock.x, dimBlock.y, dimBlock.z);
    printf("dimGrid(%d, %d, %d)\n", dimGrid.x, dimGrid.y, dimGrid.z);
#endif

    bool keepRunning = true;

    // 4.loop while the Frontier array is not empty
    while (keepRunning)
    {
        keepRunning = false;
        // update the "keepRunning" boolean variable: HOST --> DEVICE
        checkCudaErrors(cudaMemcpy(d_keepRunning, &keepRunning, sizeof(bool), cudaMemcpyHostToDevice));
        // call the KERNEL
        bfsCUDA<<<dimGrid, dimBlock>>>(d_Vertices, d_Edges, d_Frontier, d_Visited, d_Cost, d_keepRunning, numVertices,
                                       graph->edgeCount);
        checkCudaErrors(cudaPeekAtLastError());   // check for Kernel errors
        checkCudaErrors(cudaDeviceSynchronize()); // block the CPU until GPU done

        // update the "keepRunning" boolean variable: HOST <-- DEVICE
        checkCudaErrors(cudaMemcpy(&keepRunning, d_keepRunning, sizeof(bool), cudaMemcpyDeviceToHost));
    }

    // Final BFS costs result will be in d_cost -> h_cost
    checkCudaErrors(cudaMemcpy(h_Cost, d_Cost, sizeVInt, cudaMemcpyDeviceToHost));

    // release the krakens!
    checkCudaErrors(cudaFree(d_Vertices));
    checkCudaErrors(cudaFree(d_Edges));
    checkCudaErrors(cudaFree(d_Frontier));
    checkCudaErrors(cudaFree(d_Visited));
    checkCudaErrors(cudaFree(d_Cost));
    checkCudaErrors(cudaFree(d_keepRunning));

    free(h_Frontier);
    free(h_Visited);
}

/******************************************************************************
 *  CUDA_SSSP (Graph G(V, E,W ), Source Vertex S)
 *
 * 1: Create vertex array Va , edge array Ea and weight array Wa from G(V, E, W)
 * 2: Create mask array Ma , cost array Ca and Updating cost array Ua of size V
 * 3: Initialize mask Ma to false, cost array Ca and Updating cost array Ua to ∞
 * 4: Ma [S] ← true, Ca [S] ← 0, Ua [S] ← 0
 * 5: while Ma not Empty do
 * 6:   for each vertex V in parallel do
 * 7:     Invoke CUDA SSSP KERNEL1(Va, Ea, Wa, Ma, Ca, Ua) on the grid
 * 8:     Invoke CUDA SSSP KERNEL2(Va, Ea, Wa, Ma, Ca, Ua) on the grid
 * 9:   end for
 *10: end while
 *
 *  "Single source shortest path (SSSP) problem is, given weighted graph
 *     G(V,E,W) with positive weights and a source vertex S, find the smallest
 *     combined weight of edges that is required to reach every vertex V from
 *     source vertex S"
 ******************************************************************************/
__host__ void ssspOnDevice(GraphData* graph, int sourceIndex, int* h_Cost)
{
    if (!graph || !h_Cost || sourceIndex < 0 || sourceIndex >= graph->vertexCount)
    {
        printf("Failed to run SSSP on the Device\n");
        return;
    }

    // HOST graph data
    int* h_Mask;         // Ma (mask array)
    int* h_UpdatingCost; // Ua (updating cost array)

    int numVertices = graph->vertexCount;
    // NOTE: the cost array and updating cost array should have been float*
    //       Also, the mask array should have been bool* or char*
    //       However, atomicMin() and atomicMax() doesn't play well w/ these
    //         data-types, so we're using integers...
    // int sizeVFloat  = sizeof(float) * numVertices;
    // int sizeVBool   = sizeof(bool)  * numVertices;
    int sizeVInt = sizeof(int) * numVertices;

    h_Mask = (int*)malloc(sizeVInt);
    h_UpdatingCost = (int*)malloc(sizeVInt);

    // 3.
    memset(h_Mask, 0, sizeVInt);
    for (int i = 0; i < numVertices; i++)
    {
        h_Cost[i] = INT_MAX; // use FLT_MAX if float*
        h_UpdatingCost[i] = INT_MAX;
    }

    // 4.
    h_Mask[sourceIndex] = 1;
    h_Cost[sourceIndex] = 0;
    h_UpdatingCost[sourceIndex] = 0;

    // DEVICE graph data
    int blockSize = checkDevice();
    int* d_Vertices;
    int* d_Edges;
    int* d_Weights;
    int* d_Mask;
    int* d_Cost;
    int* d_UpdatingCost;
    bool* d_keepRunning;

    // 1 - copy the vertices, edges and weights: HOST --> DEVICE (GPU)
    checkCudaErrors(cudaMalloc(&d_Vertices, sizeVInt));
    checkCudaErrors(cudaMalloc(&d_Edges, sizeof(int) * graph->edgeCount));
    checkCudaErrors(cudaMalloc(&d_Weights, sizeof(int) * graph->edgeCount));
    checkCudaErrors(cudaMemcpy(d_Vertices, graph->vertexArray, sizeVInt, cudaMemcpyHostToDevice));
    checkCudaErrors(cudaMemcpy(d_Edges, graph->edgeArray, sizeof(int) * graph->edgeCount, cudaMemcpyHostToDevice));
    checkCudaErrors(cudaMemcpy(d_Weights, graph->weightArray, sizeof(int) * graph->edgeCount, cudaMemcpyHostToDevice));

    // 2 - copy the mask, cost, updating cost arrays: HOST --> DEVICE
    checkCudaErrors(cudaMalloc(&d_Mask, sizeVInt));
    checkCudaErrors(cudaMalloc(&d_Cost, sizeVInt));
    checkCudaErrors(cudaMalloc(&d_UpdatingCost, sizeVInt));
    checkCudaErrors(cudaMalloc(&d_keepRunning, sizeof(bool)));
    checkCudaErrors(cudaMemcpy(d_Mask, h_Mask, sizeVInt, cudaMemcpyHostToDevice));
    checkCudaErrors(cudaMemcpy(d_Cost, h_Cost, sizeVInt, cudaMemcpyHostToDevice));
    checkCudaErrors(cudaMemcpy(d_UpdatingCost, h_UpdatingCost, sizeVInt, cudaMemcpyHostToDevice));

    // set up the execution parameters
    // 1 thread for each vertex
    dim3 dimBlock(blockSize);
    dim3 dimGrid(ceil(numVertices / (double)dimBlock.x));

//////// debug
#ifdef DEBUG
    printf("numVertices: %d\n", numVertices);
    printf("dimBlock(%d, %d, %d)\n", dimBlock.x, dimBlock.y, dimBlock.z);
    printf("dimGrid(%d, %d, %d)\n", dimGrid.x, dimGrid.y, dimGrid.z);
#endif

    bool keepRunning = true;

    // 5. loop while the Mask array is not empty
    while (keepRunning)
    {
        keepRunning = false;
        checkCudaErrors(cudaMemcpy(d_keepRunning, &keepRunning, sizeof(bool), cudaMemcpyHostToDevice));
        // call the KERNELs
        ssspCUDA1<<<dimGrid, dimBlock>>>(d_Vertices, d_Edges, d_Weights, d_Mask, d_Cost, d_UpdatingCost, numVertices,
                                         graph->edgeCount);
        ssspCUDA2<<<dimGrid, dimBlock>>>(d_Vertices, d_Edges, d_Weights, d_Mask, d_Cost, d_UpdatingCost, numVertices,
                                         graph->edgeCount, d_keepRunning);
// //////////////// DEBUG
#ifdef DEBUG

        // copy back the current cost, ucost, mask
        checkCudaErrors(cudaMemcpy(h_Cost, d_Cost, sizeVInt, cudaMemcpyDeviceToHost));
        checkCudaErrors(cudaMemcpy(h_UpdatingCost, d_UpdatingCost, sizeVInt, cudaMemcpyDeviceToHost));
        checkCudaErrors(cudaMemcpy(h_Mask, d_Mask, sizeVInt, cudaMemcpyDeviceToHost));

        // display the current cost, ucost, umask
        printf("after a iteration: \n");
        printf("cost:  ");
        for (int i = 0; i < numVertices; i++)
        {
            printf("%5d ", h_Cost[i]);
        }
        printf("\n");
        printf("ucost: ");
        for (int i = 0; i < numVertices; i++)
        {
            printf("%5d ", h_UpdatingCost[i]);
        }
        printf("\n");
        printf("mask:  ");
        for (int i = 0; i < numVertices; i++)
        {
            printf("%5d ", h_Mask[i]);
        }
        printf("\n");
#endif
        //////////////// END DEBUG

        // update the "keepRunning" boolean variable: HOST <- DEVICE
        checkCudaErrors(cudaMemcpy(&keepRunning, d_keepRunning, sizeof(bool), cudaMemcpyDeviceToHost));
    }

    // copy the computed costs back to the host
    checkCudaErrors(cudaMemcpy(h_Cost, d_Cost, sizeVInt, cudaMemcpyDeviceToHost));
    // free the memory
    checkCudaErrors(cudaFree(d_Vertices));
    checkCudaErrors(cudaFree(d_Edges));
    checkCudaErrors(cudaFree(d_Weights));
    checkCudaErrors(cudaFree(d_Mask));
    checkCudaErrors(cudaFree(d_Cost));
    checkCudaErrors(cudaFree(d_UpdatingCost));
    checkCudaErrors(cudaFree(d_keepRunning));

    free(h_Mask);
    free(h_UpdatingCost);
}

/******************************************************************************
 *  ASPS_USING_SSSP(G(V,E,W))
 *
 * 1:  Create vertex array Va , edge array Ea , weight array Wa from G(V,E,W),
 * 2:  Create mask array Ma , cost array Ca and updating cost array Ua of size V
 * 3:  for S from 1 to V do
 * 4:    Ma [S] ← true
 * 6:    Ca [S] ← 0
 * 7:    while Ma not Empty do
 * 8:      for each vertex V in parallel do
 * 9:        Invoke CUDA SSSP KERNEL1(Va, Ea, Wa, Ma, Ca, Ua) on the grid
 * 10:       Invoke CUDA SSSP KERNEL2(Va, Ea, Wa, Ma, Ca, Ua) on the grid
 * 11:     end for
 * 12:   end while
 * 13: end for
 ******************************************************************************/
__host__ void apspOnDevice(GraphData* graph, int** h_Costs)
{
    if (!graph || !h_Costs)
    {
        printf("Failed to run APSP on the Device\n");
        return;
    }

    // HOST graph data
    int* h_Infinity;

    int numVertices = graph->vertexCount;
    int sizeVInt = sizeof(int) * numVertices;

    h_Infinity = (int*)malloc(sizeVInt);
    for (int i = 0; i < numVertices; i++)
    {
        h_Infinity[i] = INT_MAX;
    }

    // DEVICE graph data
    int blockSize = checkDevice();
    int* d_Vertices;
    int* d_Edges;
    int* d_Weights;
    int* d_Mask;
    int* d_Cost;
    int* d_UpdatingCost;
    bool* d_keepRunning;

    // 1 - copy the vertices, edges and weights: HOST --> DEVICE (GPU)
    checkCudaErrors(cudaMalloc(&d_Vertices, sizeVInt));
    checkCudaErrors(cudaMalloc(&d_Edges, sizeof(int) * graph->edgeCount));
    checkCudaErrors(cudaMalloc(&d_Weights, sizeof(int) * graph->edgeCount));
    checkCudaErrors(cudaMemcpy(d_Vertices, graph->vertexArray, sizeVInt, cudaMemcpyHostToDevice));
    checkCudaErrors(cudaMemcpy(d_Edges, graph->edgeArray, sizeof(int) * graph->edgeCount, cudaMemcpyHostToDevice));
    checkCudaErrors(cudaMemcpy(d_Weights, graph->weightArray, sizeof(int) * graph->edgeCount, cudaMemcpyHostToDevice));

    // 2 - allocate the mask, cost, updating cost arrays on the DEVICE
    checkCudaErrors(cudaMalloc(&d_Mask, sizeVInt));
    checkCudaErrors(cudaMalloc(&d_Cost, sizeVInt));
    checkCudaErrors(cudaMalloc(&d_UpdatingCost, sizeVInt));
    checkCudaErrors(cudaMalloc(&d_keepRunning, sizeof(bool)));

    // set up the execution parameters
    // 1 thread for each vertex
    dim3 dimBlock(blockSize);
    dim3 dimGrid(ceil(numVertices / (double)dimBlock.x));

    for (int vertex = 0; vertex < numVertices; vertex++)
    {
        // reset the mask, cost, updating cost arrays on DEVICE
        checkCudaErrors(cudaMemset(d_Mask, 0, sizeVInt));
        checkCudaErrors(cudaMemcpy(d_Cost, h_Infinity, sizeVInt, cudaMemcpyHostToDevice));
        checkCudaErrors(cudaMemcpy(d_UpdatingCost, h_Infinity, sizeVInt, cudaMemcpyHostToDevice));

        // 4, 5
        // initMaskAndCost<<< 1, 1 >>>(d_Mask, d_Cost, vertex);
        checkCudaErrors(cudaMemset(d_Mask + vertex, true, sizeof(int)));
        checkCudaErrors(cudaMemset(d_Cost + vertex, 0, sizeof(int)));

        bool keepRunning = true;
        while (keepRunning)
        {
            keepRunning = false;
            checkCudaErrors(cudaMemcpy(d_keepRunning, &keepRunning, sizeof(bool), cudaMemcpyHostToDevice));
            // call the KERNELs
            ssspCUDA1<<<dimGrid, dimBlock>>>(d_Vertices, d_Edges, d_Weights, d_Mask, d_Cost, d_UpdatingCost,
                                             numVertices, graph->edgeCount);
            // checkCudaErrors(cudaPeekAtLastError());   // check for Kernel errors
            // checkCudaErrors(cudaDeviceSynchronize()); // block the CPU until GPU done
            ssspCUDA2<<<dimGrid, dimBlock>>>(d_Vertices, d_Edges, d_Weights, d_Mask, d_Cost, d_UpdatingCost,
                                             numVertices, graph->edgeCount, d_keepRunning);
            checkCudaErrors(cudaPeekAtLastError());   // check for Kernel errors
            checkCudaErrors(cudaDeviceSynchronize()); // block the CPU until GPU done

            // update the "keepRunning" boolean variable: HOST <- DEVICE
            checkCudaErrors(cudaMemcpy(&keepRunning, d_keepRunning, sizeof(bool), cudaMemcpyDeviceToHost));
        }

        checkCudaErrors(cudaMemcpy(h_Costs[vertex], d_Cost, sizeVInt, cudaMemcpyDeviceToHost));
    }

    // free the memory
    checkCudaErrors(cudaFree(d_Vertices));
    checkCudaErrors(cudaFree(d_Edges));
    checkCudaErrors(cudaFree(d_Weights));
    checkCudaErrors(cudaFree(d_Mask));
    checkCudaErrors(cudaFree(d_Cost));
    checkCudaErrors(cudaFree(d_UpdatingCost));
    checkCudaErrors(cudaFree(d_keepRunning));

    free(h_Infinity);
}
