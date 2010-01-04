#!/usr/bin/rake

build = "build" 
rjl   = File.join("build/rjl")
interp_tests   = Dir.glob("tests/test_*.i").to_a
rjl_tests      = Dir.glob("tests/test_*.r").to_a
rjl_perf_tests = Dir.glob("perf_tests/test_*.r").to_a
test_outputs = []
perf_test_outputs = []
directory build

file rjl => [build, "rjl.cpp"] do
  sh "g++ -Wall -O3 -o #{rjl} rjl.cpp"
end

def call_rjl(rjl, test_input, type, prefix="")
  test_output = File.join("build", test_input + "." + type + ".out")
  test_output_dir = File.dirname(test_output)
  directory test_output_dir
  file test_output => [test_input, rjl, test_output_dir] do
    sh "#{prefix} #{rjl} --#{type}=\"#{test_input}\" > #{test_output}"
  end
  return test_output
end

def compare_rjl(rjl, test_input, type)
  test_output = File.join("build", test_input + "." + type + ".out")
  test_output_dir = File.dirname(test_output)
  directory test_output_dir
  expected_output = File.join("expected", test_input + "." + type + ".out")
  diff_output = File.join("build", test_input + "." + type + ".out.diff")
  file diff_output => [test_output, expected_output, rjl, test_output_dir] do
    sh "diff #{test_output} #{expected_output} > #{diff_output}"
  end
  return diff_output
end

interp_tests.sort.each do |test_input|
  test_outputs << call_rjl(rjl, test_input, 'interp')
end

for test_type in %w(lex parse code_gen run) do
  rjl_tests.sort.each do |test_input|
    test_outputs << call_rjl(rjl, test_input, test_type)
  end
end

rjl_perf_tests.sort.each do |test_input|
  perf_test_outputs << call_rjl(rjl, test_input, "run", "time")
end

for test_type in %w(run) do
  rjl_tests.sort.each do |test_input|
    test_outputs << compare_rjl(rjl, test_input, test_type)
  end
end

task :clean do
  sh "rm -rf build"
end

task :test => test_outputs
task :perf_test => perf_test_outputs
task :default => :test
