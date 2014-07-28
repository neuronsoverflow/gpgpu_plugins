#ifndef GRAPH_H
#define GRAPH_H

/******************************************************************************
* Graph G(V,E)
*
* Data structure based on Pawan Harish and P.J. Narayanan's paper:
* "Accelerating Large Graph Algorithms on the GPU Using CUDA" (HiPC 2007)
*****************************************************************************/
typedef struct
{
   // (V) This contains the index for it's adjacency list in the edgeArray
   int* vertexArray;

   // Vertex count
   int vertexCount;

   // (E) This contains a compact adjacency list of all edges in the graph
   int* edgeArray;

   // Edge count
   int edgeCount;

   // (W) Weight array contains a compact list of all weights for all edges
   int* weightArray;

} GraphData;

void bfsOnDevice (GraphData* graph, int source, int* costs);
void ssspOnDevice(GraphData* graph, int source, int* costs);
void apspOnDevice(GraphData* graph, int** costs);

#endif
