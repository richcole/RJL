require 'rubygems'
require "treetop"
require "polyglot"
require "rjl"

class Node
  def initialize(*elem)
    @elem = elem
  end
end

def nodes(x, r=[])
  if x.is_a?(Array) then
    return x
  elsif x.is_a?(Node) then
    return x
  elsif x.respond_to?(:elements) then
    if x.elements != nil then
      for e in x.elements do
        if e.respond_to?(:node) then 
          r << e.node
        elsif e.respond_to?(:elements) then
          nodes(e, r)
        end
      end
    end
  end
  puts " #{r.inspect}"
  return r
end

parser = RJLParser.new

for input in File.read("examples.txt").split(/\n*\s*===\s*\n*\s*/) do
  puts "Testing: " + input
  output = parser.parse(input)
  if ! output then
    p parser.failure_reason
  else
    p output.node
  end
end
