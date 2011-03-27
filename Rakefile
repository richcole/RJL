#!/usr/bin/rake

require 'fileutils'

class String
  def /(x)
    return File.join(self, x)
  end
end

build = "build"
debug = build / "debug"
rjl   = debug / "rjl"

directory debug

cpp_source = Dir.glob("src/*.cpp").to_a
h_source = Dir.glob("src/*.h").to_a

file rjl => [ debug ] + cpp_source + h_source do
  sh "g++ -Wall -ggdb -o #{rjl} #{cpp_source.join(" ")}"
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

tests = Dir.glob("tests/*.r").to_a.sort.map { |x| run_test(x, rjl) }

task :tests => tests
task :default => rjl
