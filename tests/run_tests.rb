#!/usr/bin/ruby

require 'open3'

PASSED = "\e[1;32mPASSED\e[0m".freeze  # green color
FAILED = "\e[1;31mFAILED\e[0m".freeze  # red   color

TEST_PRIME = true
TEST_GRAPH = true
TEST_MATRIX = true
BUILD_WITH_CMAKE = true

BASE_DIR = File.expand_path("#{__dir__}/..").freeze
TMP_DIR = "#{BASE_DIR}/tests/tmp".freeze
DATA_DIR = "#{BASE_DIR}/tests/data".freeze
BUILD_DIR = "#{BASE_DIR}/build".freeze

MAIN_PROGRAM_PATH  = "#{BUILD_DIR}/bin/gpgpu".freeze
PLUGIN_HELLO_PATH  = "#{BUILD_DIR}/lib/hello.so".freeze
PLUGIN_MATRIX_PATH = "#{BUILD_DIR}/lib/matrix.so".freeze
PLUGIN_GRAPH_PATH  = "#{BUILD_DIR}/lib/graph.so".freeze
PLUGIN_PRIME_PATH  = "#{BUILD_DIR}/lib/prime.so".freeze

CMAKE_BUILD_CMD = "mkdir -p #{BUILD_DIR} && cd #{BUILD_DIR} && " \
                  "cmake -DCMAKE_RUNTIME_OUTPUT_DIRECTORY='#{BUILD_DIR}/bin' " \
                  "-DCMAKE_LIBRARY_OUTPUT_DIRECTORY='#{BUILD_DIR}/lib' " \
                  '-DENABLE_CLANG_FORMAT=OFF -DENABLE_CLANG_TIDY=OFF .. && ' \
                  'cmake --build .'.freeze

$failures = 0

###############################################################################
# run_matrix()
###############################################################################
def run_matrix(pipe)
  # $matrix_files = %w(matrix3_3 matrix100_100 matrix300_300 matrix1000_1000) # matrix5000_5000)
  $matrix_files = `ls -1 #{DATA_DIR}/matrix/*C.txt`.split.map { |m| File.basename(m, 'txt')[0..-3] }
  prefix = "#{DATA_DIR}/matrix/"
  pipe.puts "set matrix displayResult 0"
  $matrix_files.each do |f|
    pipe.puts "set matrix inputFileMatrixA #{prefix}#{f}A.txt"
    pipe.puts "set matrix inputFileMatrixB #{prefix}#{f}B.txt"
    pipe.puts "set matrix OutputFileMatrixC #{TMP_DIR}/#{f}C.txt"
    pipe.puts "run matrix"
  end
end

###############################################################################
# check_matrix_results()
###############################################################################
def check_matrix_results()
  return unless $matrix_files
  prefix = "#{DATA_DIR}/matrix/"
  $matrix_files.each do |f|
    if `diff -w -B #{TMP_DIR}/#{f}C.txt #{prefix}#{f}C.txt; echo $?`.strip == '0'
      puts "#{PASSED} MATRIX - #{f}C.txt"
    else
      puts "#{FAILED} MATRIX - #{f}C.txt"
      puts "failed command: diff -w -B #{TMP_DIR}/#{f}C.txt #{prefix}#{f}C.txt"
      $failures += 1
      exit 1
    end
  end
end

###############################################################################
# run_prime()
###############################################################################
def run_prime(pipe)
  $prime_limits = [101, 200, 256, 500, 512, 1000, 1024, 10000, 50000, 104729]

  $prime_limits.each do |limit|
    pipe.puts "set prime outputFile #{TMP_DIR}/primes_#{limit}.txt"
    pipe.puts "set prime limit #{limit}"
    pipe.puts "run prime"
  end
end

###############################################################################
# check_prime_results()
###############################################################################
def check_prime_results()
  return unless $prime_limits
  expected = "#{DATA_DIR}/primesExpected.txt"
  $prime_limits.each do |limit|
    actual = "#{TMP_DIR}/primes_#{limit}.txt"
    lines = `wc -l #{actual}`.strip.split(" ").first
    unless lines
      puts "failed to check the number of lines in: #{actual}"
      $failures += 1
      exit 1
    end
    cmd = "head -n #{lines} #{expected} | diff #{actual} -; echo $?"
    if `#{cmd}`.strip == '0'
      puts "#{PASSED} PRIMES - primes_#{limit}.txt"
    else
      puts "#{FAILED} PRIMES - primes_#{limit}.txt"
      puts "failed command: #{cmd}"
      $failures += 1
      exit 1
    end
  end
end

###############################################################################
# test_graph()
###############################################################################
def run_graph(pipe)
  $graph_files = %w(simple)
  input_prefix = "#{DATA_DIR}/graph/"
  output_prefix = "#{TMP_DIR}/"
  pipe.puts "set graph displayResult 0"
  $graph_files.each do |f|
    pipe.puts "set graph inputFile #{input_prefix}#{f}.txt"
    $vertices = `grep -e '^$' -v #{input_prefix}#{f}.txt | wc -l`.strip

    for i in 0...$vertices.to_i
      # BFS
      pipe.puts "set graph sourceNode #{i}"
      pipe.puts "set graph outputFile #{output_prefix}#{f}BFS#{i}.txt"
      pipe.puts "set graph searchMode bfs"
      pipe.puts "run graph"

      # SSSP
      pipe.puts "set graph outputFile #{output_prefix}#{f}SSSP#{i}.txt"
      pipe.puts "set graph searchMode sssp"
      pipe.puts "run graph"
    end

    # APSP
    pipe.puts "set graph outputFile #{output_prefix}#{f}APSP.txt"
    pipe.puts "set graph searchMode apsp"
    pipe.puts "run graph"
  end
end

###############################################################################
# check_graph_results()
###############################################################################
def check_graph_results()
  input_prefix = "#{DATA_DIR}/graph/"
  output_prefix = "#{TMP_DIR}/"

  $graph_files.each do |f|
    # condense the generated files
    `rm -f #{output_prefix}#{f}BFS.txt #{output_prefix}#{f}SSSP.txt`
    for i in 0...$vertices.to_i
      `cat #{output_prefix}#{f}BFS#{i}.txt >> #{output_prefix}#{f}BFS.txt`
      `cat #{output_prefix}#{f}SSSP#{i}.txt >> #{output_prefix}#{f}SSSP.txt`
    end

    # compare each result against the expected outputs
    cmdBFS = "diff -B #{output_prefix}#{f}BFS.txt #{input_prefix}#{f}ExpectedBFS.txt"
    cmdSSSP = "diff -B #{output_prefix}#{f}SSSP.txt #{input_prefix}#{f}ExpectedSSSP.txt"
    cmdAPSP = "tail -n +3 #{output_prefix}#{f}APSP.txt | diff -w -B -E - #{input_prefix}#{f}ExpectedAPSP.txt"

    tests = ['bfs', 'sssp', 'apsp']
    cmds = [cmdBFS, cmdSSSP, cmdAPSP]
    cmds.each_with_index do |cmd, i|
      if `#{cmd}; echo $?`.strip == '0'
        puts "#{PASSED} GRAPHS - #{f}_#{tests[i]}"
      else
        puts "#{FAILED} GRAPHS - #{f}_#{tests[i]}"
        puts "failed command: #{cmd}"
        $failures += 1
        exit 1
      end
    end
  end
end

###############################################################################
# main program
###############################################################################

# make sure everything is built before running the tests
result = system(CMAKE_BUILD_CMD)
if result == false || result == nil
  puts "Failed to build the system for running the tests"
  exit result
end

# clear the old test output files and create a tmp folder if it doesn't exist
`rm -f #{TMP_DIR}/*`
`mkdir -p #{TMP_DIR}`

# open a pipe with popen to emulate terminal inputs / output
puts "Running all tests..."
shell_output = ""
cmd_status = 0
Open3.popen2e(MAIN_PROGRAM_PATH, 'r+') do |stdin, stdout, wait_thr|
  # out_reader = Thread.new { p stdout.read }

  # load the plugins
  stdin.puts("load #{PLUGIN_HELLO_PATH}")
  stdin.puts("load #{PLUGIN_MATRIX_PATH}") if TEST_MATRIX
  stdin.puts("load #{PLUGIN_PRIME_PATH}") if TEST_PRIME
  stdin.puts("load #{PLUGIN_GRAPH_PATH}") if TEST_GRAPH

  # list the parameters for each plugin
  stdin.puts("list") # TODO: this command seems to be breaking things... hmm...

  # run hello sample
  stdin.puts("run hello")

  # test the matrix
  run_matrix(stdin) if TEST_MATRIX

  # test the prime numbers
  run_prime(stdin) if TEST_PRIME

  # test the graph
  run_graph(stdin) if TEST_GRAPH

  stdin.puts("exit")
  stdin.close

  sleep 1 # TODO: I think this is a hack to make it work... need to investigate if there's a better way
  stdout.close
  # shell_output << out_reader.value

  cmd_status = wait_thr.value.exitstatus
end

puts "Checking test results..."

check_matrix_results() if TEST_MATRIX
check_prime_results() if TEST_PRIME
check_graph_results() if TEST_GRAPH

puts shell_output if $failures > 0
cmd_status
