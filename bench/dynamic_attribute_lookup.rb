require "benchmark"
include Benchmark

require "fopen_struct"
require "fast_open_struct"
require "ostruct"

N = 5_000_000

puts "Dynamic attribute lookup:"

bmbm 14 do |b|
  b.report "OpenStruct" do
    os = OpenStruct.new
    os.a = 1
    os.b = 2
    os.c = 3
    N.times do
      os.a
      os.b
      os.c
    end
  end

  b.report "FastOpenStruct" do
    os = FastOpenStruct.new
    os.a = 1
    os.b = 2
    os.c = 3
    N.times do
      os.a
      os.b
      os.c
    end
  end

  b.report "FOpenStruct" do
    os = FOpenStruct.new
    os.a = 1
    os.b = 2
    os.c = 3
    N.times do
      os.a
      os.b
      os.c
    end
  end
end
