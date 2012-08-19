require 'set'

class String
  def /(o)
    File.join(self, o)
  end
end

class Builder 
  include Rake::DSL

  def initialize
    @build_dir = "build"
    @release_build_dir = "build" / "release"
    @debug_build_dir = "build" / "debug"
    @cpp_flags = "-Isrc"
  end

  def obj(src, args, dst_dir)
    dst = dst_dir / File.basename(src) + ".o"
    rule(dst => [ src ] + h_depends(src)) do
      run "mkdir -p #{File.dirname(dst)}"
      run "g++ #{args} -c -o #{dst} #{src}"
    end
    dst
  end

  def link(srcs, args, dst)
    rule(dst => srcs) do
      run "mkdir -p #{File.dirname(dst)}"
      run "g++ #{args} -o #{dst} #{srcs.join(" ")}"
    end
    dst
  end

  def cpp_files
    Dir.glob("src/*.cpp").to_a
  end

  def h_depends(src)
    content = File.read(src)
    hs = Dir.glob("src/*.h").to_a.select do |h|
      content.match(/"#{File.basename(h)}"/)
    end
    hs = hs + Set.new(hs.map { |h| h_depends(h) }.flatten).to_a
    hs
  end

  def run(cmd)
    puts cmd
    if ! system(cmd) then
      raise("Command failed: " + cmd)
    end
  end

  def rules
    release_obj = cpp_files.map do |src| 
      obj(src, "-Wall -O #{@cpp_flags}", @release_build_dir / "obj")
    end
    debug_obj = cpp_files.map do |src| 
      obj(src, "-Wall -ggdb #{@cpp_flags}", @debug_build_dir + "/obj")
    end
    release_binary = link(release_obj, "-O", @release_build_dir / "/rjl")
    debug_binary = link(debug_obj, "-ggdb", @debug_build_dir / "/rjl")

    rule "default" => [release_binary, debug_binary]

    rule "clean" do
      run "rm -rf build"
    end
  end

end

Builder.new.rules
