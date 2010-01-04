#!/usr/bin/rake

build = "build" 
debug_rjl      = File.join("build/debug/rjl")
release_rjl    = File.join("build/release/rjl")
interp_tests   = Dir.glob("tests/test_*.i").to_a
rjl_tests      = Dir.glob("tests/test_*.r").to_a
rjl_perf_tests = Dir.glob("perf_tests/test_*.r").to_a
test_outputs = []
test_diff_outputs = []
perf_test_outputs = []
directory build

def compile_rjl(target, options, build)
  directory (target_dir = File.dirname(target))
  file target => [build, "rjl.cpp", target_dir] do
    sh "g++ #{options} -o #{target} rjl.cpp"
  end
end

task :no_tabs do
  puts "\n=== Checking for tabs in rjl.cpp"
  sh "  ! grep -q '[[:cntrl:]]' rjl.cpp"
end

compile_rjl(release_rjl, "-O3 -Wall", build)
compile_rjl(debug_rjl, "-ggdb -Wall", build)

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
    sh "diff #{test_output} #{expected_output} > #{diff_output} || true"
  end
  return diff_output
end

interp_tests.sort.each do |test_input|
  test_outputs << call_rjl(debug_rjl, test_input, 'interp')
end

for test_type in %w(lex parse code_gen run) do
  rjl_tests.sort.each do |test_input|
    test_outputs << call_rjl(debug_rjl, test_input, test_type)
  end
end

rjl_perf_tests.sort.each do |test_input|
  perf_test_outputs << call_rjl(debug_rjl, test_input, "run", "time")
end

for test_type in %w(run) do
  rjl_tests.sort.each do |test_input|
    test_diff_outputs << compare_rjl(debug_rjl, test_input, test_type)
  end
end

task :eval_test_outputs do
  puts "\n=== Evaluating Test Outputs"
  for test_output in test_diff_outputs do
    if File.size(test_output) > 0 then
      puts "FAILURE: #{test_output}"
      puts File.read(test_output);
    end
  end
end

task :clean do
  sh "rm -rf build"
end

task :test => test_outputs + test_diff_outputs + [:eval_test_outputs]
task :perf_test => perf_test_outputs
task :default => [:no_tabs, :test]
