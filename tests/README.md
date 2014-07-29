Tests
=========

Description
-----
the `run_tests.rb` script is set to read the input files from the data folder and automatically test each one of the 3 plugins.


Execution
----------
run all the tests with:

```sh
ruby run_tests.rb
```

Expected Output
---
If there were no errors, we should see something like this:
```
Running all tests...
Checking test results...
PASSED MATRIX - matrix1000_1000C.txt
PASSED MATRIX - matrix100_100C.txt
PASSED MATRIX - matrix300_300C.txt
PASSED MATRIX - matrix3_3C.txt
PASSED MATRIX - matrixSimpleC.txt
PASSED PRIMES - primes_101.txt
PASSED PRIMES - primes_200.txt
PASSED PRIMES - primes_256.txt
PASSED PRIMES - primes_500.txt
PASSED PRIMES - primes_512.txt
PASSED PRIMES - primes_1000.txt
PASSED PRIMES - primes_1024.txt
PASSED PRIMES - primes_10000.txt
PASSED PRIMES - primes_50000.txt
PASSED PRIMES - primes_104729.txt
PASSED GRAPHS - simple_bfs
PASSED GRAPHS - simple_sssp
PASSED GRAPHS - simple_apsp
```
