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

def call_rjl(rjl, test_input, type)
  test_output = File.join("build", File.basename(test_input) + "." + type + ".out")
  file test_output => [test_input, rjl] do
    sh "#{rjl} --#{type}=\"#{test_input}\" > #{test_output}"
  end
  return test_output
end

interp_tests.sort.each do |test_input|
  test_outputs << call_rjl(rjl, test_input, 'interp')
end

for test_type in %w(lex parse code_gen run) do
  rjl_tests.sort.each do |test_input|
    test_outputs << call_rjl(rjl, test_input, test_type)
  end
end

task :test => test_outputs
task :default => :test
