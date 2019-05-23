#include "constants.h"
#include "graph.h"
#include "pluginHeader.h"

#include <boost/algorithm/string.hpp> // starts_with, to_lower
#include <cstdlib>                    // atoi, strtoull
#include <cstring>                    // strcpy
#include <fstream>                    // ifstream, ofstream
#include <iomanip>                    // setw
#include <iostream>                   // cout
#include <sstream>                    // stringstream
#include <vector>                     // vector

// Graph functions
int createGraphFromFile(GraphData& graph, char* file);
int writeGraphSearchResults(const GraphData& graph, int* costs, int source);
int writeGraphSearchResults(const GraphData& graph, int** costs);
int runBFS();
int runSSSP();
int runAPSP();

enum inputs
{
    MODE,
    INPUT,
    OUTPUT,
    SOURCE,
    DISPLAY
};

/* globals */
char params[][256] = {"bfs", "inputGraph.txt", "resultGraph.txt", "0", "1"};
const int NUM_ARGS = sizeof params / sizeof params[0];
clock_t total_t = 0;
const char* PARAM_INFO = "searchMode,inputFile,outputFile,sourceNode,displayResult";

/******************************************************************************
 * main()
 * - this function is not used by the plugin, but this can be built as
 *   a standalone executable
 ******************************************************************************/
int main(int argc, char** argv)
{
    if (argc == 1)
    {
        std::cout << "Usage: " << argv[0] << " searchMode inputFile outputFile sourceNode\n";
        return 0;
    }

    if (argc > 1)
    {
        if (strcmp(argv[1], "--help") == 0)
        {
            displayPluginInfo();
            return 0;
        }
        strcpy(params[MODE], argv[1]);
    }

    if (argc > 2)
        strcpy(params[INPUT], argv[2]);

    if (argc > 3)
        strcpy(params[OUTPUT], argv[3]);

    if (argc > 4)
        strcpy(params[SOURCE], argv[4]);

    return run();
}

////////////////////////////// GRAPH FUNCTIONS ////////////////////////////////

/******************************************************************************
 *
 ******************************************************************************/
int runBFS()
{
    clock_t start_t;
    clock_t end_t;

    GraphData graph;
    int source = 0;

    if (createGraphFromFile(graph, params[INPUT]) == ERROR)
    {
        std::cout << "Failed to open: " << params[INPUT] << "\n";
        return ERROR;
    }

    // check that the source is a valid index
    source = atoi(params[SOURCE]); // get the source index
    if (source < 0 || source >= graph.vertexCount)
    {
        std::cout << "Source node out of bounds. Must be a vertex index 0-" << graph.vertexCount - 1 << ".\n";
        return ERROR;
    }

    int* costs;

    // allocate memory for the costs and of BFS to each node
    costs = new (std::nothrow) int[graph.vertexCount]();
    if (!costs)
    {
        std::cout << "Failed to allocate memory for the vertices\n";
        return ERROR;
    }

    // run the algorithm on CUDA
    // NOTE: a more accurate measure of time would happen inside the *.cu file,
    //       using CUDA's timing events:
    //       cudaEvent_t start; cudaEventCreate(&start); cudaEventRecord(start);
    start_t = clock();
    bfsOnDevice(&graph, source, costs);
    end_t = clock();
    total_t = end_t - start_t;

    // write the results to the file
    writeGraphSearchResults(graph, costs, source);

    if (atoi(params[DISPLAY]))
    {
        std::cout << "BFS(" << source << ") Costs:\n";
        for (int i = 0; i < graph.vertexCount; i++)
            std::cout << costs[i] << " ";
        std::cout << "\n";
    }

    // cleanup
    delete[] costs;

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
int runSSSP()
{
    clock_t start_t;
    clock_t end_t;

    GraphData graph;

    // open and read the graph file
    if (createGraphFromFile(graph, params[INPUT]) == ERROR)
    {
        std::cout << "Failed to open: " << params[INPUT] << "\n";
        return ERROR;
    }

    int source = atoi(params[SOURCE]); // get the source index

    // check that the source is a valid index
    if (source < 0 || source >= graph.vertexCount)
    {
        std::cout << "Source node out of bounds. Must be a vertex index 0-" << graph.vertexCount - 1 << ".\n";
        return ERROR;
    }

    int* costs;

    // allocate memory for the costs
    costs = new (std::nothrow) int[graph.vertexCount];
    if (!costs)
    {
        std::cout << "Failed to allocate memory for the vertices\n";
        return ERROR;
    }

    // run the algorithm on CUDA
    start_t = clock();
    ssspOnDevice(&graph, source, costs);
    end_t = clock();
    total_t = end_t - start_t;

    // write the results to the file
    writeGraphSearchResults(graph, costs, source);

    if (atoi(params[DISPLAY]))
    {
        std::cout << "SSSP(" << source << ") Costs:\n";
        for (int i = 0; i < graph.vertexCount; i++)
            std::cout << costs[i] << " ";
        std::cout << "\n";
    }

    // cleanup
    delete[] costs;

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
int runAPSP()
{
    clock_t start_t;
    clock_t end_t;

    GraphData graph;

    // open and read the graph file
    if (createGraphFromFile(graph, params[INPUT]) == ERROR)
    {
        std::cout << "Failed to open: " << params[INPUT] << "\n";
        return ERROR;
    }

    int** costs;

    // allocate memory for the costs - 2d array, 1 array for each vertex
    costs = new (std::nothrow) int*[graph.vertexCount]();
    if (!costs)
    {
        std::cout << "Failed to allocate memory for the vertices\n";
        return ERROR;
    }
    for (int i = 0; i < graph.vertexCount; i++)
    {
        costs[i] = new (std::nothrow) int[graph.vertexCount]();
        if (!(costs[i]))
            std::cout << "Failed to allocate memory for the vertices!\n";
    }

    // run the algorithm on CUDA
    start_t = clock();
    apspOnDevice(&graph, costs);
    end_t = clock();
    total_t = end_t - start_t;

    // write the results to the file
    writeGraphSearchResults(graph, costs);

    // TODO: add a top row w/ the columns, use setw for nice formatting
    if (atoi(params[DISPLAY]))
    {
        std::cout << "APSP:\n";
        for (int i = 0; i < graph.vertexCount; i++)
        {
            std::cout << i << ": ";
            for (int j = 0; j < graph.vertexCount; j++)
            {
                std::cout << costs[i][j];
                if ((j + 1) != graph.vertexCount)
                    std::cout << ", ";
            }
            std::cout << "\n";
        }
    }

    // cleanup
    for (int i = 0; i < graph.vertexCount; i++)
        delete[] costs[i];
    delete[] costs;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
int createGraphFromFile(GraphData& graph, char* file)
{
    std::ifstream fIn(file, std::ifstream::in);
    if (fIn.fail())
    {
        // std::cout << "Error on opening the file\n";
        return ERROR;
    }

    std::string line;
    std::vector<int> vertices; // holds the index for the edges
    std::vector<int> edges;    // holds the list of connected vertices
    std::vector<int> weights;

    int src;
    int dest;
    int weight;
    int index = 0;
    while (fIn >> src && !fIn.eof())
    {
        vertices.push_back(index);

        getline(fIn, line);
        std::stringstream restOfLine(line);
        while (restOfLine >> dest >> weight && !fIn.eof())
        {
            edges.push_back(dest);
            weights.push_back(weight);
            index++;
        }
    }
    fIn.close();
    if (!vertices.size())
    {
        // std::cout << "error on the size of the vertices!\n";
        return ERROR;
    }

// display the vectors
#ifdef DEBUG
    std::cout << "Vertices (" << vertices.size() << "):\n";
    for (int i = 0; i < vertices.size(); i++)
        std::cout << vertices[i] << " ";
    std::cout << "\n";

    std::cout << "Edges (" << edges.size() << "):\n";
    for (int i = 0; i < edges.size(); i++)
        std::cout << edges[i] << " ";
    std::cout << "\n";

    std::cout << "Weights (" << weights.size() << "):\n";
    for (int i = 0; i < weights.size(); i++)
        std::cout << weights[i] << " ";
    std::cout << "\n";
#endif

    // copy the data from the vectors to the struct
    graph.vertexCount = vertices.size();
    graph.edgeCount = edges.size();
    graph.vertexArray = new (std::nothrow) int[vertices.size()];
    graph.edgeArray = new (std::nothrow) int[edges.size()];
    graph.weightArray = new (std::nothrow) int[edges.size()];
    if (!graph.vertexArray || !graph.edgeArray || !graph.weightArray)
    {
        // std::cout << "error allocating...\n";
        return ERROR; // error allocating...
    }

    std::copy(vertices.begin(), vertices.end(), graph.vertexArray);
    std::copy(edges.begin(), edges.end(), graph.edgeArray);
    std::copy(weights.begin(), weights.end(), graph.weightArray);

    return OK;
}

/******************************************************************************
 * writeGraphSearchResults()
 * - this function is used both for the BFS and for the SSSP
 ******************************************************************************/
int writeGraphSearchResults(const GraphData& graph, int* costs, int source)
{
    std::ofstream fout(params[OUTPUT]);
    if (!fout)
    {
        return ERROR;
    }

    fout << "Source Node: " << source << "\n";
    fout << '\n';
    fout << "Destination Node => Cost\n";
    for (int i = 0; i < graph.vertexCount; i++)
    {
        fout << std::setw(5) << std::left << i << std::right << " => " << costs[i] << '\n';
    }
    fout.close();
    return OK;
}

/******************************************************************************
 * writeGraphSearchResults() - writes the results of APSP to a file
 ******************************************************************************/
int writeGraphSearchResults(const GraphData& graph, int** costs)
{
    std::ofstream fout(params[OUTPUT]);
    if (!fout)
    {
        return ERROR;
    }
    fout << "                Cost to Destination Nodes:\n";
    fout << "Source Node:    ";

    // display a tab-separated list of the node indices
    for (int i = 0; i < graph.vertexCount; i++)
        fout << i << '\t';
    fout << '\n';

    // display the actual values for each row
    for (int row = 0; row < graph.vertexCount; row++)
    {
        fout << std::setw(6) << std::left << row << std::setw(7) << ": " << std::right;
        for (int col = 0; col < graph.vertexCount; col++)
            fout << "\t" << costs[row][col];
        fout << '\n';
    }
    return OK;
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

    std::string mode = params[MODE];
    boost::algorithm::to_lower(mode);

    std::cout << "Running Graph Search on mode: \"" << mode << "\"\n";

    if (mode == "bfs")
        runBFS();
    else if (mode == "sssp")
        runSSSP();
    else if (mode == "apsp")
        runAPSP();
    else
    {
        std::cout << "Invalid mode set. Options are:\n"
                  << " - BFS\n"
                  << " - SSSP\n"
                  << " - APSP\n";
    }
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
    std::cout << "The Graph Search Plugin is designed to run three algorithms:\n"
              << "\t* BFS  - Breadth First Search\n"
              << "\t* SSSP - Single Source Shortest Path\n"
              << "\t* APSP - All-Pairs Shortest Path\n"
              << "The configuration params for running the plugin appropriately are as follows:\n"
              << "\t* searchMode - can be 'BFS', 'SSSP', or 'APSP'\n"
              << "\t* inputFile  - the file that contains the Graph data\n"
              << "\t* outputFile - the result of the search will be stored in this file\n"
              << "\t* sourceNode - The starting vertex or node for the search (used in\n"
              << "                           BFS and SSSP)\n"
              << "\t* displayResult - flag '0' or '1'. If true, then display the result on\n"
              << "                           the screen\n";
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
