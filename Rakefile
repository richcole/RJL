#!/usr/bin/rake

build = "build" 
rjl   = File.join("build/rjl")
interp_tests = Dir.glob("tests/test_*.i").to_a
rjl_tests    = Dir.glob("tests/test_*.r").to_a
test_outputs = []
directory build

file rjl => [build, "rjl.cpp"] do
  sh "g++ -Wall -ggdb -o #{rjl} rjl.cpp"
end

def call_rjl(rjl, test_input, type)
  test_output = File.join("build", File.basename(test_input) + "." + type + ".out")
  file test_output => [test_input, rjl] do
    sh "#{rjl} --#{type}=\"#{test_input}\" > #{test_output}"
  end
  return test_output
end

def compare_rjl(rjl, test_input, type)
  test_output = File.join("build", File.basename(test_input) + "." + type + ".out")
  expected_output = File.join("expected", File.basename(test_input) + "." + type + ".out")
  diff_output = File.join("build", File.basename(test_input) + "." + type + ".out.diff")
  file diff_output => [test_output, expected_output, rjl] do
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

for test_type in %w(run) do
  rjl_tests.sort.each do |test_input|
    test_outputs << compare_rjl(rjl, test_input, test_type)
  end
end

task :clean do
  sh "rm -rf build"
end

task :test => test_outputs
task :default => :test
