require "benchmark"
include Benchmark

require "fopen_struct"
require "fast_open_struct"
require "ostruct"

N = 5_000_000

puts "Attribute assignment:"

bmbm 14 do |b|
  b.report "OpenStruct" do
    os = OpenStruct.new a: 1, b: 2, c: 3
    N.times do
      os.a = 4
      os.b = 5
      os.c = 6
    end
  end

  b.report "FastOpenStruct" do
    os = FastOpenStruct.new a: 1, b: 2, c: 3
    N.times do
      os.a = 4
      os.b = 5
      os.c = 6
    end
  end

  b.report "FOpenStruct" do
    os = FOpenStruct.new a: 1, b: 2, c: 3
    N.times do
      os.a = 4
      os.b = 5
      os.c = 6
    end
  end
end
