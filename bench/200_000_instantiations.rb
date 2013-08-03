require "benchmark"
include Benchmark

require "fopen_struct"
require "fast_open_struct"
require "ostruct"

N = 200_000

puts "#{N} instantiations:"

bmbm 36 do |b|
  b.report "OpenStruct" do
    N.times do
      OpenStruct.new a: 1, b: 2, c: 3
    end
  end

  b.report "FastOpenStruct" do
    N.times do
      FastOpenStruct.new a: 1, b: 2, c: 3
    end
  end

  b.report "FOpenStruct" do
    N.times do
      FOpenStruct.new a: 1, b: 2, c: 3
    end
  end

  FOpenStruct.calls_until_define_accessor = 10

  b.report "FOpenStruct with accessor definition" do
    N.times do
      FOpenStruct.new a: 1, b: 2, c: 3
    end
  end
end
