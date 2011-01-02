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

task :default => rjl
