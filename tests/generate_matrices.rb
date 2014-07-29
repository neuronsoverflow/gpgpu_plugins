#!/usr/bin/ruby
require 'matrix'

# this file generates random square matrices
# WARNING: MATRIX SIZES 1000+ MAY TAKE HOURS TO BE GENERATED
matrix_sizes = [3, 100, 300, 1000, 5000, 10000]

puts "generating matrices of sizes: #{matrix_sizes}"
matrices = nil

# generate matrices A, B, C where C = A * B
matrix_sizes.each do |size|
  matrices = [nil, nil, nil]
  (0..2).each do |i| # matrices A, B
    puts "\tgenerating matrix #{(i + 'A'.ord).chr}"
    matrices[i] = Matrix.build(size) { (rand * 10).to_i }
  end
  puts "\tcomputing C = A * B"
  matrices[2] = matrices[0] * matrices[1] # matric C

  # now that they have been generated, save them to the file
  prefix = "data/matrix/"
  filename = "#{prefix}matrix#{size}_#{size}"
  ['A', 'B', 'C'].each_with_index do |name, i|
    file = "#{filename}#{name}.txt"
    puts "writing: #{file}"

    File.open(file, 'w') do |file|
      file.puts "rows=#{matrices[i].row_size}"
      file.puts "cols=#{matrices[i].column_size}"
      file.puts "\n"
      # put a row at a time
      for j in 0..matrices[i].row_size
        file.puts matrices[i].row(j).to_a.join(" ")
      end
    end
  end
end
