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
  target = "run_#{File.basename(test_src)}"
  task target => [test_src, rjl] do
    sh "valgrind #{rjl} #{test_src}"
  end
  return target
end

tests = Dir.glob("tests/*.r").to_a.sort.map { |x| run_test(x, rjl) }
puts "tests=" + tests.inspect

task :tests => tests
task :default => rjl
