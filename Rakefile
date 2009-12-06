#!/usr/bin/rake

build = "build" 
rjl   = File.join("build/rjl")
interp_tests = Dir.glob("test_*.i").to_a
rjl_tests    = Dir.glob("test_*.r").to_a
test_outputs = []
directory build

file rjl => [build, "rjl.cpp"] do
  sh "g++ -Wall -ggdb -o #{rjl} rjl.cpp"
end

interp_tests.sort.each do |test_input|
  test_output = File.join("build", File.basename(test_input) + ".out")
  test_outputs << test_output
  file test_output => [test_input, rjl] do
    sh "#{rjl} --interpret=\"#{test_input}\" > #{test_output}"
  end
end

rjl_tests.sort.each do |test_input|
  test_output = File.join("build", File.basename(test_input) + ".lex.out")
  test_outputs << test_output
  file test_output => [test_input, rjl] do
    sh "#{rjl} --lex=\"#{test_input}\" > #{test_output}"
  end
end

rjl_tests.sort.each do |test_input|
  test_output = File.join("build", File.basename(test_input) + ".parse.out")
  test_outputs << test_output
  file test_output => [test_input, rjl] do
    sh "#{rjl} --parse=\"#{test_input}\" > #{test_output}"
  end
end

task :test => test_outputs
task :default => :test
