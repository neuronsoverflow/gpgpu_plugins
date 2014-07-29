Plugins
=========

Each one of the following CUDA programs can be generated in two ways:
- as a dynamic shared object (*.so files): `make plugins` (or simply `make`)
- as a standalone program: `make executables`

The difference between them is that the standalone programs take their parameters through ARGV, while the *.so files must be loaded, set and run through the Plugin Engine (`../gpgpu`)


matrix.so
----
Default parameters:
```
matrix:
	PARAM NAME => PARAM_VALUE
	inputFileMatrixA => inputFileMatrixA.txt
	inputFileMatrixB => inputFileMatrixB.txt
	OutputFileMatrixC => OutputFileMatrixC.txt
	displayResult => 0
```


prime.so
----
Default parameters:
```
prime:
	PARAM NAME => PARAM_VALUE
	outputFile => prime_numbers.txt
	limit => 101
```

graph.so
-----------
Default parameters:
```
graph:
	PARAM NAME => PARAM_VALUE
	searchMode => bfs
	inputFile => inputGraph.txt
	outputFile => resultGraph.txt
	sourceNode => 0
	displayResult => 1
```

