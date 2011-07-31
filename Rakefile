#!/usr/bin/rake

require 'fileutils'

class String
  def /(x)
    return File.join(self, x)
  end
end

build = "build"
debug = build / "debug"
release = build / "release"
rjl_debug   = debug / "rjl"
rjl_release   = release / "rjl"

directory debug
directory release

cpp_source = Dir.glob("src/*.cpp").to_a
h_source = Dir.glob("src/*.h").to_a

file rjl_debug => [ debug ] + cpp_source + h_source do
  sh "g++ -Wall -ggdb -o #{rjl_debug} #{cpp_source.join(" ")} -lSDL"
end

file rjl_release => [ release ] + cpp_source + h_source do
  sh "g++ -Wall -O9 -o #{rjl_release} #{cpp_source.join(" ")} -lSDL"
end

def run_test(test_src, rjl)
  test_basename = File.basename(test_src)
  target = "run_#{test_basename}"
  output_dir = "build/test-output"
  test_output = output_dir / test_basename + ".out"
  expected_output = "tests/expected" / test_basename + ".out"
  directory output_dir
  task target => [test_src, rjl, output_dir, expected_output] do
    sh "#{rjl} #{test_src} > #{test_output}"
    sh "diff #{expected_output} #{test_output}"
  end
  return target
end

tests = Dir.glob("tests/*.r").to_a.sort.map { |x| run_test(x, rjl_release) }

task :tests => tests
task :default => [rjl_debug, rjl_release]
task :debug => [rjl_debug]
task :release => [rjl_release]
task :clean do
  sh "rm -rf #{build}"
end
task :window => [debug] do
  sh "g++ -ggdb -o #{debug/"window"} window/window.cpp -lSDL"
end
